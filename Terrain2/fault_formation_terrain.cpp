
#include "fault_formation_terrain.h"

void FaultFormationTerrain::CreateFaultFormation(int TerrainSize, int Iterations, float MinHeight, float MaxHeight)
{
    if (MinHeight >= MaxHeight) {
        printf("%s: MinHeight (%f) must be less-than MaxHeight (%f\n)", __FUNCTION__, MinHeight, MaxHeight);
        assert(0);
    }

    m_terrainSize = TerrainSize;
    m_minHeight = MinHeight;
    m_maxHeight = MaxHeight;

    m_heightMap.InitArray2D(TerrainSize, TerrainSize, 0.0f);

    CreateFaultFormationInternal(Iterations, MinHeight, MaxHeight);

    m_triangleList.CreateTriangleList(m_terrainSize, m_terrainSize, this);
}


void FaultFormationTerrain::CreateFaultFormationInternal(int Iterations, float MinHeight, float MaxHeight)
{
    float DeltaHeight = MaxHeight - MinHeight;

    for (int CurIter = 0 ; CurIter < Iterations ; CurIter++) {
        float IterationRatio = ((float)CurIter / (float)Iterations);
        float Height = MaxHeight - IterationRatio * DeltaHeight;

        TerrainPoint p1, p2;

        GenRandomTerrainPoints(p1, p2);

        int DirX = p2.x - p1.x;
        int DirZ = p2.z - p1.z;

        for (int z = 0 ; z < m_terrainSize ; z++) {
            for (int x = 0 ; x < m_terrainSize ; x++) {
                int DirX_in = x - p1.x;
                int DirZ_in = z - p1.z;

                int CrossProduct = DirX_in * DirZ - DirX * DirZ_in;

                if (CrossProduct > 0) {
                    float CurHeight = m_heightMap.Get(x, z);
                    m_heightMap.Set(x, z, CurHeight + Height);
                }
            }
#ifdef DEBUG_PRINT
            printf("\n");
#endif
            
        }        
    }
}


void FaultFormationTerrain::GenRandomTerrainPoints(TerrainPoint& p1, TerrainPoint& p2)
{
    p1.x = rand() % m_terrainSize;
    p1.z = rand() % m_terrainSize;

    int Counter = 0;

    do {
        p2.x = rand() % m_terrainSize;
        p2.z = rand() % m_terrainSize;

        if (Counter++ == 1000) {
            printf("Endless loop detected in %s:%d\n", __FILE__, __LINE__);
            assert(0);
        }
    } while (p1.IsEqual(p2));
}
