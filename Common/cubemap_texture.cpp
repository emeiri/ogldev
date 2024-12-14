/*

        Copyright 2011 Etay Meiri

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

#include <iostream>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "ogldev_math_3d.h"
#include "ogldev_cubemap_texture.h"
#include "ogldev_util.h"
#include "3rdparty/stb_image.h"

static const GLenum types[6] = {  GL_TEXTURE_CUBE_MAP_POSITIVE_X,
                                  GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
                                  GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
                                  GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
                                  GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
                                  GL_TEXTURE_CUBE_MAP_NEGATIVE_Z };

#define CUBEMAP_NUM_FACES 6

#define CUBE_MAP_INDEX_POS_X 0
#define CUBE_MAP_INDEX_NEG_X 1
#define CUBE_MAP_INDEX_POS_Y 2
#define CUBE_MAP_INDEX_NEG_Y 3
#define CUBE_MAP_INDEX_POS_Z 4
#define CUBE_MAP_INDEX_NEG_Z 5


static glm::vec3 FaceCoordsToXYZ(int x, int y, int FaceID, int FaceSize)
{
    float A = 2.0f * float(x) / FaceSize;
    float B = 2.0f * float(y) / FaceSize;

    glm::vec3 Ret;

    switch (FaceID) {
    case CUBE_MAP_INDEX_POS_X:
        Ret = glm::vec3(A - 1.0f, 1.0f, 1.0f - B);
        break;

    case CUBE_MAP_INDEX_NEG_X:
        Ret = glm::vec3(1.0f - A, -1.0f, 1.0f - B);
        break;

    case CUBE_MAP_INDEX_POS_Y:
        Ret = glm::vec3(1.0f - B, A - 1.0f, 1.0f);
        break;

    case CUBE_MAP_INDEX_NEG_Y:
        Ret = glm::vec3(B - 1.0f, A - 1.0f, -1.0f);
        break;

    case CUBE_MAP_INDEX_POS_Z:
        Ret = glm::vec3(-1.0f, A - 1.0f, 1.0f - B);
        break;

    case CUBE_MAP_INDEX_NEG_Z:
        Ret = glm::vec3(1.0f, 1.0f - A, 1.0f - B);
        break;
     
    default:
        assert(0);
    }

    return Ret;
}


static void ConvertEquirectangularImageToCubemap(const Bitmap& b, 
                                                 std::vector<Bitmap>& Cubemap)
{
    int FaceSize = b.w_ / 4;

    Cubemap.resize(CUBEMAP_NUM_FACES);

    for (int i = 0; i < CUBEMAP_NUM_FACES; i++) {
        Cubemap[i].Init(FaceSize, FaceSize, b.comp_, b.fmt_);
    }

    int MaxW = b.w_ - 1;
    int MaxH = b.h_ - 1;

    for (int face = 0; face < CUBEMAP_NUM_FACES; face++) {
        for (int y = 0; y < FaceSize; y++) {
            for (int x = 0; x < FaceSize; x++) {
                glm::vec3 P = FaceCoordsToXYZ(x, y, face, FaceSize);
                float R = sqrtf(P.x * P.x + P.y * P.y);
                float phi = atan2f(P.y, P.x);
                float theta = atan2f(P.z, R);

                // Calculate texture coordinates
                float u = (float)((phi + M_PI) / (2.0f * M_PI));
                float v = (float((M_PI / 2.0f - theta) / M_PI));

                // Scale texture coordinates by image size
                float U = u * b.w_;
                float V = v * b.h_;

                // 4-samples for bilinear interpolation
                int U1 = CLAMP(int(floor(U)), 0, MaxW);
                int V1 = CLAMP(int(floor(V)), 0, MaxH);
                int U2 = CLAMP(U1 + 1, 0, MaxW);
                int V2 = CLAMP(V1 + 1, 0, MaxH);

                // Calculate the fractional part
                float s = U - U1;
                float t = V - V1;

                // Fetch 4-samples
                glm::vec4 BottomLeft  = b.getPixel(U1, V1);
                glm::vec4 BottomRight = b.getPixel(U2, V1);
                glm::vec4 TopLeft     = b.getPixel(U1, V2);
                glm::vec4 TopRight    = b.getPixel(U2, V2);

                // Bilinear interpolation
                glm::vec4 color = BottomLeft * (1 - s) * (1 - t) + 
                                  BottomRight * (s) * (1 - t) + 
                                  TopLeft * (1 - s) * t + 
                                  TopRight * (s) * (t);

                Cubemap[face].setPixel(x, y, color);
            }   // j loop
        }   // i loop
    }   // Face loop
}


CubemapTexture::CubemapTexture(const string& Directory,
                               const string& PosXFilename,
                               const string& NegXFilename,
                               const string& PosYFilename,
                               const string& NegYFilename,
                               const string& PosZFilename,
                               const string& NegZFilename)
{
    string::const_iterator it = Directory.end();
    it--;
    string BaseDir = (*it == '/') ? Directory : Directory + "/";

    m_fileNames[0] = BaseDir + PosXFilename;
    m_fileNames[1] = BaseDir + NegXFilename;
    m_fileNames[2] = BaseDir + PosYFilename;
    m_fileNames[3] = BaseDir + NegYFilename;
    m_fileNames[4] = BaseDir + PosZFilename;
    m_fileNames[5] = BaseDir + NegZFilename;

    m_textureObj = 0;
}


CubemapTexture::~CubemapTexture()
{
    if (m_textureObj != 0) {
        glDeleteTextures(1, &m_textureObj);
    }
}


void CubemapTexture::Load()
{
    stbi_set_flip_vertically_on_load(0);

    glGenTextures(1, &m_textureObj);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureObj);

    for (unsigned int i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(types) ; i++) {
        int Width, Height;
        void* pData = NULL;

        int BPP;
        unsigned char* image_data = stbi_load(m_fileNames[i].c_str(), &Width, &Height, &BPP, 0);

        if (!image_data) {
            printf("Can't load texture from '%s' - %s\n", m_fileNames[i].c_str(), stbi_failure_reason());
            exit(0);
        }

        printf("Width %d, height %d, bpp %d\n", Width, Height, BPP);

        pData = image_data;

        glTexImage2D(types[i], 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, pData);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        stbi_image_free(image_data);
    }
}


void CubemapTexture::Bind(GLenum TextureUnit)
{
    glActiveTexture(TextureUnit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureObj);
}


CubemapEctTexture::CubemapEctTexture(const std::string& Filename)
{
    m_filename = Filename;
}


void CubemapEctTexture::Load()
{
    int Width, Height, Comp;

    const float* pImg = stbi_loadf(m_filename.c_str(), &Width, &Height, &Comp, 3);
    
    if (!pImg) {
        printf("Error loading '%s'\n", m_filename.c_str());
        exit(1);
    }

    Bitmap In(Width, Height, Comp, eBitmapFormat_Float, (void*)pImg);
    std::vector<Bitmap> Cubemap;
    ConvertEquirectangularImageToCubemap(In, Cubemap);

    stbi_image_free((void*)pImg);

    LoadCubemapData(Cubemap);
}


void CubemapEctTexture::LoadCubemapData(const std::vector<Bitmap>& Cubemap)
{
    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_textureObj);
    glTextureParameteri(m_textureObj, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_textureObj, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_textureObj, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_textureObj, GL_TEXTURE_BASE_LEVEL, 0);
    glTextureParameteri(m_textureObj, GL_TEXTURE_MAX_LEVEL, 0);
    glTextureParameteri(m_textureObj, GL_TEXTURE_MAX_LEVEL, 0);
    glTextureParameteri(m_textureObj, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_textureObj, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureStorage2D(m_textureObj, 1, GL_RGB32F, Cubemap[0].w_, Cubemap[0].h_);

    for (int i = 0; i < CUBEMAP_NUM_FACES; i++) {
        const void* pSrc = Cubemap[i].data_.data();
        glTextureSubImage3D(m_textureObj, 
                            0,      // mipmap level
                            0,      // xOffset
                            0,      // yOffset
                            i,      // zOffset (layer in the case of a cubemap)
                            Cubemap[0].w_, Cubemap[0].h_,   // 2D image dimensions
                            1,          // depth
                            GL_RGB,     // format
                            GL_FLOAT,   // data type
                            pSrc);
    }
}


void CubemapEctTexture::Bind(GLenum TextureUnit)
{
    glBindTextures(TextureUnit - GL_TEXTURE0, 1, &m_textureObj);
}
