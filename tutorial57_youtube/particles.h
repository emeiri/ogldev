#pragma once

#include "ogldev_color_technique.h"
#include "particles_technique.h"

class Particles
{
public:
    Particles();

    void Init();
    void Update(float t);
    void Render(const Matrix4f& VP);

private:
    ColorTechnique m_colorTech;
    ParticlesTechnique m_particlesTech;

    int m_numParticlesX;
    int m_numParticlesY;
    int m_numParticlesZ;

    GLuint totalParticles;

    float time, deltaT, speed, angle;
    GLuint particlesVao;
    GLuint bhVao, bhBuf;  // black hole VAO and buffer
    glm::vec4 bh1, bh2;

    void initBuffers();
};


