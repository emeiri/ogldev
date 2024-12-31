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
#include "particles.h"

#include <vector>

#include <glm/gtc/matrix_transform.hpp>

Particles::Particles()
{
    m_numParticlesX = 100;
    m_numParticlesY = 100;
    m_numParticlesZ = 100;

    m_speed = 35.0f;
    m_angle = 0.0f;

    m_bh1 = glm::vec4(5, 0, 0, 1);
    m_bh2 = glm::vec4(-5, 0, 0, 1);

    m_totalParticles = m_numParticlesX * m_numParticlesY * m_numParticlesZ;
}

void Particles::Init()
{
    m_colorTech.Init();    
    m_particlesTech.Init();

    InitBuffers();
}

void Particles::InitBuffers()
{
    vector<Vector4f> Positions(m_totalParticles);
    CalcPositions(Positions);

    glCreateBuffers(1, &m_posBuf);
    glCreateBuffers(1, &m_velBuf);

    GLuint BufSize = (int)Positions.size() * sizeof(Positions[0]);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_posBuf);
    glBufferData(GL_SHADER_STORAGE_BUFFER, BufSize, Positions.data(), GL_DYNAMIC_DRAW);

    vector<Vector4f> Velocities(Positions.size(), Vector4f(0.0f));
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_velBuf);
    glBufferData(GL_SHADER_STORAGE_BUFFER, BufSize, Velocities.data(), GL_DYNAMIC_COPY);

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_posBuf);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    glCreateBuffers(1, &m_bhBuf);
    glBindBuffer(GL_ARRAY_BUFFER, m_bhBuf);
    GLfloat data[] = { m_bh1.x, m_bh1.y, m_bh1.z, m_bh1.w, m_bh2.x, m_bh2.y, m_bh2.z, m_bh2.w };
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), data, GL_DYNAMIC_DRAW);

    glGenVertexArrays(1, &m_bhVao);
    glBindVertexArray(m_bhVao);

    glBindBuffer(GL_ARRAY_BUFFER, m_bhBuf);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}


void Particles::CalcPositions(vector<Vector4f>& Positions)
{
    Vector4f p(0.0f, 0.0f, 0.0f, 1.0f);

    float dx = 2.0f / m_numParticlesX;
    float dy = 2.0f / m_numParticlesY;
    float dz = 2.0f / m_numParticlesZ;

    // We want to center the particles at (0,0,0)
    Matrix4f Translation;
    Translation.InitTranslationTransform(Vector3f(-1.0f));

    int ParticleIndex = 0;
    for (int x = 0; x < m_numParticlesX; x++) {
        for (int y = 0; y < m_numParticlesY; y++) {
            for (int z = 0; z < m_numParticlesZ; z++) {
                p.x = dx * x;
                p.y = dy * y;
                p.z = dz * z;
                p.w = 1.0f;
                p = Translation * p;
                Positions[ParticleIndex] = p;
                ParticleIndex++;
            }
        }
    }
}



void Particles::Update(float dt)
{ 
    m_angle += m_speed * dt;
   // printf("%f\n", m_angle);
    if (m_angle > 360.0f) {
        m_angle -= 360.0f;
    }
}


void Particles::Render(const Matrix4f& VP)
{
    // Rotate the attractors ("black holes")
    glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(m_angle), glm::vec3(0, 0, 1));
    Vector3f BlackHolePos1(glm::vec3(rot * m_bh1));
    Vector3f BlackHolePos2(glm::vec3(rot * m_bh2));

    ExecuteComputeShader(BlackHolePos1, BlackHolePos2);

    RenderParticles(BlackHolePos1, BlackHolePos2, VP);
}

void Particles::ExecuteComputeShader(const Vector3f& BlackHolePos1, const Vector3f& BlackHolePos2)
{
    m_particlesTech.Enable();
    m_particlesTech.SetBlackHoles(BlackHolePos1, BlackHolePos2);

    glDispatchCompute(m_totalParticles, 1, 1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}


void Particles::RenderParticles(const Vector3f& BlackHolePos1, const Vector3f& BlackHolePos2, const Matrix4f& VP)
{
    // Draw the scene
    m_colorTech.Enable();
    m_colorTech.SetWVP(VP);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw the particles
    glPointSize(2.0f);
    m_colorTech.SetColor(Vector4f(0.0f, 0.0f, 0.0f, 0.2f));
    glBindVertexArray(m_vao);
    glDrawArrays(GL_POINTS,0, m_totalParticles);
    glBindVertexArray(0);

    // Draw the black holes
    glPointSize(15.0f);
    GLfloat data[] = { BlackHolePos1.x, BlackHolePos1.y, BlackHolePos1.z, 1.0f, BlackHolePos2.x, BlackHolePos2.y, BlackHolePos2.z, 1.0f };
    glBindBuffer(GL_ARRAY_BUFFER, m_bhBuf);
    glBufferSubData( GL_ARRAY_BUFFER, 0, 8 * sizeof(GLfloat), data );
    m_colorTech.SetColor(Vector4f(1,0,0,1.0f));
    glBindVertexArray(m_bhVao);
    glDrawArrays(GL_POINTS, 0, 2);
    glBindVertexArray(0);
}


