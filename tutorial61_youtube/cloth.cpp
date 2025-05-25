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

#define PRIM_RESTART 0xffffff

Cloth::Cloth() : m_tex(GL_TEXTURE_2D)
{
}

void Cloth::Init()
{
    m_renderTech.Init(); 

    m_clothNormTech.Init();

    m_clothTech.Init();
    m_clothTech.Enable();
    float dx = clothSize.x / (m_numParticles.x - 1);
    float dy = clothSize.y / (m_numParticles.y - 1);
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
    std::vector<float> TexCoords;//(TotalParticles);

    InitVertices(Positions, Velocities, TexCoords);

    // Every row is one triangle strip
    vector<GLuint> el;
    for (int row = 0; row < m_numParticles.y - 1; row++) {
        for (int col = 0; col < m_numParticles.x; col++) {
            el.push_back((row + 1) * m_numParticles.x + (col));
            el.push_back((row)*m_numParticles.x + (col));
        }
        el.push_back(PRIM_RESTART);
    }

    // We need buffers for position (2), element index,
    // velocity (2), normal, and texture coordinates.
    GLuint bufs[7];
    glGenBuffers(7, bufs);
    posBufs[0] = bufs[0];
    posBufs[1] = bufs[1];
    velBufs[0] = bufs[2];
    velBufs[1] = bufs[3];
    normBuf = bufs[4];
    elBuf = bufs[5];
    tcBuf = bufs[6];

    GLuint parts = m_numParticles.x * m_numParticles.y;

    // The buffers for positions
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, posBufs[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, parts * 4 * sizeof(GLfloat), &Positions[0], GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, posBufs[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, parts * 4 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);

    // Velocities
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, velBufs[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, parts * 4 * sizeof(GLfloat), &Velocities[0], GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, velBufs[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, parts * 4 * sizeof(GLfloat), NULL, GL_DYNAMIC_COPY);

    // Normal buffer
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, normBuf);
    glBufferData(GL_SHADER_STORAGE_BUFFER, parts * 4 * sizeof(GLfloat), NULL, GL_DYNAMIC_COPY);

    // Element indicies
    glBindBuffer(GL_ARRAY_BUFFER, elBuf);
    glBufferData(GL_ARRAY_BUFFER, el.size() * sizeof(GLuint), &el[0], GL_DYNAMIC_COPY);

    // Texture coordinates
    glBindBuffer(GL_ARRAY_BUFFER, tcBuf);
    glBufferData(GL_ARRAY_BUFFER, TexCoords.size() * sizeof(GLfloat), &TexCoords[0], GL_STATIC_DRAW);

    numElements = GLuint(el.size());

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, posBufs[0]);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, normBuf);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, tcBuf);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elBuf);
    glBindVertexArray(0);
}


void Cloth::InitVertices(std::vector<glm::vec4>& Positions, 
                         std::vector<glm::vec4>& Velocities, 
                         std::vector<float>& TexCoords)
{
    glm::mat4 transf = glm::translate(glm::mat4(1.0), glm::vec3(0, clothSize.y, 0));
    transf = glm::rotate(transf, glm::radians(-80.0f), glm::vec3(1, 0, 0));
    transf = glm::translate(transf, glm::vec3(0, -clothSize.y, 0));

    float dx = clothSize.x / (m_numParticles.x - 1);
    float dy = clothSize.y / (m_numParticles.y - 1);
    float ds = 1.0f / (m_numParticles.x - 1);
    float dt = 1.0f / (m_numParticles.y - 1);
    glm::vec4 p(0.0f, 0.0f, 0.0f, 1.0f);
    int Index = 0;
    for (int i = 0; i < m_numParticles.y; i++) {
        for (int j = 0; j < m_numParticles.x; j++) {
            p.x = dx * j;
            p.y = dy * i;
            p.z = 0.0f;
            p = transf * p;
            
            Positions[Index] = p;

            TexCoords.push_back(ds * j);
            TexCoords.push_back(dt * i);

            Index++;
        }
    }
}


void Cloth::Update(float dt)
{ 
  if( time == 0.0f ) {
    deltaT = 0.0f;
  } else {
    deltaT = dt - time;
  }
  time = dt;

}

void Cloth::Render(const Matrix4f& WV, const Matrix4f& WVP)
{
    ExecuteClothSim();

    RecalcNormals();

    RenderCloth(WVP, WVP);
}

void Cloth::ExecuteClothSim()
{
    m_clothTech.Enable();

    for (int i = 0; i < 1000; i++) {
        glDispatchCompute(m_numParticles.x / 10, m_numParticles.y / 10, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        // Swap buffers
        readBuf = 1 - readBuf;  // 0 --> 1 --> 0 --> ...
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, posBufs[readBuf]);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, posBufs[1 - readBuf]);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, velBufs[readBuf]);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, velBufs[1 - readBuf]);
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
    glDrawElements(GL_TRIANGLE_STRIP, numElements, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}


