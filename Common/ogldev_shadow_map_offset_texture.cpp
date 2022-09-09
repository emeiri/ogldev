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

float jitter()
{
    static std::default_random_engine generator;
    static std::uniform_real_distribution<float> distrib(-0.5f, 0.5f);
    return distrib(generator);
}


static void GenOffsetTextureData(int TextureSize, int FilterSize, std::vector<float>& Data)
{
    int NumFilterSamples = FilterSize * FilterSize;
    int BufferSize = TextureSize * TextureSize * NumFilterSamples * 2;

    Data.resize(BufferSize);

    int Index = 0;

    for (int TexY = 0 ; TexY < TextureSize ; TexY++) {
        for (int TexX = 0 ; TexX < TextureSize ; TexX++) {
            for (int s = 0 ; s < NumFilterSamples ; s += 2) {
                int u1 = s % FilterSize;
                int v1 = (NumFilterSamples - 1 - s) / FilterSize;
                int u2 = (s + 1) % FilterSize;
                int v2 = (NumFilterSamples - 1 - s - 1) / FilterSize;

                printf("%d %d\n", u1, v1);
                printf("%d %d\n", u2, v2);
                //                printf("%d %d %d %d\n", x1, y1, x2, y2);

                float f1 = (u1 + 0.5f + jitter()) / FilterSize;
                float f2 = (v1 + 0.5f + jitter()) / FilterSize;
                float f3 = (u2 + 0.5f + jitter()) / FilterSize;
                float f4 = (v2 + 0.5f + jitter()) / FilterSize;

                //printf("%f %f %f %f\n", f1, f2, f3, f4);

                //printf("%f %f\n", f1, f2);
                //printf("%f %f\n", f3, f4);

                Data[Index]     = sqrtf(f2) * cosf(2 * M_PI * f1);
                Data[Index + 1] = sqrtf(f2) * sinf(2 * M_PI * f1);
                Data[Index + 2] = sqrtf(f4) * cosf(2 * M_PI * f3);
                Data[Index + 3] = sqrtf(f4) * sinf(2 * M_PI * f3);

                //printf("%d %d %d: %f %f %f %f\n", y, x, s, Data[index], Data[index + 1], Data[index + 2], Data[index + 3]);

                printf("%f %f\n", Data[Index], Data[Index + 1]);
                printf("%f %f\n", Data[Index + 2], Data[Index + 3]);

                Index += 4;
            }
        }
    }
}


ShadowMapOffsetTexture::ShadowMapOffsetTexture(int TextureSize, int FilterSize)
{
    std::vector<float> Data;

    GenOffsetTextureData(TextureSize, FilterSize, Data);

    CreateTexture(TextureSize, FilterSize, Data);
}


void ShadowMapOffsetTexture::CreateTexture(int TextureSize, int FilterSize, const vector<float>& Data)
{
    int NumFilterSamples = FilterSize * FilterSize;

    glActiveTexture(GL_TEXTURE1);

    glGenTextures(1, &m_textureObj);

    glBindTexture(GL_TEXTURE_3D, m_textureObj);

    glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA32F, NumFilterSamples / 2, TextureSize, TextureSize );

    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, NumFilterSamples / 2, TextureSize, TextureSize, GL_RGBA, GL_FLOAT, &Data[0]);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_3D, 0);
}


void ShadowMapOffsetTexture::Bind(GLenum TextureUnit)
{
    glActiveTexture(TextureUnit);
    glBindTexture(GL_TEXTURE_3D, m_textureObj);
}
