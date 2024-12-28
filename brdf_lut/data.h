#pragma once

#include "brdf_lut_technique.h"

class Particles
{
public:
    Particles();

    void Init();
    void Render();
    const std::vector<char>& GetOutput();

private:
    
    void InitBuffers();
  //  void ExecuteComputeShader(const Vector3f& BlackHolePos1, const Vector3f& BlackHolePos2);
   // void RenderParticles(const Vector3f& BlackHolePos1, const Vector3f& BlackHolePos2, const Matrix4f& VP);

    BRDF_LUT_Technique m_brdfTech;
    std::vector<char> m_outputData;
    GLuint m_vao;
    GLuint m_outputBuf = 0;
};


