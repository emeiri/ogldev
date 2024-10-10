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


#include "ogldev_infinite_grid.h"


InfiniteGrid::InfiniteGrid()
{

}


void InfiniteGrid::Init()
{
    if (!m_infiniteGridTech.Init()) {
        printf("Error initializing the infinite grid technique\n");
        exit(1);
    }
}


void InfiniteGrid::Render(const InfiniteGridConfig& Config, const Matrix4f& VP, const Vector3f& CameraPos)
{
    GLint CurProg = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &CurProg);

    m_infiniteGridTech.Enable();

    m_infiniteGridTech.SetVP(VP);
    m_infiniteGridTech.SetCameraWorldPos(CameraPos);
    m_infiniteGridTech.SetCellSize(Config.CellSize);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 6, 1, 0);
    glDisable(GL_BLEND);

    glUseProgram(CurProg);
}