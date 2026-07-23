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


#include "ogldev_texture.h"
#include "cloth.h"

#include <vector>

#include <glm/gtc/matrix_transform.hpp>

#define PRIM_RESTART 0xffffffff

Cloth::Cloth() : m_tex(GL_TEXTURE_2D)
{
}

void Cloth::Init()
{
    m_renderTech.Init(); 

    m_clothNormTech.Init();

    m_clothTech.Init();
    m_clothTech.Enable();
    float dx = m_clothSize.x / (m_numParticles.x - 1);
    float dy = m_clothSize.y / (m_numParticles.y - 1);
    m_clothTech.SetRestLengthHoriz(dx);
    m_clothTech.SetRestLengthVert(dy);
    m_clothTech.SetRestLengthDiag(sqrtf(dx * dx + dy * dy));

    InitBuffers();

    m_tex.Load("../Content/textures/me_textile.png", false);
}

void Cloth::InitBuffers()
{
    // Initial positions of the particles
    int TotalParticles = m_numParticles.x * m_numParticles.y;
    std::vector<glm::vec4> Positions(TotalParticles);
    std::vector<glm::vec4> Velocities(TotalParticles, glm::vec4(0.0f));
    std::vector<glm::vec2> TexCoords(TotalParticles);

    InitVertices(Positions, Velocities, TexCoords);

    std::vector<GLuint> Indices;
    InitIndices(Indices);

    GenBufferHandles();

    BindAndUploadBuffers(TotalParticles, Positions, Velocities, Indices, TexCoords);

    SetupVAO();
}


void Cloth::SetupVAO()
{
    glCreateVertexArrays(1, &m_vao);

    // Position
    glVertexArrayVertexBuffer(
        m_vao,
        0,
        m_posBufs[0],
        0,
        sizeof(glm::vec4)
    );

    glEnableVertexArrayAttrib(m_vao, 0);
    glVertexArrayAttribFormat(
        m_vao,
        0,
        4,
        GL_FLOAT,
        GL_FALSE,
        0
    );
    glVertexArrayAttribBinding(m_vao, 0, 0);

    // Normal
    glVertexArrayVertexBuffer(
        m_vao,
        1,
        m_normBuf,
        0,
        sizeof(glm::vec4)
    );

    glEnableVertexArrayAttrib(m_vao, 1);
    glVertexArrayAttribFormat(
        m_vao,
        1,
        4,
        GL_FLOAT,
        GL_FALSE,
        0
    );
    glVertexArrayAttribBinding(m_vao, 1, 1);

    // Texture coordinates
    glVertexArrayVertexBuffer(
        m_vao,
        2,
        m_tcBuf,
        0,
        sizeof(glm::vec2)
    );

    glEnableVertexArrayAttrib(m_vao, 2);
    glVertexArrayAttribFormat(
        m_vao,
        2,
        2,
        GL_FLOAT,
        GL_FALSE,
        0
    );
    glVertexArrayAttribBinding(m_vao, 2, 2);

    glVertexArrayElementBuffer(m_vao, m_ib);
}

void Cloth::BindAndUploadBuffers(
    int totalParticles,
    std::vector<glm::vec4>& positions,
    std::vector<glm::vec4>& velocities,
    std::vector<GLuint>& indices,
    std::vector<glm::vec2>& texCoords)
{
    glNamedBufferData(
        m_posBufs[0],
        ARRAY_SIZE_IN_BYTES(positions),
        positions.data(),
        GL_DYNAMIC_DRAW
    );

    glNamedBufferData(
        m_posBufs[1],
        ARRAY_SIZE_IN_BYTES(positions),
        nullptr,
        GL_DYNAMIC_DRAW
    );

    glNamedBufferData(
        m_velBufs[0],
        ARRAY_SIZE_IN_BYTES(velocities),
        velocities.data(),
        GL_DYNAMIC_COPY
    );

    glNamedBufferData(
        m_velBufs[1],
        ARRAY_SIZE_IN_BYTES(velocities),
        nullptr,
        GL_DYNAMIC_COPY
    );

    glNamedBufferData(
        m_normBuf,
        static_cast<GLsizeiptr>(
            totalParticles * sizeof(glm::vec4)
            ),
        nullptr,
        GL_DYNAMIC_COPY
    );

    glNamedBufferData(
        m_ib,
        ARRAY_SIZE_IN_BYTES(indices),
        indices.data(),
        GL_STATIC_DRAW
    );

    glNamedBufferData(
        m_tcBuf,
        ARRAY_SIZE_IN_BYTES(texCoords),
        texCoords.data(),
        GL_STATIC_DRAW
    );

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_posBufs[0]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_posBufs[1]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_velBufs[0]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_velBufs[1]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_normBuf);
}


