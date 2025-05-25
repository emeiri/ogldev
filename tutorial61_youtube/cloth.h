#pragma once

#include "render_technique.h"
#include "cloth_technique.h"
#include "cloth_normal_technique.h"
#include "ogldev_texture.h"

class Cloth
{
public:
    Cloth();

    void Init();

    void Render(float dt, const Matrix4f& WV, const Matrix4f& WVP);

private:
    
    void InitBuffers();
    void InitVertices(std::vector<glm::vec4>& Positions, 
                      std::vector<glm::vec4>& Velocities, 
                      std::vector<glm::vec2>& TexCoords);
    void ExecuteClothSim(float dt);
    void RecalcNormals();
    void RenderCloth(const Matrix4f& WV, const Matrix4f& WVP);

    Texture m_tex;

    ClothTechnique m_clothTech;
    RenderTechnique m_renderTech;    
    ClothNormalTechnique m_clothNormTech;

    GLuint numElements = 0;
    glm::ivec2 m_numParticles = glm::ivec2(100, 100);
    glm::vec2 m_clothSize = glm::vec2(4.0f, 3.0f);
	
    GLuint m_vao = 0;
    GLuint m_curBuf = 0;
    GLuint posBufs[2], velBufs[2];
    GLuint normBuf, elBuf, tcBuf;
};


