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

    m_tex.Load("../Content/textures/me_textile.png");
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

    // We need buffers for position (2), element index,
    // velocity (2), normal, and texture coordinates.
    GLuint bufs[7];
    glGenBuffers(7, bufs);
    m_posBufs[0] = bufs[0];
    m_posBufs[1] = bufs[1];
    m_velBufs[0] = bufs[2];
    m_velBufs[1] = bufs[3];
    m_normBuf = bufs[4];
    m_ib = bufs[5];
    m_tcBuf = bufs[6];

    // The buffers for positions
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_posBufs[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, TotalParticles * 4 * sizeof(GLfloat), &Positions[0], GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_posBufs[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, TotalParticles * 4 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);

    // Velocities
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_velBufs[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, TotalParticles * 4 * sizeof(GLfloat), &Velocities[0], GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_velBufs[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, TotalParticles * 4 * sizeof(GLfloat), NULL, GL_DYNAMIC_COPY);

    // Normal buffer
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_normBuf);
    glBufferData(GL_SHADER_STORAGE_BUFFER, TotalParticles * 4 * sizeof(GLfloat), NULL, GL_DYNAMIC_COPY);

    // Element indicies
    glBindBuffer(GL_ARRAY_BUFFER, m_ib);
    glBufferData(GL_ARRAY_BUFFER, ARRAY_SIZE_IN_BYTES(Indices), Indices.data(), GL_DYNAMIC_COPY);

    // Texture coordinates
    glBindBuffer(GL_ARRAY_BUFFER, m_tcBuf);
    glBufferData(GL_ARRAY_BUFFER, ARRAY_SIZE_IN_BYTES(TexCoords), &TexCoords[0], GL_STATIC_DRAW);

    m_numIndices = GLuint(Indices.size());

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_posBufs[0]);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, m_normBuf);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, m_tcBuf);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ib);
    glBindVertexArray(0);
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
        assert(Index < Indices.size());
        Indices[Index] = PRIM_RESTART;
        Index++;        
    }
}


void Cloth::Render(float dt, const Matrix4f& WV, const Matrix4f& WVP)
{
    ExecuteClothSim(dt);

    RecalcNormals();

    RenderCloth(WVP, WVP);
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

void Cloth::RenderCloth(const Matrix4f& WV, const Matrix4f& WVP)
{
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(PRIM_RESTART);

    m_renderTech.Enable();

    m_renderTech.SetWVMatrix(WV);
    m_renderTech.SetNormalMatrix(WV);
    m_renderTech.SetWVPMatrix(WVP);

    m_tex.Bind(GL_TEXTURE0);

    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLE_STRIP, m_numIndices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}


