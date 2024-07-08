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


#pragma once


#include <GL/glew.h>
#include <vector>

#include "ogldev_math_3d.h"

class Grid {
 public:
    Grid();

    ~Grid();

    void CreateTriangleList(int Width, int Depth);

    void Destroy();

    void Render();

 private:

    struct Vertex {
        Vector3f Pos;        
        Vector2f Tex;

        void InitVertex(int x, int z);
    };

    void CreateGLState();

	void PopulateBuffers();
    void InitVertices(std::vector<Vertex>& Vertices);
    void InitIndices(std::vector<uint>& Indices);

    int m_width = 0;
    int m_depth = 0;
    GLuint m_vao = 0;
    GLuint m_vb = 0;
    GLuint m_ib = 0;
};