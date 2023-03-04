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
