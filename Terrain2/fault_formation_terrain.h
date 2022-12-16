#ifndef FAULT_FORMATION_TERRAIN_H
#define FAULT_FORMATION_TERRAIN_H

#include "terrain.h"

class FaultFormationTerrain : public BaseTerrain {

 public:
    FaultFormationTerrain() {}

    void CreateFaultFormation(int Size, int Iterations, float MinHeight, float MaxHeight, float Filter);

 private:
    void CreateFaultFormationInternal(int Iterations, float MinHeight, float MaxHeight, float Filter);
    void GenRandomTerrainPoints(TerrainPoint& p1, TerrainPoint& p2);
    void ApplyFIRFilter(float Filter);
    void ApplyFIRFilterBand(int Start, int Stride, int Count, float Filter);
};

#endif
