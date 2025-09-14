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
#include <math.h>
#include "ogldev_util.h"
#include "ogldev_texture.h"
#include "3rdparty/stb_image.h"
#include "3rdparty/stb_image_write.h"


static int GetNumMipMapLevels2D(int w, int h)
{
    int levels = 1;
    while ((w | h) >> levels) {
        levels += 1;
    }
    return levels;
}


Texture::Texture(GLenum TextureTarget, const std::string& FileName)
{
    m_textureTarget = TextureTarget;
    m_fileName      = FileName;
}


Texture::Texture(GLenum TextureTarget)
{
    m_textureTarget = TextureTarget;
}


void Texture::Load(u32 BufferSize, void* pData, bool IsSRGB)
{
    void* pImageData = stbi_load_from_memory((const stbi_uc*)pData, BufferSize, &m_imageWidth, &m_imageHeight, &m_imageBPP, 0);

    LoadInternal(pImageData, IsSRGB);

    stbi_image_free(pImageData);
}

bool Texture::Load(bool IsSRGB)
{
    const char* pExt = strrchr(m_fileName.c_str(), '.');

    m_isKTX = pExt && !strcmp(pExt, ".ktx");

    unsigned char* pImageData = NULL;

    gli::texture gliTex;
    if (m_isKTX) {
        gliTex = gli::load_ktx(m_fileName.c_str());
        gli::gl GL(gli::gl::PROFILE_KTX);
        m_ktxFormat = GL.translate(gliTex.format(), gliTex.swizzles());
        glm::tvec3<GLsizei> extent(gliTex.extent(0));
        m_imageWidth = extent.x;
        m_imageHeight = extent.y;
        m_imageBPP = extent.z;        
        pImageData = (unsigned char*)gliTex.data();
    } else {
        stbi_set_flip_vertically_on_load(1);

        pImageData = stbi_load(m_fileName.c_str(), &m_imageWidth, &m_imageHeight, &m_imageBPP, 0);
    }

    if (!pImageData) {
        printf("Can't load texture from '%s' - %s\n", m_fileName.c_str(), stbi_failure_reason());
        exit(0);
    }

    printf("Loaded texture '%s' width %d, height %d, bpp %d\n", m_fileName.c_str(), m_imageWidth, m_imageHeight, m_imageBPP);

    LoadInternal(pImageData, IsSRGB);

    // Free the image data after loading it into OpenGL
    if (!m_isKTX) {
        stbi_image_free(pImageData);
    }
        
    return true;
}


void Texture::Load(const std::string& Filename, bool IsSRGB)
{
    m_fileName = Filename;

    if (!Load(IsSRGB)) {
        exit(0);
    }
}


void Texture::LoadRaw(int Width, int Height, int BPP, const unsigned char* pImageData, bool IsSRGB)
{
    m_imageWidth = Width;
    m_imageHeight = Height;
    m_imageBPP = BPP;

    LoadInternal(pImageData, IsSRGB);
}


void Texture::LoadInternal(const void* pImageData, bool IsSRGB)
{
    if (IsGLVersionHigher(4, 5)) {
        LoadInternalDSA(pImageData, IsSRGB);
    } else {
        LoadInternalNonDSA(pImageData, IsSRGB);
    }
}


