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


#ifndef TRIANGLE_LIST_H
#define TRIANGLE_LIST_H

#include <GL/glew.h>
#include <vector>

#include "ogldev_math_3d.h"

// this header is included by terrain.h so we have a forward 
// declaration for BaseTerrain.
class BaseTerrain;

class TriangleList {
 public:
    TriangleList();

    ~TriangleList();

    void CreateTriangleList(int Width, int Depth, const BaseTerrain* pTerrain);

    void Destroy();

    void Render();

 private:

    struct Vertex {
        Vector3f Pos;        
        Vector2f Tex;
        float LightFactor;

        void InitVertex(const BaseTerrain* pTerrain, int x, int z);
    };

    void CreateGLState();

	void PopulateBuffers(const BaseTerrain* pTerrain);
    void InitVertices(const BaseTerrain* pTerrain, std::vector<Vertex>& Vertices);
    void InitIndices(std::vector<uint>& Indices);

    int m_width = 0;
    int m_depth = 0;
    GLuint m_vao = 0;
    GLuint m_vb = 0;
    GLuint m_ib = 0;
};

#endif
