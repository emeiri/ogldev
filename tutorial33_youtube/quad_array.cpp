#include <assert.h>

#include "quad_array.h"

#define POSITION_LOCATION 0
#define PRIM_ID_LOCATION  1

#define NUM_VERTICES 6

QuadArray::QuadArray(uint MaxNumQuads)
{
    m_maxNumQuads = MaxNumQuads;

    glGenVertexArrays(1, &m_VAO);

    glBindVertexArray(m_VAO);

    glGenBuffers(ARRAY_SIZE_IN_ELEMENTS(m_Buffers), m_Buffers);

    CreateVertexBuffer();

    CreateQuadIDBuffer();

    // Make sure the VAO is not changed from the outside
    glBindVertexArray(0);
}


QuadArray::~QuadArray()
{

}


void QuadArray::CreateVertexBuffer()
{
    Vector2f vertices[] = { Vector2f(0.0f, 0.0f),   // bottom left
                            Vector2f(0.0f, 1.0f),   // top left
                            Vector2f(1.0f, 1.0f),   // top right
                            Vector2f(0.0f, 0.0f),   // bottom left
                            Vector2f(1.0f, 1.0f),   // top right
                            Vector2f(1.0f, 0.0f) }; // bottom right

    vector<Vector2f> vertices_vec;
    vertices_vec.resize(m_maxNumQuads * NUM_VERTICES);

    for (uint i = 0 ; i < m_maxNumQuads ; i++) {
        for (int j = 0; j < NUM_VERTICES ; j++) {
            vertices_vec[i * NUM_VERTICES + j] = vertices[j];
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[POS_VB]);
    glBufferData(GL_ARRAY_BUFFER, vertices_vec.size() * sizeof(Vector2f), &vertices_vec[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(POSITION_LOCATION);
    glVertexAttribPointer(POSITION_LOCATION, 2, GL_FLOAT, GL_FALSE, 0, 0);
}


void QuadArray::CreateQuadIDBuffer()
{
    vector<GLuint> quad_id_vec;
    quad_id_vec.resize(m_maxNumQuads * NUM_VERTICES);

    for (uint i = 0 ; i < m_maxNumQuads ; i++) {
        for (int j = 0; j < NUM_VERTICES ; j++) {
            quad_id_vec[i * NUM_VERTICES + j] = i;
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[QUAD_ID_VB]);
    glBufferData(GL_ARRAY_BUFFER, quad_id_vec.size() * sizeof(GLuint), &quad_id_vec[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(PRIM_ID_LOCATION);
    glVertexAttribIPointer(PRIM_ID_LOCATION, 1, GL_UNSIGNED_INT, 0, 0);
}


void QuadArray::Render(uint NumQuads)
{
    assert(NumQuads <= m_maxNumQuads);

    if (NumQuads == 0) {
        NumQuads = m_maxNumQuads;
    }

    glBindVertexArray(m_VAO);

    glDrawArrays(GL_TRIANGLES, 0, NumQuads * NUM_VERTICES);

    glBindVertexArray(0);
}
