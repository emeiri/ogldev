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


#include "ogldev_sprite_batch.h"
#include "ogldev_engine_common.h"


SpriteBatch::SpriteBatch(const char* pFilename, uint NumSpritesX, uint NumSpritesY, uint WindowWidth, uint WindowHeight)
{
    m_pFilename = pFilename;
    m_windowWidth = (float)WindowWidth;
    m_windowHeight = (float)WindowHeight;
    m_windowAR = m_windowHeight / m_windowWidth;

    uint NumSprites = NumSpritesX * NumSpritesY;
    m_pQuads = new QuadArray(NumSprites);

    InitSpriteTech();

    InitSpriteSheet();
}


void SpriteBatch::InitSpriteTech()
{
    if (!m_spriteTech.Init()) {
        printf("Error initializing the sprite technique\n");
        exit(1);
    }

    m_spriteTech.Enable();
    m_spriteTech.SetTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
}


void SpriteBatch::InitSpriteSheet()
{
    m_pSpriteSheet = new Texture(GL_TEXTURE_2D, m_pFilename);

    if (!m_pSpriteSheet->Load()) {
        printf("Error loading sprite sheet\n");
    }
}


void SpriteBatch::MousePosToNDC(float mouse_x, float mouse_y, float& ndc_x, float& ndc_y)
{
    ndc_x = (2.0f * mouse_x) / m_windowWidth - 1.0f;
    ndc_y = (2.0f * mouse_y) / m_windowHeight - 1.0f;
}


void SpriteBatch::Render()
{
    m_spriteTech.Enable();

    float SpriteHeight = 3792.0 / 8.0f;
    float SpriteWidth = 4092.0f / 6.0f;

    //        printf("Sprite %f:%f\n", SpriteHeight, SpriteWidth);

    float SpriteAspectRatio = SpriteHeight / SpriteWidth;

    float TileHeightNorm = 1.0f / 8.0f;
    float TileWidthNorm = 1.0f / 6.0f;

    //        printf("Tile %f:%f\n", TileHeightNorm, TileWidthNorm);

    float NDCPixelX = 2.0f / m_windowWidth;
    float NDCPixelY = 2.0f / m_windowHeight;

    float XStart = 0.0f;
    float YStart = 0.0f;
    float Stride = 150.0f;
    float TileWidth = 150.0f;
    float TileHeight = TileWidth * SpriteAspectRatio;

    float TileWidthNDC = NDCPixelX * TileWidth;
    float TileHeightNDC = NDCPixelY * TileHeight;

    //        printf("TileWidthNDC %f TileHeightNDC %f\n", TileWidthNDC, TileHeightNDC);

    for (int h = 0 ; h < 8 ; h++) {
        for (int w = 0 ; w < 6 ; w++) {
            uint TileIndex = h * 6 + w;
            //                float TilePosX = w * TileWidthNorm;
            //                float TilePosY = h * TileHeightNorm;

            float PosX = XStart + w * Stride;
            float PosY = YStart + h * Stride;

            float NDCX, NDCY;
            MousePosToNDC(PosX, PosY, NDCX, NDCY);

            float UBase = w * TileWidthNorm;
            float VBase = h * TileHeightNorm;
            //                printf("pos %f,%f\n", TilePosX, TilePosY);

            m_spriteTech.SetQuad(TileIndex,
                                   NDCX, NDCY, TileWidthNDC, TileHeightNDC,
                                   UBase, VBase, TileWidthNorm, TileHeightNorm);
        }
    }

    /*        m_spriteEffect.SetQuad(0,
              -0.5f, -0.5f, 0.5f * ar, 0.5f,
              0.0f, 0.0f, 0.1f * ar, 0.1f);
              m_spriteEffect.SetQuad(1,
              0.0f, 0.0f, 0.1f * ar, 0.1f,
              0.0f, 0.0f, 0.1f * ar, 0.1f);*/
    m_pSpriteSheet->Bind(GL_TEXTURE0);

    m_pQuads->Render();
}
