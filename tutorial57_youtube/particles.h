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
    
    void InitBuffers();
    void CalcPositions(vector<Vector4f>& Positions);
    void ExecuteComputeShader(const Vector3f& BlackHolePos1, const Vector3f& BlackHolePos2);
    void RenderParticles(const Vector3f& BlackHolePos1, const Vector3f& BlackHolePos2, const Matrix4f& VP);

    ColorTechnique m_colorTech;
    ParticlesTechnique m_particlesTech;

    int m_numParticlesX = 0;
    int m_numParticlesY = 0;
    int m_numParticlesZ = 0;

    int m_totalParticles = 0;

    float m_speed, m_angle;
    GLuint m_vao;
    GLuint bhVao, bhBuf;  // black hole VAO and buffer

    glm::vec4 bh1, bh2;    
};


