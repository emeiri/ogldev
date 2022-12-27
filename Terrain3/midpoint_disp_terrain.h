#ifndef MIDPOINT_DISP_TERRAIN_H
#define MIDPOINT_DISP_TERRAIN_H

#include "terrain.h"

class MidpointDispTerrain : public BaseTerrain {

 public:
    MidpointDispTerrain() {}

    void CreateMidpointDisplacement(int Size, float Roughness, float MinHeight, float MaxHeight);

 private:
    void CreateMidpointDisplacementF32(float Roughness);
    void DiamondStep(int RectSize, float CurHeight);
    void SquareStep(int RectSize, float CurHeight);
};

#endif
