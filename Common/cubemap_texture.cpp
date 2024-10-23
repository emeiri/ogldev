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

#include "ogldev_cubemap_texture.h"
#include "ogldev_util.h"
#include "3rdparty/stb_image.h"

static const GLenum types[6] = {  GL_TEXTURE_CUBE_MAP_POSITIVE_X,
                                  GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
                                  GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
                                  GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
                                  GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
                                  GL_TEXTURE_CUBE_MAP_NEGATIVE_Z };


template <typename T>
T clamp(T v, T a, T b)
{
    if (v < a) return a;
    if (v > b) return b;
    return v;
}


glm::vec3 FaceCoordsToXYZ(int i, int j, int FaceID, int FaceSize)
{
    float A = 2.0f * float(i) / FaceSize;
    float B = 2.0f * float(j) / FaceSize;

    glm::vec3 Ret;

    switch (FaceID) {
    case 0:
        Ret = glm::vec3(-1.0f, A - 1.0f, B - 1.0f);
        break;

    case 1:
        Ret = glm::vec3(A - 1.0f, -1.0f, 1.0f - B);
        break;

    case 2:
        Ret = glm::vec3(1.0f, A - 1.0f, 1.0f - B);
        break;

    case 3:
        Ret = glm::vec3(1.0f - A, 1.0f, 1.0f - B);
        break;

    case 4:
        Ret = glm::vec3(B - 1.0f, A - 1.0f, 1.0f);
        break;

    case 5:
        Ret = glm::vec3(1.0f - B, A - 1.0f, -1.0f);
        break;

    default:
        assert(0);
    }

    return Ret;
}

enum eBitmapType
{
    eBitmapType_2D,
    eBitmapType_Cube
};

enum eBitmapFormat
{
    eBitmapFormat_UnsignedByte,
    eBitmapFormat_Float,
};


struct Bitmap
{
    Bitmap() = default;
    Bitmap(int w, int h, int comp, eBitmapFormat fmt)
        :w_(w), h_(h), comp_(comp), fmt_(fmt), data_(w* h* comp* getBytesPerComponent(fmt))
    {
        initGetSetFuncs();
    }
    Bitmap(int w, int h, int d, int comp, eBitmapFormat fmt)
        :w_(w), h_(h), d_(d), comp_(comp), fmt_(fmt), data_(w* h* d* comp* getBytesPerComponent(fmt))
    {
        initGetSetFuncs();
    }
    Bitmap(int w, int h, int comp, eBitmapFormat fmt, void* ptr)
        :w_(w), h_(h), comp_(comp), fmt_(fmt), data_(w* h* comp* getBytesPerComponent(fmt))
    {
        initGetSetFuncs();
        memcpy(data_.data(), ptr, data_.size());
    }
    int w_ = 0;
    int h_ = 0;
    int d_ = 1;
    int comp_ = 3;
    eBitmapFormat fmt_ = eBitmapFormat_UnsignedByte;
    eBitmapType type_ = eBitmapType_2D;
    std::vector<uint8_t> data_;

    static int getBytesPerComponent(eBitmapFormat fmt)
    {
        if (fmt == eBitmapFormat_UnsignedByte) return 1;
        if (fmt == eBitmapFormat_Float) return 4;
        return 0;
    }

    void setPixel(int x, int y, glm::vec4& c)
    {
        (*this.*setPixelFunc)(x, y, c);
    }
    glm::vec4 getPixel(int x, int y)
    {
        return ((*this.*getPixelFunc)(x, y));
    }
private:
    using setPixel_t = void(Bitmap::*)(int, int, glm::vec4&);
    using getPixel_t = glm::vec4(Bitmap::*)(int, int);
    setPixel_t setPixelFunc = &Bitmap::setPixelUnsignedByte;
    getPixel_t getPixelFunc = &Bitmap::getPixelUnsignedByte;

    void initGetSetFuncs()
    {
        switch (fmt_)
        {
        case eBitmapFormat_UnsignedByte:
            setPixelFunc = &Bitmap::setPixelUnsignedByte;
            getPixelFunc = &Bitmap::getPixelUnsignedByte;
            break;
        case eBitmapFormat_Float:
            setPixelFunc = &Bitmap::setPixelFloat;
            getPixelFunc = &Bitmap::getPixelFloat;
            break;
        }
    }

