#ifndef FAULT_FORMATION_TERRAIN_H
#define FAULT_FORMATION_TERRAIN_H

#include "terrain.h"

class FaultFormationTerrain : public BaseTerrain {

 public:
    FaultFormationTerrain() {}

    void CreateFaultFormation(int TerrainSize, int Iterations, float MinHeight, float MaxHeight, float Filter);

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

    void CreateFaultFormationInternal(int Iterations, float MinHeight, float MaxHeight, float Filter);
    void GenRandomTerrainPoints(TerrainPoint& p1, TerrainPoint& p2);
    void ApplyFIRFilter(float Filter);
    void ApplyFIRFilterBand(int Start, int Stride, int Count, float Filter);
    float FIRFilterSinglePoint(int x, int z, float PrevFractalVal, float Filter);
};

#endif
