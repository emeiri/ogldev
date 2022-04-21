#include <assert.h>

#include "quad_array.h"

#define POSITION_LOCATION 0
#define PRIM_ID_LOCATION  1

#define NUM_VERICES 4
#define NUM_INDICES 6

QuadArray::QuadArray(uint NumQuads)
{
    m_numQuads = NumQuads;

    assert(NumQuads > 0);

    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    glGenBuffers(ARRAY_SIZE_IN_ELEMENTS(m_Buffers), m_Buffers);

    CreateVertexBuffer();

    CreatePrimIdBuffer();

    CreateIndexBuffer();

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
                            Vector2f(1.0f, 0.0f) }; // bottom right

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[POS_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(POSITION_LOCATION);
    glVertexAttribPointer(POSITION_LOCATION, 2, GL_FLOAT, GL_FALSE, 0, 0);
}


void QuadArray::CreatePrimIdBuffer()
{
    vector<uint> prim_id_vec;
    prim_id_vec.resize(m_numQuads * NUM_VERICES);

    for (uint i = 0 ; i < m_numQuads ; i++) {
        for (int j = 0; j < NUM_VERICES ; j++) {
            prim_id_vec[i * m_numQuads + j] = i;
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[PRIM_ID_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(prim_id_vec), &prim_id_vec[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(PRIM_ID_LOCATION);
    glVertexAttribIPointer(PRIM_ID_LOCATION, 1, GL_UNSIGNED_INT, 0, 0);
}


void QuadArray::CreateIndexBuffer()
{
    uint indices[] = { 0, 1, 2, 0, 2, 3};

    vector<uint> indices_vec;
    indices_vec.resize(m_numQuads * NUM_INDICES);

    for (uint i = 0 ; i < m_numQuads ; i++) {
        for (uint j = 0 ; j < ARRAY_SIZE_IN_ELEMENTS(indices) ; j++) {
            indices_vec[i * NUM_INDICES + j] = indices[j];
        }
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[INDEX_BUFFER]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_vec), &indices_vec[0], GL_STATIC_DRAW);

}


void QuadArray::Render()
{
    glBindVertexArray(m_VAO);

    glDrawElements(GL_TRIANGLES, m_numQuads * NUM_INDICES, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}