    void setPixelFloat(int x, int y, glm::vec4& c)
    {
        int ofs = comp_ * (y * w_ + x);
        float* data = reinterpret_cast<float*>(data_.data());
        if (comp_ > 0) data[ofs + 0] = c.x;
        if (comp_ > 1) data[ofs + 1] = c.y;
        if (comp_ > 2) data[ofs + 2] = c.z;
        if (comp_ > 3) data[ofs + 3] = c.w;
    }
    glm::vec4 getPixelFloat(int x, int y)
    {
        int ofs = comp_ * (y * w_ + x);
        float* data = reinterpret_cast<float*>(data_.data());
        return glm::vec4(
            comp_ > 0 ? data[ofs + 0] : 0.0f,
            comp_ > 1 ? data[ofs + 1] : 0.0f,
            comp_ > 2 ? data[ofs + 2] : 0.0f,
            comp_ > 3 ? data[ofs + 3] : 0.0f);
    }

    void setPixelUnsignedByte(int x, int y, glm::vec4& c)
    {
        int ofs = comp_ * (y * w_ + x);
        if (comp_ > 0) data_[ofs + 0] = uint8_t(c.x * 255.0f);
        if (comp_ > 1) data_[ofs + 1] = uint8_t(c.y * 255.0f);
        if (comp_ > 2) data_[ofs + 2] = uint8_t(c.z * 255.0f);
        if (comp_ > 3) data_[ofs + 3] = uint8_t(c.w * 255.0f);
    }
    glm::vec4 getPixelUnsignedByte(int x, int y)
    {
        int ofs = comp_ * (y * w_ + x);
        return glm::vec4(
            comp_ > 0 ? float(data_[ofs + 0]) / 255.0f : 0.0f,
            comp_ > 1 ? float(data_[ofs + 1]) / 255.0f : 0.0f,
            comp_ > 2 ? float(data_[ofs + 2]) / 255.0f : 0.0f,
            comp_ > 3 ? float(data_[ofs + 3]) / 255.0f : 0.0f);
    }
};


Bitmap convertEquirectangularMapToVerticalCross(Bitmap& b)
{
    if (b.type_ != eBitmapType_2D) return Bitmap();

    int faceSize = b.w_ / 4;

    int w = faceSize * 3;
    int h = faceSize * 4;

    Bitmap result(w, h, b.comp_, b.fmt_);

    glm::ivec2 kFaceOffsets[] =
    {
        glm::ivec2(faceSize, faceSize * 3),
        glm::ivec2(0, faceSize),
        glm::ivec2(faceSize, faceSize),
        glm::ivec2(faceSize * 2, faceSize),
        glm::ivec2(faceSize, 0),
        glm::ivec2(faceSize, faceSize * 2)
    };

    int clampW = b.w_ - 1;
    int clampH = b.h_ - 1;

    for (int face = 0; face != 6; face++)
    {
        for (int i = 0; i != faceSize; i++)
        {
            for (int j = 0; j != faceSize; j++)
            {
                glm::vec3 P = FaceCoordsToXYZ(i, j, face, faceSize);
                float R = hypot(P.x, P.y);
                float theta = atan2f(P.y, P.x);
                float phi = atan2f(P.z, R);
                //	float point source coordinates
                float Uf = float(2.0f * faceSize * (theta + M_PI) / M_PI);
                float Vf = float(2.0f * faceSize * (M_PI / 2.0f - phi) / M_PI);
                // 4-samples for bilinear interpolation
                int U1 = clamp(int(floor(Uf)), 0, clampW);
                int V1 = clamp(int(floor(Vf)), 0, clampH);
                int U2 = clamp(U1 + 1, 0, clampW);
                int V2 = clamp(V1 + 1, 0, clampH);
                // fractional part
                float s = Uf - U1;
                float t = Vf - V1;
                // fetch 4-samples
                glm::vec4 A = b.getPixel(U1, V1);
                glm::vec4 B = b.getPixel(U2, V1);
                glm::vec4 C = b.getPixel(U1, V2);
                glm::vec4 D = b.getPixel(U2, V2);
                // bilinear interpolation
                glm::vec4 color = A * (1 - s) * (1 - t) + B * (s) * (1 - t) + C * (1 - s) * t + D * (s) * (t);
                result.setPixel(i + kFaceOffsets[face].x, j + kFaceOffsets[face].y, color);
            }
        };
    }

    return result;
}



