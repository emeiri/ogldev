/*
    Copyright 2023 Etay Meiri

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

#include "ogldev_array_2d.h"
#include "ogldev_math_3d.h"

#pragma once

class SlopeLighter
{
public:
    SlopeLighter(const Array2D<float>* pHeightmap) : m_pHeightmap(pHeightmap) {}

    void InitLighter(const Vector3f& LightDir, int TerrainSize, float Softness);

    float GetLighting(int x, int z) const;

private:
    const Array2D<float>* m_pHeightmap = NULL;

    int m_terrainSize = 0;
    float m_softness = 0.0f;

    // how much to move to the first vertex towards the light source
    int m_dz0 = 0;
    int m_dx0 = 0;
    // how much to move to the second vertex towards the light source
    int m_dz1 = 0;
    int m_dx1 = 0;
    float m_factor = 0.0f; // the interpolation factor between the two vertices
};