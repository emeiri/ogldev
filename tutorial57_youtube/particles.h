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
	
    GLuint m_vao = 0;
    GLuint m_posBuf = 0;
	GLuint m_velBuf = 0;
    
	GLuint m_bhVao = 0;
	GLuint m_bhBuf = 0;

    glm::vec4 m_bh1, m_bh2;    
};


