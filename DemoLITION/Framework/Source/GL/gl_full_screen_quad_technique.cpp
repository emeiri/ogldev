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

#include "GL/gl_full_screen_quad_technique.h"


FullScreenQuadTechnique::FullScreenQuadTechnique()
{
}

bool FullScreenQuadTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "Framework/Shaders/GL/full_screen_quad.vs")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "Framework/Shaders/GL/tex.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    glGenVertexArrays(1, &m_dummyVAO);

    return true;
}


void FullScreenQuadTechnique::Render()
{    
   // glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
   // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(m_dummyVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

