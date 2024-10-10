/*

        Copyright 2024 Etay Meiri

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


#pragma once


#include "ogldev_infinite_grid_technique.h"

struct InfiniteGridConfig {
    float Size = 100.0f;
    float CellSize = 0.025f;
    Vector4f ColorThin = Vector4f(0.5f, 0.5f, 0.5f, 1.0f);
    Vector4f ColorThick = Vector4f(0.0f, 0.0f, 0.0f, 1.0f);
    float MinPixelsBetweenCells = 2.0f;
};


class InfiniteGrid
{
public:
    InfiniteGrid();

    void Init();

    void Render(const InfiniteGridConfig& Config, const Matrix4f& VP, const Vector3f& CameraPos);

private:

    InfiniteGridTechnique m_infiniteGridTech;
};