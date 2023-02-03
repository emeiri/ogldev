/*

        Copyright 2023 Etay Meiri

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


#ifndef TEXTURE_GENERATOR_H
#define TEXTURE_GENERATOR_H

#include <stdio.h>

#include "ogldev_texture.h"
#include "ogldev_stb_image.h"

struct TextureHeightDesc {
    float Low = 0.0f;
    float Optimal = 0.0f;
    float High = 0.0f;

    void Print() const { printf("Low %f Optimal %f High %f", Low, Optimal, High); }
};


struct TextureTile {
    STBImage Image;
    TextureHeightDesc HeightDesc;
};


class BaseTerrain;

class TextureGenerator {
 public:
    TextureGenerator();

    void LoadTile(const char* Filename);

    Texture* GenerateTexture(int TextureSize, BaseTerrain* pTerrain, float MinHeight, float MaxHeight);

 private:

    void CalculateTextureRegions(float MinHeight, float MaxHeight);

    float RegionPercent(int Tile, float Height);

    #define MAX_TEXTURE_TILES 4

    TextureTile m_textureTiles[MAX_TEXTURE_TILES] = {};
    int m_numTextureTiles = 0;
};

#endif