Bitmap convertVerticalCrossToCubeMapFaces(const Bitmap& b)
{
    int faceWidth = b.w_ / 3;
    int faceHeight = b.h_ / 4;

    Bitmap cubemap(faceWidth, faceHeight, 6, b.comp_, b.fmt_);
    cubemap.type_ = eBitmapType_Cube;

    const uint8_t* src = b.data_.data();
    uint8_t* dst = cubemap.data_.data();

    /*
            ------
            | +Y |
     ----------------
     | -X | -Z | +X |
     ----------------
            | -Y |
            ------
            | +Z |
            ------
    */

    int pixelSize = cubemap.comp_ * Bitmap::getBytesPerComponent(cubemap.fmt_);

    for (int face = 0; face != 6; ++face)
    {
        for (int j = 0; j != faceHeight; ++j)
        {
            for (int i = 0; i != faceWidth; ++i)
            {
                int x = 0;
                int y = 0;

                switch (face)
                {
                    // GL_TEXTURE_CUBE_MAP_POSITIVE_X
                case 0:
                    x = i;
                    y = faceHeight + j;
                    break;

                    // GL_TEXTURE_CUBE_MAP_NEGATIVE_X
                case 1:
                    x = 2 * faceWidth + i;
                    y = 1 * faceHeight + j;
                    break;

                    // GL_TEXTURE_CUBE_MAP_POSITIVE_Y
                case 2:
                    x = 2 * faceWidth - (i + 1);
                    y = 1 * faceHeight - (j + 1);
                    break;

                    // GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
                case 3:
                    x = 2 * faceWidth - (i + 1);
                    y = 3 * faceHeight - (j + 1);
                    break;

                    // GL_TEXTURE_CUBE_MAP_POSITIVE_Z
                case 4:
                    x = 2 * faceWidth - (i + 1);
                    y = b.h_ - (j + 1);
                    break;

                    // GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
                case 5:
                    x = faceWidth + i;
                    y = faceHeight + j;
                    break;
                }

                memcpy(dst, src + (y * b.w_ + x) * pixelSize, pixelSize);

                dst += pixelSize;
            }
        }
    }

    return cubemap;
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
//    stbi_set_flip_vertically_on_load(0);

    int w, h, comp;
    const float* img = stbi_loadf(m_filename.c_str(), & w, & h, & comp, 3);
    Bitmap in(w, h, comp, eBitmapFormat_Float, (void*)img);
    Bitmap out = convertEquirectangularMapToVerticalCross(in);
    stbi_image_free((void*)img);

    //stbi_write_hdr("screenshot.hdr", out.w_, out.h_, out.comp_, (const float*)out.data_.data());

    Bitmap cubemap = convertVerticalCrossToCubeMapFaces(out);

    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_textureObj);
    glTextureParameteri(m_textureObj, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_textureObj, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_textureObj, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_textureObj, GL_TEXTURE_BASE_LEVEL, 0);
    glTextureParameteri(m_textureObj, GL_TEXTURE_MAX_LEVEL, 0);
    glTextureParameteri(m_textureObj, GL_TEXTURE_MAX_LEVEL, 0);
    glTextureParameteri(m_textureObj, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_textureObj, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureStorage2D(m_textureObj, 1, GL_RGB32F, cubemap.w_, cubemap.h_);
    const uint8_t* data = cubemap.data_.data();

    for (unsigned i = 0; i != 6; ++i)
    {
        glTextureSubImage3D(m_textureObj, 0, 0, 0, i, cubemap.w_, cubemap.h_, 1, GL_RGB, GL_FLOAT, data);
        data += cubemap.w_ * cubemap.h_ * cubemap.comp_ * Bitmap::getBytesPerComponent(cubemap.fmt_);
    }
}


void CubemapEctTexture::Bind(GLenum TextureUnit)
{
    glBindTextures(TextureUnit - GL_TEXTURE0, 1, &m_textureObj);
}
