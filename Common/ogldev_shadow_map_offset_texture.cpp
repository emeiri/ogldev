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


static void GenOffsetTextureData(int WindowSize, int FilterSize, std::vector<float>& Data)
{
    int BufferSize = WindowSize * WindowSize * FilterSize * FilterSize * 2;

    Data.resize(BufferSize);

    int Index = 0;

    for (int TexY = 0 ; TexY < WindowSize ; TexY++) {
        for (int TexX = 0 ; TexX < WindowSize ; TexX++) {
            for (int v = FilterSize - 1 ; v >= 0 ; v--) {
                for (int u = 0 ; u < FilterSize ; u++) {
                    float x = ((float)u + 0.5f + jitter()) / (float)FilterSize;
                    float y = ((float)v + 0.5f + jitter()) / (float)FilterSize;

                    assert(Index + 1 < Data.size());
                    Data[Index]     = sqrtf(y) * cosf(2 * (float)M_PI * x);
                    Data[Index + 1] = sqrtf(y) * sinf(2 * (float)M_PI * x);

                    Index += 2;
                }
            }
        }
    }
}


ShadowMapOffsetTexture::ShadowMapOffsetTexture(int WindowSize, int FilterSize)
{
    std::vector<float> Data;

    GenOffsetTextureData(WindowSize, FilterSize, Data);

    CreateTexture(WindowSize, FilterSize, Data);
}


void ShadowMapOffsetTexture::CreateTexture(int WindowSize, int FilterSize, const vector<float>& Data)
{
    int NumFilterSamples = FilterSize * FilterSize;

    glActiveTexture(GL_TEXTURE1);

    glGenTextures(1, &m_textureObj);

    glBindTexture(GL_TEXTURE_3D, m_textureObj);

    glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA32F, NumFilterSamples / 2, WindowSize, WindowSize );

    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, NumFilterSamples / 2, WindowSize, WindowSize, GL_RGBA, GL_FLOAT, &Data[0]);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_3D, 0);
}


void ShadowMapOffsetTexture::Bind(GLenum TextureUnit)
{
    glActiveTexture(TextureUnit);
    glBindTexture(GL_TEXTURE_3D, m_textureObj);
}