void Cloth::GenBufferHandles()
{
    GLuint buffers[7]{};
    glCreateBuffers(7, buffers);

    m_posBufs[0] = buffers[0];
    m_posBufs[1] = buffers[1];
    m_velBufs[0] = buffers[2];
    m_velBufs[1] = buffers[3];
    m_ib = buffers[4];
    m_tcBuf = buffers[5];
    m_normBuf = buffers[6];
}


void Cloth::InitVertices(std::vector<glm::vec4>& Positions, 
                         std::vector<glm::vec4>& Velocities, 
                         std::vector<glm::vec2>& TexCoords)
{
    float dx = m_clothSize.x / (m_numParticles.x - 1);
    float dy = m_clothSize.y / (m_numParticles.y - 1);
    float ds = 1.0f / (m_numParticles.x - 1);
    float dt = 1.0f / (m_numParticles.y - 1);

    int Index = 0;

    for (int y = 0; y < m_numParticles.y; y++) {
        for (int x = 0; x < m_numParticles.x; x++) {
            glm::vec4 p(dx * x, 0.0f, dy * y, 1.0f);
            
            Positions[Index] = p;

            glm::vec2 tc(ds * x, dt * y);

            TexCoords[Index] = tc;

            Index++;
        }
    }
}


void Cloth::InitIndices(std::vector<GLuint>& Indices)
{
    int NumResetValues = m_numParticles.y - 1;
    m_numIndices = (m_numParticles.y - 1) * m_numParticles.x * 2 + NumResetValues;
    Indices.resize(m_numIndices);

    int Index = 0;

    for (int row = 0; row < m_numParticles.y - 1; row++) {
        for (int col = 0; col < m_numParticles.x; col++) {
            assert(Index < Indices.size());
            Indices[Index] = row * m_numParticles.x + col;
            Index++; 
         
            assert(Index < Indices.size());
            Indices[Index] = (row + 1) * m_numParticles.x + col;
            Index++;
        }

        // Add a reset at the end of each line
        assert(Index < Indices.size());
        Indices[Index] = PRIM_RESTART;
        Index++;        
    }
}


void Cloth::Render(float dt, const Matrix4f& World, const Matrix4f& View, const Matrix4f& Projection)
{
    ExecuteClothSim(dt);

    RecalcNormals();

    RenderCloth(World, View, Projection);
}

void Cloth::ExecuteClothSim(float dt)
{
    m_clothTech.Enable();

    int Iterations = std::min((int)(30.0f / dt), 3000);

    for (int i = 0; i < Iterations; i++) {
        glDispatchCompute(m_numParticles.x / 10, m_numParticles.y / 10, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        // Swap buffers
        m_curBuf = 1 - m_curBuf;  // 0 --> 1 --> 0 --> ...
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_posBufs[m_curBuf]);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_posBufs[1 - m_curBuf]);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_velBufs[m_curBuf]);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_velBufs[1 - m_curBuf]);
    }
}


void Cloth::RecalcNormals()
{
    m_clothNormTech.Enable();
    glDispatchCompute(m_numParticles.x / 10, m_numParticles.y / 10, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}


void Cloth::RenderCloth(
    const Matrix4f& world,
    const Matrix4f& view,
    const Matrix4f& projection)
{
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(PRIM_RESTART);

    m_renderTech.Enable();
    m_renderTech.SetWorldMatrix(world);
    m_renderTech.SetNormalMatrix(world);
    m_renderTech.SetViewMatrix(view);
    m_renderTech.SetProjectionMatrix(projection);

    m_tex.Bind(GL_TEXTURE0);

    // The simulation ping-pongs between two position buffers.
    glVertexArrayVertexBuffer(
        m_vao,
        0,
        m_posBufs[m_curBuf],
        0,
        sizeof(glm::vec4)
    );

    glBindVertexArray(m_vao);

    glDrawElements(
        GL_TRIANGLE_STRIP,
        static_cast<GLsizei>(m_numIndices),
        GL_UNSIGNED_INT,
        nullptr
    );

    glBindVertexArray(0);
}

