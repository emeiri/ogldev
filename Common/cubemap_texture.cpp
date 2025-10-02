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
#include <gli/gli.hpp>

#include "ogldev_math_3d.h"
#include "ogldev_ect_cubemap.h"
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
		
		// TODO: can be done outside the loop
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        stbi_image_free(image_data);
    }
}


void CubemapTexture::LoadKTX(const std::string& Filename)
{
    const char* pExt = strrchr(Filename.c_str(), '.');

    m_isKTX = pExt && !strcmp(pExt, ".ktx");

    if (!m_isKTX) {
        printf("'%s' is not a KTX file\n", Filename.c_str());
        assert(0);
    }

    glGenTextures(1, &m_textureObj);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureObj);

    gli::texture texture = gli::load(Filename);
    if (texture.empty()) {
        printf("Empty texture '%s'\n", Filename.c_str());
        assert(0);
    }

    gli::gl GL(gli::gl::PROFILE_GL33);
    m_ktxFormat = GL.translate(texture.format(), texture.swizzles());
    GLenum Target = GL.translate(texture.target());

    assert(Target == GL_TEXTURE_CUBE_MAP);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, (GLint)(texture.levels() - 1));

    for (std::size_t Level = 0; Level < texture.levels(); ++Level) {
        for (std::size_t Face = 0; Face < 6; ++Face) {
            glm::tvec3<GLsizei> Extent = texture.extent(Level);            
            glTexImage2D((GLenum)(GL_TEXTURE_CUBE_MAP_POSITIVE_X + Face), (GLint)Level, m_ktxFormat.Internal,
                         Extent.x, Extent.y, 0, m_ktxFormat.External, m_ktxFormat.Type, texture.data(0, Face, Level));
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);   
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

    stbi_set_flip_vertically_on_load(0);

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