void Texture::LoadInternalNonDSA(const void* pImageData, bool IsSRGB)
{
    glGenTextures(1, &m_textureObj);
    glBindTexture(m_textureTarget, m_textureObj);

    GLenum InternalFormat = GL_NONE;

    if (m_textureTarget == GL_TEXTURE_2D) {
        switch (m_imageBPP) {
        case 1: {
            glTexImage2D(m_textureTarget, 0, GL_RED, m_imageWidth, m_imageHeight, 0, GL_RED, GL_UNSIGNED_BYTE, pImageData);
            GLint SwizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_RED };
            glTexParameteriv(m_textureTarget, GL_TEXTURE_SWIZZLE_RGBA, SwizzleMask);
        }
                break;

        case 2:
            glTexImage2D(m_textureTarget, 0, GL_RG, m_imageWidth, m_imageHeight, 0, GL_RG, GL_UNSIGNED_BYTE, pImageData);
            break;

        case 3:
            InternalFormat = IsSRGB ? GL_SRGB8 : GL_RGB8;
            glTexImage2D(m_textureTarget, 0, GL_RGB, m_imageWidth, m_imageHeight, 0, InternalFormat, GL_UNSIGNED_BYTE, pImageData);
            break;

        case 4:
            InternalFormat = IsSRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8;
            glTexImage2D(m_textureTarget, 0, GL_RGBA, m_imageWidth, m_imageHeight, 0, InternalFormat, GL_UNSIGNED_BYTE, pImageData);
            break;

        default:
            NOT_IMPLEMENTED;
        }
    } else {
        printf("Support for texture target %x is not implemented\n", m_textureTarget);
        exit(1);
    }

    glTexParameteri(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(m_textureTarget, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(m_textureTarget, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(m_textureTarget, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //glTexParameteri(m_textureTarget, GL_TEXTURE_WRAP_R, GL_REPEAT);

    glGenerateMipmap(m_textureTarget);

    glBindTexture(m_textureTarget, 0);
}

void Texture::LoadInternalDSA(const void* pImageData, bool IsSRGB)
{
    glCreateTextures(m_textureTarget, 1, &m_textureObj);

    int Levels = std::min(5, (int)log2f((float)std::max(m_imageWidth, m_imageHeight)));

    GLenum InternalFormat = GL_NONE;

    if (m_textureTarget == GL_TEXTURE_2D) {
        if (m_isKTX) {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
          //  int NumMipmaps = GetNumMipMapLevels2D(m_imageWidth, m_imageHeight);
            glTextureStorage2D(m_textureObj, Levels, m_ktxFormat.Internal, m_imageWidth, m_imageHeight);
            glTextureSubImage2D(m_textureObj, 0, 0, 0, m_imageWidth, m_imageHeight, m_ktxFormat.External, m_ktxFormat.Type, pImageData);
        } else {
            switch (m_imageBPP) {
            case 1: {
                glTextureStorage2D(m_textureObj, Levels, GL_R8, m_imageWidth, m_imageHeight);
                glTextureSubImage2D(m_textureObj, 0, 0, 0, m_imageWidth, m_imageHeight, GL_RED, GL_UNSIGNED_BYTE, pImageData);
                GLint SwizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_RED };
                glTextureParameteriv(m_textureObj, GL_TEXTURE_SWIZZLE_RGBA, SwizzleMask);
            }
                break;

            case 2:
                glTextureStorage2D(m_textureObj, Levels, GL_RG8, m_imageWidth, m_imageHeight);
                glTextureSubImage2D(m_textureObj, 0, 0, 0, m_imageWidth, m_imageHeight, GL_RG, GL_UNSIGNED_BYTE, pImageData);
                break;

            case 3:
                InternalFormat = IsSRGB ? GL_SRGB8 : GL_RGB8;
                glTextureStorage2D(m_textureObj, Levels, InternalFormat, m_imageWidth, m_imageHeight);
                glTextureSubImage2D(m_textureObj, 0, 0, 0, m_imageWidth, m_imageHeight, GL_RGB, GL_UNSIGNED_BYTE, pImageData);
                break;

            case 4:
                InternalFormat = IsSRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8;
                glTextureStorage2D(m_textureObj, Levels, InternalFormat, m_imageWidth, m_imageHeight);
                glTextureSubImage2D(m_textureObj, 0, 0, 0, m_imageWidth, m_imageHeight, GL_RGBA, GL_UNSIGNED_BYTE, pImageData);
                break;

            default:
                NOT_IMPLEMENTED;
            }
        }
    }
    else {
        printf("Support for texture target %x is not implemented\n", m_textureTarget);
        exit(1);
    }

    glTextureParameteri(m_textureObj, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(m_textureObj, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(m_textureObj, GL_TEXTURE_BASE_LEVEL, 0);
    glTextureParameteri(m_textureObj, GL_TEXTURE_MAX_LEVEL, Levels - 1);
    glTextureParameteri(m_textureObj, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_textureObj, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(m_textureObj, GL_TEXTURE_MAX_ANISOTROPY, 16);

    glGenerateTextureMipmap(m_textureObj);

    m_bindlessHandle = glGetTextureHandleARB(m_textureObj);
    glMakeTextureHandleResidentARB(m_bindlessHandle);
}


void Texture::LoadF32(int Width, int Height, const float* pImageData)
{
    if (!IsGLVersionHigher(4, 5)) {
        OGLDEV_ERROR0("Non DSA version is not implemented\n");
    }

    m_imageWidth = Width;
    m_imageHeight = Height;

    glCreateTextures(m_textureTarget, 1, &m_textureObj);
    glTextureStorage2D(m_textureObj, 1, GL_R32F, m_imageWidth, m_imageHeight);
    glTextureSubImage2D(m_textureObj, 0, 0, 0, m_imageWidth, m_imageHeight, GL_RED, GL_FLOAT, pImageData);

    glTextureParameteri(m_textureObj, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_textureObj, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameterf(m_textureObj, GL_TEXTURE_BASE_LEVEL, 0);
    glTextureParameteri(m_textureObj, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_textureObj, GL_TEXTURE_WRAP_T, GL_REPEAT);
}


void Texture::Bind(GLenum TextureUnit)
{
    if (IsGLVersionHigher(4, 5)) {
        BindInternalDSA(TextureUnit);
    } else {
        BindInternalNonDSA(TextureUnit);
    }
}


void Texture::BindInternalNonDSA(GLenum TextureUnit)
{
    glActiveTexture(TextureUnit);
    glBindTexture(m_textureTarget, m_textureObj);
}


void Texture::BindInternalDSA(GLenum TextureUnit)
{
    glBindTextureUnit(TextureUnit - GL_TEXTURE0, m_textureObj);
}
