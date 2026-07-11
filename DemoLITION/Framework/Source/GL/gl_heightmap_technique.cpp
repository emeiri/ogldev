/*

        Copyright 2026 Etay Meiri

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


#include "GL/gl_heightmap_technique.h"


bool HeightmapTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "Framework/Shaders/GL/heightmap.vs")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "Framework/Shaders/GL/heightmap.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    return InitCommon();
}


bool HeightmapTechnique::InitCommon()
{
    GET_UNIFORM(gWVP);

    return true;
}


void HeightmapTechnique::SetWVP(const Matrix4f& WVP)
{
    glUniformMatrix4fv(m_gWVPLoc, 1, GL_TRUE, (const GLfloat*)WVP.m);
}


