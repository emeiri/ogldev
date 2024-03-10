#include <stdio.h>
#include <vector>

#include <GL/glew.h>
#include "ogldev_math_3d.h"
#include "triangle_list.h"
#include "terrain.h"

void TriangleList::Vertex::InitVertex(int Width, int Depth, int x, int z, float WorldScale)
{
    float y = 0.0f;
    Pos = Vector3f((float)x * WorldScale, y, (float)z * WorldScale);
    Tex = Vector2f((float)x / (float)Width, (float)z / (float)Depth);
}


TriangleList::TriangleList()
{
}


void TriangleList::CreateTriangleList(int Width, int Depth, float WorldScale)
{
    m_depth = Depth;
    m_width = Width;

    CreateGLState();

    PopulateBuffer(WorldScale);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    printf("Triangle strip %dx%d created\n", Width, Depth);
}


void TriangleList::CreateGLState()
{
    glGenVertexArrays(1, &m_vao);

    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vb);
    glBindBuffer(GL_ARRAY_BUFFER, m_vb);

    glGenBuffers(1, &m_ib);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ib);

    int pos_loc = 0;
    int tex_loc = 1;

    size_t Offset = 0;
    glEnableVertexAttribArray(pos_loc);
    glVertexAttribPointer(pos_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)Offset);
    Offset += 3;

    glEnableVertexAttribArray(tex_loc);
    glVertexAttribPointer(tex_loc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(Offset * sizeof(float)));
    Offset += 2;

    printf("GL state created\n");
}


void TriangleList::PopulateBuffer(float WorldScale)
{
    std::vector<Vertex> Vertices;
    Vertices.resize(m_width * m_depth);
    //printf("Preparing space for %lu vertices\n", Vertices.size());
    InitVertices(Vertices, WorldScale);

    std::vector<uint> Indices;
    int NumQuads = (m_width - 1) * (m_depth - 1);
    Indices.resize(NumQuads * 6);
    InitIndices(Indices);

    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices[0]) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices[0]) * Indices.size(), &Indices[0], GL_STATIC_DRAW);
}


void TriangleList::InitVertices(std::vector<Vertex>& Vertices, float WorldScale)
{
    int Index = 0;
    Vertex v;

    for (int z = 0 ; z < m_depth ; z++) {
        for (int x = 0 ; x < m_width ; x++) {
            //            printf("%d %d\n", x, z);
            v.InitVertex(m_width, m_depth, x, z, WorldScale);         // bottom left

            // Add "top left" triangle
            assert(Index < Vertices.size());
            Vertices[Index++] = v;
        }
    }

    assert(Index == Vertices.size());
}


void TriangleList::InitIndices(std::vector<uint>& Indices)
{
    int Index = 0;

    for (int z = 0 ; z < m_depth - 1 ; z++) {
        for (int x = 0 ; x < m_width - 1 ; x++) {
            //            printf("%d %d\n", x, z);
            uint IndexBottomLeft = z * m_width + x;
            uint IndexTopLeft = (z + 1) * m_width + x;
            uint IndexTopRight = (z + 1) * m_width + x + 1;
            uint IndexBottomRight = z * m_width + x + 1;

            // Add "top left" triangle
            assert(Index < Indices.size());
            Indices[Index++] = IndexBottomLeft;
            assert(Index < Indices.size());
            Indices[Index++] = IndexTopLeft;
            assert(Index < Indices.size());
            Indices[Index++] = IndexTopRight;

            // Add "bottom right" triangle
            assert(Index < Indices.size());
            Indices[Index++] = IndexBottomLeft;
            assert(Index < Indices.size());
            Indices[Index++] = IndexTopRight;
            assert(Index < Indices.size());
            Indices[Index++] = IndexBottomRight;
        }
    }

    assert(Index == Indices.size());

}


void TriangleList::Render()
{
    glBindVertexArray(m_vao);

    glDrawElements(GL_TRIANGLES, (m_depth - 1) * (m_width - 1) * 6, GL_UNSIGNED_INT, NULL);

    glBindVertexArray(0);
}
