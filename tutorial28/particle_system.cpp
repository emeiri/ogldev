/*

	Copyright 2011 Etay Meiri

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

#include "ogldev_engine_common.h"
#include "ogldev_util.h"
#include "particle_system.h"
#include "ogldev_math_3d.h"

#define MAX_PARTICLES 1000
#define PARTICLE_LIFETIME 10.0f

#define PARTICLE_TYPE_LAUNCHER 0.0f
#define PARTICLE_TYPE_SHELL 1.0f
#define PARTICLE_TYPE_SECONDARY_SHELL 2.0f

struct Particle
{
    float Type;    
    Vector3f Pos;
    Vector3f Vel;    
    float LifetimeMillis;    
};


ParticleSystem::ParticleSystem()
{
    m_currVB = 0;
    m_currTFB = 1;
    m_isFirst = true;
    m_time = 0;
    m_pTexture = NULL;            
    
    ZERO_MEM(m_transformFeedback);
    ZERO_MEM(m_particleBuffer);
}


ParticleSystem::~ParticleSystem()
{
    SAFE_DELETE(m_pTexture);
    
    if (m_transformFeedback[0] != 0) {
        glDeleteTransformFeedbacks(2, m_transformFeedback);
    }
    
    if (m_particleBuffer[0] != 0) {
        glDeleteBuffers(2, m_particleBuffer);
    }
}


bool ParticleSystem::InitParticleSystem(const Vector3f& Pos)
{   
    Particle Particles[MAX_PARTICLES];
    ZERO_MEM(Particles);

    Particles[0].Type = PARTICLE_TYPE_LAUNCHER;
    Particles[0].Pos = Pos;
    Particles[0].Vel = Vector3f(0.0f, 0.0001f, 0.0f);
    Particles[0].LifetimeMillis = 0.0f;

    glGenTransformFeedbacks(2, m_transformFeedback);    
    glGenBuffers(2, m_particleBuffer);
    
    for (unsigned int i = 0; i < 2 ; i++) {
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_transformFeedback[i]);
        glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Particles), Particles, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_particleBuffer[i]);        
    }
                      
    if (!m_updateTechnique.Init()) {
        return false;
    }
    
    m_updateTechnique.Enable();
    
    m_updateTechnique.SetRandomTextureUnit(RANDOM_TEXTURE_UNIT_INDEX);
    m_updateTechnique.SetLauncherLifetime(100.0f);
    m_updateTechnique.SetShellLifetime(10000.0f);
    m_updateTechnique.SetSecondaryShellLifetime(2500.0f);
    
    if (!m_randomTexture.InitRandomTexture(1000)) {
        return false;
    }
    
    m_randomTexture.Bind(RANDOM_TEXTURE_UNIT);

    if (!m_billboardTechnique.Init()) {
        return false;
    }
    
    m_billboardTechnique.Enable();

    m_billboardTechnique.SetColorTextureUnit(COLOR_TEXTURE_UNIT_INDEX);

    m_billboardTechnique.SetBillboardSize(0.01f);
    
    m_pTexture = new Texture(GL_TEXTURE_2D, "../Content/fireworks_red.jpg");
    
    if (!m_pTexture->Load()) {
        return false;
    }        
    
    return GLCheckError();
}


void ParticleSystem::Render(int DeltaTimeMillis, const Matrix4f& VP, const Vector3f& CameraPos)
{
    m_time += DeltaTimeMillis;
    
    UpdateParticles(DeltaTimeMillis);

    RenderParticles(VP, CameraPos);

    m_currVB = m_currTFB;
    m_currTFB = (m_currTFB + 1) & 0x1;
}


void ParticleSystem::UpdateParticles(int DeltaTimeMillis)
{
    m_updateTechnique.Enable();
    m_updateTechnique.SetTime(m_time);
    m_updateTechnique.SetDeltaTimeMillis(DeltaTimeMillis);
   
    m_randomTexture.Bind(RANDOM_TEXTURE_UNIT);
    
    glEnable(GL_RASTERIZER_DISCARD);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[m_currVB]);    
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_transformFeedback[m_currTFB]);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), 0);                          // type
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)4);         // position
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)16);        // velocity
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)28);          // lifetime
    
    glBeginTransformFeedback(GL_POINTS);

    if (m_isFirst) {
        glDrawArrays(GL_POINTS, 0, 1);

        m_isFirst = false;
    }
    else {
        glDrawTransformFeedback(GL_POINTS, m_transformFeedback[m_currVB]);
    }            
    
    glEndTransformFeedback();

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
}
    

void ParticleSystem::RenderParticles(const Matrix4f& VP, const Vector3f& CameraPos)
{
    m_billboardTechnique.Enable();
    m_billboardTechnique.SetCameraPosition(CameraPos);
    m_billboardTechnique.SetVP(VP);
    m_pTexture->Bind(COLOR_TEXTURE_UNIT);
    
    glDisable(GL_RASTERIZER_DISCARD);

    glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[m_currTFB]);    

    glEnableVertexAttribArray(0);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)4);  // position

    glDrawTransformFeedback(GL_POINTS, m_transformFeedback[m_currTFB]);

    glDisableVertexAttribArray(0);
}
