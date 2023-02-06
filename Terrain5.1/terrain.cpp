/*
    Copyright 2022 Etay Meiri

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cerrno>
#include <string.h>

#include "terrain.h"
#include "texture_config.h"
#include "3rdparty/stb_image_write.h"

//#define DEBUG_PRINT

BaseTerrain::~BaseTerrain()
{
    Destroy();
}


void BaseTerrain::Destroy()
{
    m_heightMap.Destroy();
    m_triangleList.Destroy();
}



void BaseTerrain::InitTerrain(float WorldScale, float TextureScale, const std::vector<string>& TextureFilenames, const Vector3f& LightDir)
{
    if (!m_terrainTech.Init()) {
        printf("Error initializing tech\n");
        exit(0);
    }
	
    if (TextureFilenames.size() != ARRAY_SIZE_IN_ELEMENTS(m_pTextures)) {
        printf("%s:%d - number of provided textures (%lld) is not equal to the size of the texture array (%lld)\n",
               __FILE__, __LINE__, TextureFilenames.size(), ARRAY_SIZE_IN_ELEMENTS(m_pTextures));
        exit(0);
    }

    m_worldScale = WorldScale;
    m_textureScale = TextureScale;
    m_lightDir = LightDir;

    for (int i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(m_pTextures) ; i++) {
        m_pTextures[i] = new Texture(GL_TEXTURE_2D);
        m_pTextures[i]->Load(TextureFilenames[i]);
    }

    PrepareSlopeLightInfoParams();
}


float BaseTerrain::GetHeightInterpolated(float x, float z) const
{
    float BaseHeight = GetHeight((int)x, (int)z);

    if (((int)x + 1 >= m_terrainSize) ||  ((int)z + 1 >= m_terrainSize)) {
        return BaseHeight;
    }

    float NextXHeight = GetHeight((int)x + 1, (int)z);

    float RatioX = x - floorf(x);

    float InterpolatedHeightX = (float)(NextXHeight - BaseHeight) * RatioX + (float)BaseHeight;

    float NextZHeight = GetHeight((int)x, (int)z + 1);

    float RatioZ = z - floorf(z);

    float InterpolatedHeightZ = (float)(NextZHeight - BaseHeight) * RatioZ + (float)BaseHeight;

    float FinalHeight = (InterpolatedHeightX + InterpolatedHeightZ) / 2.0f;

    return FinalHeight;
}


void BaseTerrain::LoadFromFile(const char* pFilename)
{
    LoadHeightMapFile(pFilename);

    m_triangleList.CreateTriangleList(m_terrainSize, m_terrainSize, this);
}


void BaseTerrain::LoadHeightMapFile(const char* pFilename)
{
    int FileSize = 0;
    unsigned char* p = (unsigned char*)ReadBinaryFile(pFilename, FileSize);

    if (FileSize % sizeof(float) != 0) {
        printf("%s:%d - '%s' does not contain an whole number of floats (size %d)\n", __FILE__, __LINE__, pFilename, FileSize);
        exit(0);
    }

    m_terrainSize = (int)sqrtf((float)FileSize / (float)sizeof(float));

    printf("Terrain size %d\n", m_terrainSize);

    if ((m_terrainSize * m_terrainSize) != (FileSize / sizeof(float))) {
        printf("%s:%d - '%s' does not contain a square height map - size %d\n", __FILE__, __LINE__, pFilename, FileSize);
        exit(0);
    }

    m_heightMap.InitArray2D(m_terrainSize, m_terrainSize, (float*)p);
}


void BaseTerrain::SaveToFile(const char* pFilename)
{    
    unsigned char* p = (unsigned char*)malloc(m_terrainSize * m_terrainSize);

    float* src = m_heightMap.GetBaseAddr();

    float Delta = m_maxHeight - m_minHeight;

    for (int i = 0; i < m_terrainSize * m_terrainSize; i++) {
        float f = (src[i] - m_minHeight) / Delta;
        p[i] = (unsigned char)(f * 255.0f);
    }

    stbi_write_png("heightmap.png", m_terrainSize, m_terrainSize, 1, p, m_terrainSize);

    free(p);
}


void BaseTerrain::Render(const BasicCamera& Camera)
{
    Matrix4f VP = Camera.GetViewProjMatrix();

    m_terrainTech.Enable();
    m_terrainTech.SetVP(VP);

    for (int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(m_pTextures); i++) {
        if (m_pTextures[i]) {
            m_pTextures[i]->Bind(COLOR_TEXTURE_UNIT_0 + i);
        }
    }
	
    m_terrainTech.SetLightDir(m_lightDir);

    m_triangleList.Render();
}


void BaseTerrain::SetMinMaxHeight(float MinHeight, float MaxHeight)
{
    m_minHeight = MinHeight;
    m_maxHeight = MaxHeight;

    m_terrainTech.Enable();
    m_terrainTech.SetMinMaxHeight(MinHeight, MaxHeight);
}


void BaseTerrain::SetTextureHeights(float Tex0Height, float Tex1Height, float Tex2Height, float Tex3Height)
{
    m_terrainTech.SetTextureHeights(Tex0Height, Tex1Height, Tex2Height, Tex3Height); 
}


void BaseTerrain::PrepareSlopeLightInfoParams()
{
    /* Slope lighting works by comparing the height of the current vertex with the
       height of the vertex which is "before" it on the way to the light source. This
       means that we need to reverse the light vector and in order to improve the accuracy
       intepolate between the two vertices that are closest to the reversed light vector (since
       in most cases the light vector will go between two vertices).

       The algorithm works by doing a dot product of the reversed light vector with two vectors:
       PosX (1, 0, 0) and PosZ (0, 0, 1).

       In order to understand the calculations below we need the following table that shows the
       two dot products for the eight vertices around the current one (degrees in parenthesis)

       |-------------------------------------------------------|
       | dpx: -0.707 (135) | dpx: 0 (90)   | dpx: 0.707 (45)   |
       | dpz: 0.707 (45)   | dpz: 1, (0)   | dpz: 0.707 (45)   |
       |-------------------|---------------|-------------------|
       | dpx: -1 (180)     |               | dpx: 1 (0)        |
       | dpz: 0 (90)       |               | dpz: 0 (90)       |
       |-------------------|---------------|-------------------|
       | dpx: -0.707 (135) | dpx: 0 (90)   | dpx: 0.707 (45)   |
       | dpz: -0.707 (135) | dpz: -1 (180) | dpz: -0.707 (135) |
       |-------------------------------------------------------|
    */

    Vector3f PosX(1.0f, 0.0f, 0.0f);
    float dpx = PosX.Dot(m_lightDir * -1.0f);
    float cosx = ToDegree(acos(dpx));
    printf("PosX %f %f\n", dpx, cosx);

    Vector3f PosZ(0.0f, 0.0f, 1.0f);
    float dpz = PosZ.Dot(m_lightDir * -1.0f);
     float cosz = ToDegree(acos(dpz));
    printf("PosZ %f %f\n", dpz, cosz);

    float a45 = cosf(ToRadian(45.0f));

    bool InterpolateOnX = false;

    if (dpz >= a45) {
        //  printf("foo1\n");
        m_sli.dz0 = m_sli.dz1 = 1;
        InterpolateOnX = true;
    } else if (dpz <= -a45) {
        //printf("foo2\n");
        m_sli.dz0 = m_sli.dz1 = -1;
        InterpolateOnX = true;
    } else {
        if (dpz >= 0.0f) {
            //  printf("foo3\n");
            m_sli.dz0 = 0;
            m_sli.dz1 = 1;
            m_sli.Factor = dpz;
        } else {
            // printf("foo4\n");
            m_sli.dz0 = 0;
            m_sli.dz1 = -1;
            m_sli.Factor = -dpz;
        }

        if (dpx >= 0.0f) {
            // printf("foo5\n");
            m_sli.dx0 = m_sli.dx1 = 1;
        } else {
            // printf("foo6\n");
            m_sli.dx0 = m_sli.dx1 = -1;
        }
    }

    if (InterpolateOnX) {
        if (dpx >= 0.0f) {
            // printf("foo7\n");
            m_sli.dx0 = 0;
            m_sli.dx1 = 1;
            m_sli.Factor = dpx;
        } else {
            // printf("foo8\n");
            m_sli.dx0 = 0;
            m_sli.dx1 = -1;
            m_sli.Factor = -dpx;
        }
    }

    m_sli.Factor = 1.0f - m_sli.Factor / a45;

    printf("0: dx %d dz %d\n", m_sli.dx0, m_sli.dz0);
    printf("1: dx %d dz %d\n", m_sli.dx1, m_sli.dz1);
    printf("Factor %f\n", m_sli.Factor);
}


