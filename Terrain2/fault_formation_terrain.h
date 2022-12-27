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
    float FIRFilterSinglePoint(int x, int z, float PrevFractalVal, float Filter);
};

#endif
