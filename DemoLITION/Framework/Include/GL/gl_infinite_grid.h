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

#include "demolition_scene.h"
#include "gl_infinite_grid_technique.h"

class InfiniteGrid
{
public:
    InfiniteGrid();

    void Init(int ShadowMapTextureUnit);

    void Render(const InfiniteGridConfig& Config,
                const Matrix4f& VP,
                const Vector3f& CameraPos,
                const Matrix4f& LightVP,
                const Vector3f& LightDir);

private:

    GLuint m_dummyVAO = -1;
    InfiniteGridTechnique m_infiniteGridTech;
};