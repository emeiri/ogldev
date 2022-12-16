
#include "fault_formation_terrain.h"

void FaultFormationTerrain::CreateFaultFormation(int Size, int Iterations, float MinHeight, float MaxHeight, float Filter)
{
    if (MinHeight >= MaxHeight) {
        printf("%s: MinHeight (%f) must be less-than MaxHeight (%f\n)", __FUNCTION__, MinHeight, MaxHeight);
        assert(0);
    }

    m_terrainSize = Size;
    m_minHeight = MinHeight;
    m_maxHeight = MaxHeight;

    m_terrainTech.Enable();
    m_terrainTech.SetMinMaxHeight(MinHeight, MaxHeight);

    m_heightMap.InitArray2D(Size, Size, 0.0f);

    CreateFaultFormationInternal(Iterations, MinHeight, MaxHeight, Filter);

    m_heightMap.Normalize(MinHeight, MaxHeight);

    m_triangleList.CreateTriangleList(m_terrainSize, m_terrainSize, this);
}


void FaultFormationTerrain::CreateFaultFormationInternal(int Iterations, float MinHeight, float MaxHeight, float Filter)
{
    float DeltaHeight = MaxHeight - MinHeight;

    for (int CurIter = 0 ; CurIter < Iterations ; CurIter++) {
        float Height = MaxHeight - DeltaHeight * ((float)CurIter / (float)Iterations);

        TerrainPoint p1, p2;

        GenRandomTerrainPoints(p1, p2);

        int DirX = p2.x - p1.x;
        int DirZ = p2.z - p1.z;

        printf("Iter %d Height %f\n", CurIter, Height);
#ifdef DEBUG_PRINT
        printf("Random points: "); p1.Print(); printf(" --> "); p2.Print(); printf("\n");
        printf("   ");
        for (int i = 0 ; i < m_terrainSize ; i++) {
            printf("%4d ", i);
        }
        printf("\n");
#endif

        for (int z = 0 ; z < m_terrainSize ; z++) {
#ifdef DEBUG_PRINT
            printf("%d: ", z);
#endif
            for (int x = 0 ; x < m_terrainSize ; x++) {
                int DirX_in = x - p1.x;
                int DirZ_in = z - p1.z;

                int CrossProduct = DirX_in * DirZ - DirX * DirZ_in;
#ifdef DEBUG_PRINT
                //                printf("%4d ", CrossProduct);
#endif
                if (CrossProduct > 0) {
                    float CurHeight = m_heightMap.Get(x, z);
                    m_heightMap.Set(x, z, CurHeight + Height);
                }

#ifdef DEBUG_PRINT
                printf("%f ", m_heightMap.Get(x, z));
#endif
            }
#ifdef DEBUG_PRINT
            printf("\n");
#endif
            ApplyFIRFilter(Filter);
        }
    }
}


void FaultFormationTerrain::ApplyFIRFilter(float Filter)
{
    // left to right
    for (int z = 0 ; z < m_terrainSize ; z++) {
        float PrevFractalVal = m_heightMap.Get(0, z);
        for (int x = 1 ; x < m_terrainSize ; x++) {
            PrevFractalVal = FIRFilterSinglePoint(x, z, PrevFractalVal, Filter);
        }
    }

    // right to left
    for (int z = 0 ; z < m_terrainSize ; z++) {
        float PrevFractalVal = m_heightMap.Get(m_terrainSize - 1, z);
        for (int x = m_terrainSize - 2 ; x >= 0 ; x--) {
            PrevFractalVal = FIRFilterSinglePoint(x, z, PrevFractalVal, Filter);
        }
    }

    // top to bottom
    for (int x = 0 ; x < m_terrainSize ; x++) {
        float PrevFractalVal = m_heightMap.Get(x, 0);
        for (int z = 1 ; z < m_terrainSize ; z++) {
            PrevFractalVal = FIRFilterSinglePoint(x, z, PrevFractalVal, Filter);
        }
    }

    // bottom to top
    for (int x = 0 ; x < m_terrainSize ; x++) {
        float PrevFractalVal = m_heightMap.Get(x, m_terrainSize - 1);
        for (int z = m_terrainSize - 2 ; z >= 0 ; z--) {
            PrevFractalVal = FIRFilterSinglePoint(x, z, PrevFractalVal, Filter);
        }
    }
}


float FaultFormationTerrain::FIRFilterSinglePoint(int x, int z, float PrevFractalVal, float Filter)
{
    float CurFractalVal = m_heightMap.Get(x, z);
    float NewVal = Filter * PrevFractalVal + (1 - Filter) * CurFractalVal;
    m_heightMap.Set(x, z, NewVal);
    PrevFractalVal = NewVal;
    return PrevFractalVal;
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