Vector3f BaseTerrain::GetColor(int x, int z) const
{
    return GetSlopeScaleLighting(x, z);
}


Vector3f BaseTerrain::GetSimpleLighting(int x, int z) const
{
    float Height = GetHeight(x, z);

    float Delta = Height - m_minHeight;
    float MaxDelta = m_maxHeight - m_minHeight;

    float f = Delta / MaxDelta;

    Vector3f Color(f, f, f);

    return Color;
}


Vector3f BaseTerrain::GetSlopeScaleLighting(int x, int z) const
{
    float Height = GetHeight(x, z);

    float f = 0.0f;

    int XBefore0 = x + m_sli.dx0 * 5;
    int ZBefore0 = z + m_sli.dz0 * 5;

    int XBefore1 = x + m_sli.dx1 * 5;
    int ZBefore1 = z + m_sli.dz1 * 5;

    if ((XBefore0 >= 0) && (XBefore0 < m_terrainSize) && (ZBefore0 >= 0) && (ZBefore0 < m_terrainSize) &&
        (XBefore1 >= 0) && (XBefore1 < m_terrainSize) && (ZBefore1 >= 0) && (ZBefore1 < m_terrainSize)) {

        float HeightF32 = Height;
        float HeightBefore0 = GetHeight(XBefore0, ZBefore0);
        float HeightBefore1 = GetHeight(XBefore1, ZBefore1);

        // Interpolate between the height of the two vertices
        float HeightBefore = HeightBefore0 * m_sli.Factor + (1.0f - m_sli.Factor) * HeightBefore1;

        float LightSoftness = 2.0f;
        f = 1.0f - (HeightBefore - HeightF32) / LightSoftness;
        float min_brightness = 0.2f;
        f = std::max(std::min(f, 1.0f), min_brightness);
        //        printf("%f\n", f);
    } else {
        float Delta = Height - m_minHeight;
        float MaxDelta = m_maxHeight - m_minHeight;

        f = (float)Delta / (float)MaxDelta;
    }

    Vector3f Color(f, f, f);

    return Color;
}

