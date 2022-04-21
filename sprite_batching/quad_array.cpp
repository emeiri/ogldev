#include <assert.h>

#include "quad_array.h"

#define POSITION_LOCATION  0

QuadArray::QuadArray(uint NumQuads)
{
    assert(NumQuads > 0);

    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    glGenBuffers(ARRAY_SIZE_IN_ELEMENTS(m_Buffers), m_Buffers);

    CreateVertexBuffer();

    CreateIndexBuffer(NumQuads);

    // Make sure the VAO is not changed from the outside
    glBindVertexArray(0);
}


QuadArray::~QuadArray()
{

}


void QuadArray::CreateVertexBuffer()
{
    Vector3f vertices[] = { Vector3f(0.0f, 0.0f, 0.5f),   // bottom left
                            Vector3f(0.0f, 1.0f, 0.5f),   // top left
                            Vector3f(1.0f, 1.0f, 0.5f),   // top right
                            Vector3f(1.0f, 0.0f, 0.5f) }; // bottom right

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[POS_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(POSITION_LOCATION);
    glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

}


void QuadArray::CreateIndexBuffer(uint NumQuads)
{
    uint indices[] = { 0, 1, 2, 0, 2, 3};

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[INDEX_BUFFER]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices, GL_STATIC_DRAW);

}


void QuadArray::Render()
{
    glBindVertexArray(m_VAO);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}
