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

#include "midpoint_disp_terrain.h"

void MidpointDispTerrain::CreateMidpointDisplacement(int TerrainSize, float Roughness, float MinHeight, float MaxHeight)
{
    if (Roughness < 0.0f) {
        printf("%s: roughness must be positive - %f\n", __FUNCTION__, Roughness);
        exit(0);
    }

    m_terrainSize = TerrainSize;

    SetMinMaxHeight(MinHeight, MaxHeight);

    m_heightMap.InitArray2D(TerrainSize, TerrainSize, 0.0f);

    CreateMidpointDisplacementF32(Roughness);

    m_heightMap.Normalize(MinHeight, MaxHeight);

    m_triangleList.CreateTriangleList(m_terrainSize, m_terrainSize, this);
}


void MidpointDispTerrain::CreateMidpointDisplacementF32(float Roughness)
{
    int RectSize = CalcNextPowerOfTwo(m_terrainSize);
    float CurHeight = (float)RectSize / 2.0f;
    float HeightReduce = pow(2.0f, -Roughness);

    while (RectSize > 0) {

        DiamondStep(RectSize, CurHeight);

        SquareStep(RectSize, CurHeight);

        RectSize /= 2;
        CurHeight *= HeightReduce;
    }
}


void MidpointDispTerrain::DiamondStep(int RectSize, float CurHeight)
{
    int HalfRectSize = RectSize / 2;

    for (int y = 0 ; y < m_terrainSize ; y += RectSize) {
        for (int x = 0 ; x < m_terrainSize ; x += RectSize) {
            int next_x = (x + RectSize) % m_terrainSize;
            int next_y = (y + RectSize) % m_terrainSize;

            if (next_x < x) {
                next_x = m_terrainSize - 1;
            }

            if (next_y < y) {
                next_y = m_terrainSize - 1;
            }

            float TopLeft     = m_heightMap.Get(x, y);
            float TopRight    = m_heightMap.Get(next_x, y);
            float BottomLeft  = m_heightMap.Get(x, next_y);
            float BottomRight = m_heightMap.Get(next_x, next_y);

            int mid_x = (x + HalfRectSize) % m_terrainSize;
            int mid_y = (y + HalfRectSize) % m_terrainSize;

            float RandValue = RandomFloatRange(CurHeight, -CurHeight);
            float MidPoint = (TopLeft + TopRight + BottomLeft + BottomRight) / 4.0f;

            m_heightMap.Set(mid_x, mid_y, MidPoint + RandValue);
        }
    }
}


void MidpointDispTerrain::SquareStep(int RectSize, float CurHeight)
{
    int HalfRectSize = RectSize / 2;

    for (int y = 0 ; y < m_terrainSize ; y += RectSize) {
        for (int x = 0 ; x < m_terrainSize ; x += RectSize) {
            int next_x = (x + RectSize) % m_terrainSize;
            int next_y = (y + RectSize) % m_terrainSize;

            if (next_x < x) {
                next_x = m_terrainSize - 1;
            }

            if (next_y < y) {
                next_y = m_terrainSize - 1;
            }

            int mid_x = (x + HalfRectSize) % m_terrainSize;
            int mid_y = (y + HalfRectSize) % m_terrainSize;
              
            int prev_mid_x = (x - HalfRectSize + m_terrainSize) % m_terrainSize;
            int prev_mid_y = (y - HalfRectSize + m_terrainSize) % m_terrainSize;

            float CurTopLeft  = m_heightMap.Get(x, y);
            float CurTopRight = m_heightMap.Get(next_x, y);
            float CurCenter   = m_heightMap.Get(mid_x, mid_y);
            float PrevYCenter = m_heightMap.Get(mid_x, prev_mid_y);
            float CurBotLeft  = m_heightMap.Get(x, next_y);
            float PrevXCenter = m_heightMap.Get(prev_mid_x, mid_y);

            float CurLeftMid = (CurTopLeft + CurCenter + CurBotLeft + PrevXCenter) / 4.0f + RandomFloatRange(-CurHeight, CurHeight);
            float CurTopMid  = (CurTopLeft + CurCenter + CurTopRight + PrevYCenter) / 4.0f + RandomFloatRange(-CurHeight, CurHeight);

            m_heightMap.Set(mid_x, y, CurTopMid);
            m_heightMap.Set(x, mid_y, CurLeftMid);
        }
    }
}
