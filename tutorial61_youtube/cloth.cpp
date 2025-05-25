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
    float dx = clothSize.x / (nParticles.x - 1);
    float dy = clothSize.y / (nParticles.y - 1);
    m_clothTech.SetRestLengthHoriz(dx);
    m_clothTech.SetRestLengthVert(dy);
    m_clothTech.SetRestLengthDiag(sqrtf(dx * dx + dy * dy));

    InitBuffers();

    m_tex.Load("../Content/textures/me_textile.png");
}

void Cloth::InitBuffers()
{
    glm::mat4 transf = glm::translate(glm::mat4(1.0), glm::vec3(0, clothSize.y, 0));
    transf = glm::rotate(transf, glm::radians(-80.0f), glm::vec3(1, 0, 0));
    transf = glm::translate(transf, glm::vec3(0, -clothSize.y, 0));

    // Initial positions of the particles
    vector<GLfloat> initPos;
    vector<GLfloat> initVel(nParticles.x * nParticles.y * 4, 0.0f);
    vector<float> initTc;
    float dx = clothSize.x / (nParticles.x - 1);
    float dy = clothSize.y / (nParticles.y - 1);
    float ds = 1.0f / (nParticles.x - 1);
    float dt = 1.0f / (nParticles.y - 1);
    glm::vec4 p(0.0f, 0.0f, 0.0f, 1.0f);
    for (int i = 0; i < nParticles.y; i++) {
        for (int j = 0; j < nParticles.x; j++) {
            p.x = dx * j;
            p.y = dy * i;
            p.z = 0.0f;
            p = transf * p;
            initPos.push_back(p.x);
            initPos.push_back(p.y);
            initPos.push_back(p.z);
            initPos.push_back(1.0f);

            initTc.push_back(ds * j);
            initTc.push_back(dt * i);
        }
    }

    // Every row is one triangle strip
    vector<GLuint> el;
    for (int row = 0; row < nParticles.y - 1; row++) {
        for (int col = 0; col < nParticles.x; col++) {
            el.push_back((row + 1) * nParticles.x + (col));
            el.push_back((row)*nParticles.x + (col));
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

    GLuint parts = nParticles.x * nParticles.y;

    // The buffers for positions
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, posBufs[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, parts * 4 * sizeof(GLfloat), &initPos[0], GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, posBufs[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, parts * 4 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);

    // Velocities
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, velBufs[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, parts * 4 * sizeof(GLfloat), &initVel[0], GL_DYNAMIC_COPY);
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
    glBufferData(GL_ARRAY_BUFFER, initTc.size() * sizeof(GLfloat), &initTc[0], GL_STATIC_DRAW);

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


void Cloth::CalcPositions(vector<Vector4f>& Positions)
{
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
        glDispatchCompute(nParticles.x / 10, nParticles.y / 10, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        // Swap buffers
        readBuf = 1 - readBuf;
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, posBufs[readBuf]);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, posBufs[1 - readBuf]);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, velBufs[readBuf]);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, velBufs[1 - readBuf]);
    }

    m_clothNormTech.Enable();
    glDispatchCompute(nParticles.x / 10, nParticles.y / 10, 1);
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


