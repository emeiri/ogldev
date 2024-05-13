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

#include <GL/glew.h>

#include "ogldev_math_3d.h"
#include "ogldev_screen_quad.h"

struct Vertex {
    Vertex() {}

    Vertex(float x, float y, float u, float v)
    {
        Pos.x = x;
        Pos.y = y;
        Pos.z = 0.5f;

        Tex.u = u;
        Tex.v = v;
    }

    Vector3f Pos;
    Vector2f Tex;
};


ScreenQuad::~ScreenQuad()
{

}


void ScreenQuad::Init(int x, int y, int Width, int Height)
{
    CreateGLState();

    PopulateVertexBuffer(x, y, Width, Height);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void ScreenQuad::CreateGLState()
{
    glGenVertexArrays(1, &m_vao);

    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vb);
    glBindBuffer(GL_ARRAY_BUFFER, m_vb);

    int pos_loc = 0;
    int tex_loc = 1;

    size_t Offset = 0;
    glEnableVertexAttribArray(pos_loc);
    glVertexAttribPointer(pos_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)Offset);
    Offset += 3;

    glEnableVertexAttribArray(tex_loc);
    glVertexAttribPointer(tex_loc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(Offset * sizeof(float)));
    Offset += 2;
}


void ScreenQuad::PopulateVertexBuffer(int x, int y, int Width, int Height)
{
    GLint Viewport[4];
    glGetIntegerv(GL_VIEWPORT, Viewport);

    if ((Viewport[0] != 0) || (Viewport[1] != 0)) {
        printf("%s:%d - assuming viewport stars at 0,0\n", __FILE__, __LINE__);
        exit(0);
    }

    float Left = 2.0f * (float)x / (float)Viewport[2] - 1.0f;
    float Right = 2.0f * (float)(x + Width) / (float)Viewport[2] - 1.0f;
    float Bottom = 2.0f * (float)y / (float)Viewport[3] - 1.0f;
    float Top = 2.0f * (float)(y + Height) / (float)Viewport[3] - 1.0f;

    std::vector<Vertex> Vertices;
    Vertices.resize(6);

    Vertices[0] = Vertex(Left, Bottom, 0.0f, 0.0f);    
    Vertices[1] = Vertex(Left, Top, 0.0f, 1.0f);    // top left
    Vertices[2] = Vertex(Right, Top, 1.0f, 1.0f);   // top right
    Vertices[3] = Vertex(Left, Bottom, 0.0f, 0.0f);    
    Vertices[4] = Vertex(Right, Top, 1.0f, 1.0f);   // top right
    Vertices[5] = Vertex(Right, Bottom, 1.0f, 0.0f);  

    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices[0]) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);
}


void ScreenQuad::Render()
{
    glBindVertexArray(m_vao);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
}
