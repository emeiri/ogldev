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



void BaseTerrain::InitTerrain(float WorldScale, float TextureScale, const std::vector<string>& TextureFilenames)
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

    for (int i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(m_pTextures) ; i++) {
        m_pTextures[i] = new Texture(GL_TEXTURE_2D);
        m_pTextures[i]->Load(TextureFilenames[i]);
    }
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


void BaseTerrain::Render(const BasicCamera& Camera)
{
    Matrix4f VP = Camera.GetViewProjMatrix();

    m_terrainTech.Enable();
    m_terrainTech.SetVP(VP);

    for (int i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(m_pTextures); i++) {
        m_pTextures[i]->Bind(COLOR_TEXTURE_UNIT_0 + i);
    }

    m_triangleList.Render();
}


