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

#include <vector>
#include <math.h>
#include <random>

#include "ogldev_math_3d.h"
#include "ogldev_shadow_map_offset_texture.h"

//float jitter = 1.0f;

float jitter()
{
    static std::default_random_engine generator;
    static std::uniform_real_distribution<float> distrib(-0.5f, 0.5f);
    return distrib(generator);
}


static void GenOffsetTextureData(int Size, int NumSamplesU, int NumSamplesV, std::vector<float>& Data)
{
    int TotalNumSamples = NumSamplesU * NumSamplesV;
    int BufferSize = Size * Size * TotalNumSamples * 2;

    Data.resize(BufferSize);

    for (int y = 0 ; y < Size ; y++) {
        for (int x = 0 ; x < Size ; x++) {
            for (int s = 0 ; s < TotalNumSamples ; s += 2) {
                int x1 = s % NumSamplesU;
                int y1 = (TotalNumSamples - 1 - s) / NumSamplesU;
                int x2 = (s + 1) % NumSamplesU;
                int y2 = (TotalNumSamples - 1 - s - 1) / NumSamplesU;

                //                printf("%d %d %d %d\n", x1, y1, x2, y2);

                float f1 = (x1 + 0.5f + jitter()) / NumSamplesU;
                float f2 = (y1 + 0.5f + jitter()) / NumSamplesV;
                float f3 = (x2 + 0.5f + jitter()) / NumSamplesU;
                float f4 = (y2 + 0.5f + jitter()) / NumSamplesV;

                //printf("%f %f %f %f\n", f1, f2, f3, f4);

                int index = ((s / 2) * Size * Size + x * Size + y) * 4;

                //                printf("index %d\n", index);

                Data[index]     = sqrtf(f2) * cosf(2 * M_PI * f1);
                Data[index + 1] = sqrtf(f2) * sinf(2 * M_PI * f1);
                Data[index + 2] = sqrtf(f4) * cosf(2 * M_PI * f3);
                Data[index + 3] = sqrtf(f4) * sinf(2 * M_PI * f3);

                //printf("%d %d %d: %f %f %f %f\n", y, x, s, Data[index], Data[index + 1], Data[index + 2], Data[index + 3]);
            }
        }
    }
}


ShadowMapOffsetTexture::ShadowMapOffsetTexture(int Size, int NumSamplesU, int NumSamplesV)
{
    std::vector<float> Data;

    GenOffsetTextureData(Size, NumSamplesU, NumSamplesV, Data);

    int TotalNumSamples = NumSamplesU * NumSamplesV;

    CreateTexture(Size, TotalNumSamples, Data);
}


void ShadowMapOffsetTexture::CreateTexture(int Size, int TotalNumSamples, const vector<float>& Data)
{
    glActiveTexture(GL_TEXTURE1);

    glGenTextures(1, &m_textureObj);

    glBindTexture(GL_TEXTURE_3D, m_textureObj);

    glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA32F, Size, Size, TotalNumSamples / 2);

    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, Size, Size, TotalNumSamples / 2, GL_RGBA, GL_FLOAT, &Data[0]);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_3D, 0);
}


void ShadowMapOffsetTexture::Bind(GLenum TextureUnit)
{
    glActiveTexture(TextureUnit);
    glBindTexture(GL_TEXTURE_3D, m_textureObj);
}
