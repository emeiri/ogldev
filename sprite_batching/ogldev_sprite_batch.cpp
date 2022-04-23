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
    m_numSpritesX = (float)NumSpritesX;
    m_numSpritesY = (float)NumSpritesY;
    m_windowWidth = (float)WindowWidth;
    m_windowHeight = (float)WindowHeight;
    m_windowAR = m_windowHeight / m_windowWidth;

    uint NumSprites = NumSpritesX * NumSpritesY;
    m_pQuads = new QuadArray(NumSprites);

    InitSpriteTech();

    InitSpriteSheet();

    CalcSpriteInfo();
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


void SpriteBatch::CalcSpriteInfo()
{
    int ImageWidth, ImageHeight;
    m_pSpriteSheet->GetImageSize(ImageWidth, ImageHeight);

    float SpriteWidth  = (float)ImageWidth / m_numSpritesX;
    float SpriteHeight = (float)ImageHeight / m_numSpritesY;

    printf("Sprite %f:%f\n", SpriteHeight, SpriteWidth);

    float NumSpritesForWidth = m_windowWidth / SpriteWidth;
    float NumSpritesForHeight = m_windowHeight / SpriteHeight;

    printf("%f %f\n", NumSpritesForWidth, NumSpritesForHeight);

    //    exit(0);;

    m_spriteAspectRatio = SpriteHeight / SpriteWidth;

    m_texVSize = 1.0f / 8.0f;
    m_texUSize = 1.0f / 6.0f;

    m_ndcPixelX = 2.0f / m_windowWidth;
    m_ndcPixelY = 2.0f / m_windowHeight;

    m_tileHeight = 0.0f;
    m_tileWidth = 0.0f;

    float ImageWidthToWindowWidthRatio = ImageWidth / m_windowWidth;
    float ImageHeightToWindowHeightRatio = ImageHeight / m_windowHeight;

    if (ImageWidthToWindowWidthRatio < ImageHeightToWindowHeightRatio) {
        m_tileHeight = m_windowHeight / m_numSpritesY;
        m_tileWidth = m_tileHeight / m_spriteAspectRatio;
    } else {
        m_tileWidth = m_windowWidth / m_numSpritesX;
        m_tileHeight = m_tileWidth * m_spriteAspectRatio;
    }

    m_tileWidthNDC = m_ndcPixelX * m_tileWidth;
    m_tileHeightNDC = m_ndcPixelY * m_tileHeight;
}


void SpriteBatch::MousePosToNDC(float mouse_x, float mouse_y, float& ndc_x, float& ndc_y)
{
    ndc_x = (2.0f * mouse_x) / m_windowWidth - 1.0f;
    ndc_y = (2.0f * mouse_y) / m_windowHeight - 1.0f;
}


void SpriteBatch::Render(const vector<SpriteInfo>& sprites)
{
    m_spriteTech.Enable();

    assert(sprites.size() < SPRITE_TECH_MAX_QUADS);

    float NDCX, NDCY;

    for (int SpriteIndex = 0 ; SpriteIndex < sprites.size() ; SpriteIndex++) {

        const SpriteInfo& Info = sprites[SpriteIndex];

        MousePosToNDC((float)Info.PixelX, (float)Info.PixelY, NDCX, NDCY);

        //        printf("NDC %f %f\n", NDCX, NDCY);

        float TileWidthNDC  = m_ndcPixelX * Info.SpriteWidth;
        float TileHeightNDC = TileWidthNDC / m_spriteAspectRatio;

        printf("Tile NDC %f %f\n", TileWidthNDC, TileHeightNDC);

        float UBase = (float)Info.SpriteCol * m_texUSize;
        float VBase = (float)Info.SpriteRow * m_texVSize;

        //        printf("U %f V %f\n", UBase, VBase);

        m_spriteTech.SetQuad(SpriteIndex,
                             NDCX, NDCY, TileWidthNDC, TileHeightNDC,
                             UBase, VBase, m_texUSize, m_texVSize);

    }

    m_pSpriteSheet->Bind(COLOR_TEXTURE_UNIT);
    m_pQuads->Render(sprites.size());
}


void SpriteBatch::RenderAll()
{
    m_spriteTech.Enable();

    for (int h = 0 ; h < (uint)m_numSpritesY ; h++) {
        for (int w = 0 ; w < (uint)m_numSpritesX ; w++) {
            uint TileIndex = h * m_numSpritesX + w;

            float PosX = w * m_tileWidth;
            float PosY = h * m_tileHeight;

            float NDCX, NDCY;
            MousePosToNDC(PosX, PosY, NDCX, NDCY);

            float UBase = w * m_texUSize;
            float VBase = h * m_texVSize;

            m_spriteTech.SetQuad(TileIndex,
                                 NDCX, NDCY, m_tileWidthNDC, m_tileHeightNDC,
                                 UBase, VBase, m_texUSize, m_texVSize);
        }
    }

    m_pSpriteSheet->Bind(COLOR_TEXTURE_UNIT);

    m_pQuads->Render();
}
