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


#include "fault_formation_terrain.h"

void FaultFormationTerrain::CreateFaultFormation(int TerrainSize, int Iterations, float MinHeight, float MaxHeight)
{  
    m_terrainSize = TerrainSize;
    m_minHeight = MinHeight;
    m_maxHeight = MaxHeight;

    m_terrainTech.Enable();
    m_terrainTech.SetMinMaxHeight(MinHeight, MaxHeight);

    m_heightMap.InitArray2D(TerrainSize, TerrainSize, 0.0f);

    CreateFaultFormationInternal(Iterations, MinHeight, MaxHeight);

    m_heightMap.Normalize(MinHeight, MaxHeight);

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
