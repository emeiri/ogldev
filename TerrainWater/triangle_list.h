#ifndef TRIANGLE_LIST_H
#define TRIANGLE_LIST_H

#include <GL/glew.h>
#include <vector>

#include "ogldev_math_3d.h"

class TriangleList {
 public:
    TriangleList();

    void CreateTriangleList(int Width, int Depth, float WorldScale);

    void Render();

 private:

    struct Vertex {
        Vector3f Pos;
        Vector2f Tex;

        void InitVertex(int Width, int Depth, int x, int z, float WorldScale);
    };

    void CreateGLState();

    void PopulateBuffer(float WorldScale);

    void InitVertices(std::vector<Vertex>& Vertices, float WorldScale);
    void InitIndices(std::vector<uint>& Indices);

    int m_width = 0;
    int m_depth = 0;
    GLuint m_vao;
    GLuint m_vb;
    GLuint m_ib;
};

#endif
