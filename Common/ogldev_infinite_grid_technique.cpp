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


#include "ogldev_infinite_grid_technique.h"


bool InfiniteGridTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "../Common/Shaders/infinite_grid.vs")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "../Common/Shaders/infinite_grid.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    return InitCommon();
}


bool InfiniteGridTechnique::InitCommon()
{
    GET_UNIFORM_AND_CHECK(VPLoc, "gVP");
    GET_UNIFORM_AND_CHECK(CameraWorldPosLoc, "gCameraWorldPos");
    GET_UNIFORM_AND_CHECK(GridCellSizeLoc, "gGridCellSize");

    return true;
}


void InfiniteGridTechnique::SetVP(const Matrix4f& VP)
{
    glUniformMatrix4fv(VPLoc, 1, GL_TRUE, (const GLfloat*)VP.m);
}


void InfiniteGridTechnique::SetCameraWorldPos(const Vector3f& CameraWorldPos)
{
    glUniform3f(CameraWorldPosLoc, CameraWorldPos.x, CameraWorldPos.y, CameraWorldPos.z);
}


void InfiniteGridTechnique::SetCellSize(float CellSize)
{
    glUniform1f(GridCellSizeLoc, CellSize);
}


