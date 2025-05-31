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

#include "render_technique.h"


RenderTechnique::RenderTechnique()
{
}

bool RenderTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "render.vs")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "render.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    GET_UNIFORM_AND_CHECK(WorldMatrixLoc, "WorldMatrix");
    GET_UNIFORM_AND_CHECK(NormalMatrixLoc, "NormalMatrix");
    GET_UNIFORM_AND_CHECK(ViewMatrixLoc, "ViewMatrix");    
    GET_UNIFORM_AND_CHECK(ProjectionMatrixLoc, "ProjectionMatrix");

    return true;
}

void RenderTechnique::SetWorldMatrix(const Matrix4f& m)
{
    glUniformMatrix4fv(WorldMatrixLoc, 1, GL_TRUE, (const GLfloat*)m.m);
}

void RenderTechnique::SetNormalMatrix(const Matrix4f& m)
{
    glUniformMatrix4fv(NormalMatrixLoc, 1, GL_TRUE, (const GLfloat*)m.m);
}

void RenderTechnique::SetViewMatrix(const Matrix4f& m)
{
    glUniformMatrix4fv(ViewMatrixLoc, 1, GL_TRUE, (const GLfloat*)m.m);
}

void RenderTechnique::SetProjectionMatrix(const Matrix4f& m)
{
    glUniformMatrix4fv(ProjectionMatrixLoc, 1, GL_TRUE, (const GLfloat*)m.m);
}

