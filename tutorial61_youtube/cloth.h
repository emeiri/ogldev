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

    void Render(float dt, const Matrix4f& World, const Matrix4f& View, const Matrix4f& Projection);

private:
    
    void InitBuffers();
    void SetupVAO();
    void BindAndUploadBuffers(int TotalParticles,
                              std::vector<glm::vec4>& Positions, 
                              std::vector<glm::vec4>& Velocities, 
                              std::vector<GLuint>& Indices, 
                              std::vector<glm::vec2>& TexCoords);
    void GenBufferHandles();
    void InitIndices(std::vector<GLuint>& Indices);
    void InitVertices(std::vector<glm::vec4>& Positions, 
                      std::vector<glm::vec4>& Velocities, 
                      std::vector<glm::vec2>& TexCoords);
    void ExecuteClothSim(float dt);
    void RecalcNormals();
    void RenderCloth(const Matrix4f& World, const Matrix4f& View, const Matrix4f& Projection);

    Texture m_tex;

    ClothTechnique m_clothTech;
    RenderTechnique m_renderTech;    
    ClothNormalTechnique m_clothNormTech;

    GLuint m_numIndices = 0;
    glm::ivec2 m_numParticles = glm::ivec2(50, 50);
    glm::vec2 m_clothSize = glm::vec2(4.0f, 3.0f);
	
    GLuint m_vao = 0;
    GLuint m_curBuf = 0;
    GLuint m_posBufs[2], m_velBufs[2];
    GLuint m_normBuf, m_ib, m_tcBuf;
};


