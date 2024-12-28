#pragma once

#include <glm/glm.hpp>

#include "ogldev_color_technique.h"
#include "particles_technique.h"

class Particles
{
public:
    Particles();

    void initScene();
    void update(float t);
    void Render(const Matrix4f& VP);
    void resize(int, int);

private:
    ColorTechnique m_colorTech;
    ParticlesTechnique m_particlesTech;

    glm::ivec3 nParticles;
    GLuint totalParticles;

    float time, deltaT, speed, angle;
    GLuint particlesVao;
    GLuint bhVao, bhBuf;  // black hole VAO and buffer
    glm::vec4 bh1, bh2;

    void initBuffers();
};


