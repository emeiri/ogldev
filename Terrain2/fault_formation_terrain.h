#ifndef FAULT_FORMATION_TERRAIN_H
#define FAULT_FORMATION_TERRAIN_H

#include "terrain.h"

class FaultFormationTerrain : public BaseTerrain {

 public:
    FaultFormationTerrain() {}

    void CreateFaultFormation(int TerrainSize, int Iterations, float MinHeight, float MaxHeight);

 private:

     struct TerrainPoint {
         int x = 0;
         int z = 0;

         void Print()
         {
             printf("[%d,%d]", x, z);
         }

         bool IsEqual(TerrainPoint& p) const
         {
             return ((x == p.x) && (z == p.z));
         }
     };

    void CreateFaultFormationInternal(int Iterations, float MinHeight, float MaxHeight);
    void GenRandomTerrainPoints(TerrainPoint& p1, TerrainPoint& p2);
};

#endif
