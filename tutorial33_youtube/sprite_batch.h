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

#include "ogldev.h"
#include "ogldev_texture.h"
#include "ogldev_sprite_technique.h"
#include "quad_array.h"

class SpriteBatch
{
 public:

    struct SpriteInfo {
        uint PixelX = 0;
        uint PixelY = 0;
        uint SpriteRow = 0;    // Sprite location in the sprite sheet - row
        uint SpriteCol = 0;    // Sprite location in the sprite sheet - col
        uint SpriteWidth = 0;
    };

    SpriteBatch(const char* pFilename, uint NumSpritesX, uint NumSpritesY, uint WindowWidth, uint WindowHeight);

    void Render(const vector<SpriteInfo>& sprites);

    void RenderAll();

 private:

    void InitSpriteSheet();

    void InitSpriteTech();

    void CalcSpriteInfo();

    void ScreenPosToNDC(float mouse_x, float mouse_y, float& ndc_x, float& ndc_y);

    // constructor params
    const char* m_pFilename = NULL;
    float m_numSpritesX = 0.0f;
    float m_numSpritesY = 0.0f;
    float m_windowWidth = 0.0f;
    float m_windowHeight = 0.0f;

    // internal params
    float m_spriteAspectRatio = 0.0f;   // height/width of the sprite in the sprite sheet
    float m_texUSize = 0.0f;  // Sprite size in tex space - U
    float m_texVSize = 0.0f;  // Sprite size in tex space - V
    float m_windowAR = 0.0f;
    float m_ndcPixelX = 0.0f; // Size of pixel in NDC space - X
    float m_ndcPixelY = 0.0f; // Size of pixel in NDC space - y
    float m_tileHeightInPixels = 0.0f;
    float m_tileWidthInPixels  = 0.0f;
    float m_tileWidthNDC  = 0.0f;
    float m_tileHeightNDC = 0.0f;
    Texture* m_pSpriteSheet = NULL;
    QuadArray* m_pQuads = NULL;
    SpriteTechnique m_spriteTech;
};
