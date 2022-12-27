#include "midpoint_disp_terrain.h"

void MidpointDispTerrain::CreateMidpointDisplacement(int TerrainSize, float Roughness, float MinHeight, float MaxHeight)
{
    if (Roughness < 0.0f) {
        printf("%s: roughness must be positive - %f\n", __FUNCTION__, Roughness);
        exit(0);
    }

    m_terrainSize = TerrainSize;
    m_minHeight = MinHeight;
    m_maxHeight = MaxHeight;

    m_terrainTech.Enable();
    m_terrainTech.SetMinMaxHeight(MinHeight, MaxHeight);

    m_heightMap.InitArray2D(TerrainSize, TerrainSize, 0.0f);

    CreateMidpointDisplacementF32(Roughness);

    m_heightMap.Normalize(MinHeight, MaxHeight);

    m_triangleList.CreateTriangleList(m_terrainSize, m_terrainSize, this);
}


void MidpointDispTerrain::CreateMidpointDisplacementF32(float Roughness)
{
    int RectSize = m_terrainSize;
    float CurHeight = (float)RectSize / 2.0f;
    float HeightReduce = pow(2.0f, -Roughness);

    printf("Height reduce %f\n", HeightReduce);

    while (RectSize > 0) {
        printf("RectSize %d Height %f\n", RectSize, CurHeight);

        DiamondStep(RectSize, CurHeight);
        SquareStep(RectSize, CurHeight);

        printf("---------------\n");
        RectSize /= 2;
        CurHeight *= HeightReduce;
    }
}


void MidpointDispTerrain::DiamondStep(int RectSize, float CurHeight)
{
    for (int y = 0 ; y < m_terrainSize ; y += RectSize) {
        for (int x = 0 ; x < m_terrainSize ; x += RectSize) {
            int next_x = (x + RectSize) % m_terrainSize;
            int next_y = (y + RectSize) % m_terrainSize;

            float TopLeft     = m_heightMap.Get(x, y);
            float TopRight    = m_heightMap.Get(next_x, y);
            float BottomLeft  = m_heightMap.Get(x, next_y);
            float BottomRight = m_heightMap.Get(next_x, next_y);
            //  printf("(%d, %d): %f (%d, %d): %f\n", x, y, TopLeft, next_x, y, TopRight);
            //  printf("(%d, %d): %f (%d, %d): %f\n", x, next_y, BottomLeft, next_x, next_y, BottomRight);

            int mid_x = x + RectSize / 2;
            int mid_y = y + RectSize / 2;

            float RandValue = RandomFloatRange(CurHeight, -CurHeight);
            float MidPoint = (TopLeft + TopRight + BottomLeft + BottomRight) / 4.0f;
            // printf("Mid point (%d, %d): %f Rand %f\n", mid_x, mid_y, MidPoint, RandValue);
            m_heightMap.Set(mid_x, mid_y, MidPoint + RandValue);
        }
    }
}


void MidpointDispTerrain::SquareStep(int RectSize, float CurHeight)
{
    /*                ----------------------------------
                      |                                |
                      |           PrevYCenter          |
                      |                                |
                      |                                |
                      |                                |
    ------------------CurTopLeft..CurTopMid..CurTopRight
                      |                                |
                      |                                |
       CurPrevXCenter CurLeftMid   CurCenter           |
                      |                                |
                      |                                |
                      CurBotLeft------------------------

       CurTopMid = avg(PrevYCenter, CurTopLeft, CurTopRight, CurCenter)
       CurLeftMid = avg(CurPrevXCenterm CurTopleft, CurBotLeft, CurCenter)
    */

    int HalfRectSize = RectSize / 2;

    for (int y = 0 ; y < m_terrainSize ; y += RectSize) {
        for (int x = 0 ; x < m_terrainSize ; x += RectSize) {
            int next_x = (x + RectSize) % m_terrainSize;
            int next_y = (y + RectSize) % m_terrainSize;
            int mid_x = x + HalfRectSize;
            int mid_y = y + HalfRectSize;
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
