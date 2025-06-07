/*

        Copyright 2025 Etay Meiri

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

#include "Int/core_rendering_system.h"
#include "GL/gl_model.h"
#include "GL/gl_engine_common.h"
#include "GL/gl_ssbo_db.h"


bool UsePVP = true;     // Programmable Vertex Pulling
bool UseIndirectRender = true;

#define POSITION_LOCATION    0
#define TEX_COORD0_LOCATION  1
#define TEX_COORD1_LOCATION  2
#define NORMAL_LOCATION      3
#define TANGENT_LOCATION     4
#define BITANGENT_LOCATION   5
#define COLOR_LOCATION       6
#define BONE_ID_LOCATION     7
#define BONE_WEIGHT_LOCATION 8


GLModel::GLModel()
{

}


GLModel::~GLModel()
{
    if (m_Buffers[0] != 0) {
        glDeleteBuffers(ARRAY_SIZE_IN_ELEMENTS(m_Buffers), m_Buffers);
    }

    if (m_VAO != 0) {
        glDeleteVertexArrays(1, &m_VAO);
        m_VAO = 0;
    }
}

void GLModel::AllocBuffers()
{
    if (IsGLVersionHigher(4, 5)) {
        glCreateVertexArrays(1, &m_VAO);
        glCreateBuffers(ARRAY_SIZE_IN_ELEMENTS(m_Buffers), m_Buffers);
    }
    else {
        glGenVertexArrays(1, &m_VAO);
        glBindVertexArray(m_VAO);
        glGenBuffers(ARRAY_SIZE_IN_ELEMENTS(m_Buffers), m_Buffers);
    }
}


void GLModel::PopulateBuffersSkinned(std::vector<SkinnedVertex>& Vertices)
{
    PopulateBuffersInternal<SkinnedVertex>(Vertices);
}


void GLModel::PopulateBuffers(std::vector<Vertex>& Vertices)
{
    PopulateBuffersInternal<Vertex>(Vertices);
}


void GLModel::InitGeometryPost()
{
    if (UseIndirectRender) {
        m_indirectRender.Init(m_Meshes, m_Materials);
    }
}



template<typename VertexType>
void GLModel::PopulateBuffersInternal(std::vector<VertexType>& Vertices)
{
    if (UsePVP) {
        if (IsGLVersionHigher(4, 5)) {
            PopulateBuffersPVP(Vertices);
        }
        else {
            printf("Programmable vertex pulling but OpenGL version is less than 4.5\n");
            exit(1);
        }
    } else {
        if (IsGLVersionHigher(4, 5)) {
            PopulateBuffersDSA(Vertices);
        }
        else {
            PopulateBuffersNonDSA(Vertices);
        }
    }
}


template<typename VertexType>
void GLModel::PopulateBuffersPVP(std::vector<VertexType>& Vertices)
{
    glNamedBufferStorage(m_Buffers[VERTEX_BUFFER], sizeof(VertexType) * Vertices.size(), Vertices.data(), 0);
    glNamedBufferStorage(m_Buffers[INDEX_BUFFER], sizeof(m_Indices[0]) * m_Indices.size(), m_Indices.data(), 0);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_INDEX_VERTICES, m_Buffers[VERTEX_BUFFER]);

    glVertexArrayElementBuffer(m_VAO, m_Buffers[INDEX_BUFFER]);
}


template<typename VertexType>
void GLModel::PopulateBuffersNonDSA(std::vector<VertexType>& Vertices)
{
    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[VERTEX_BUFFER]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[INDEX_BUFFER]);

    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexType) * Vertices.size(), Vertices.data(), GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_Indices[0]) * m_Indices.size(), &m_Indices[0], GL_STATIC_DRAW);

    size_t NumFloats = 0;

    int NumElements = 3;
    glEnableVertexAttribArray(POSITION_LOCATION);
    glVertexAttribPointer(POSITION_LOCATION, NumElements, GL_FLOAT, GL_FALSE, sizeof(VertexType), (const void*)(NumFloats * sizeof(float)));
    NumFloats += NumElements;

    NumElements = 2;
    glEnableVertexAttribArray(TEX_COORD0_LOCATION);
    glVertexAttribPointer(TEX_COORD0_LOCATION, NumElements, GL_FLOAT, GL_FALSE, sizeof(VertexType), (const void*)(NumFloats * sizeof(float)));
    NumFloats += NumElements;

    NumElements = 2;
    glEnableVertexAttribArray(TEX_COORD1_LOCATION);
    glVertexAttribPointer(TEX_COORD1_LOCATION, NumElements, GL_FLOAT, GL_FALSE, sizeof(VertexType), (const void*)(NumFloats * sizeof(float)));
    NumFloats += NumElements;

    NumElements = 3;
    glEnableVertexAttribArray(NORMAL_LOCATION);
    glVertexAttribPointer(NORMAL_LOCATION, NumElements, GL_FLOAT, GL_FALSE, sizeof(VertexType), (const void*)(NumFloats * sizeof(float)));
    NumFloats += NumElements;

    NumElements = 3;
    glEnableVertexAttribArray(TANGENT_LOCATION);
    glVertexAttribPointer(TANGENT_LOCATION, NumElements, GL_FLOAT, GL_FALSE, sizeof(VertexType), (const void*)(NumFloats * sizeof(float)));
    NumFloats += NumElements;

    NumElements = 3;
    glEnableVertexAttribArray(BITANGENT_LOCATION);
    glVertexAttribPointer(BITANGENT_LOCATION, NumElements, GL_FLOAT, GL_FALSE, sizeof(VertexType), (const void*)(NumFloats * sizeof(float)));
    NumFloats += NumElements;

    NumElements = 4;
    glEnableVertexAttribArray(COLOR_LOCATION);
    glVertexAttribPointer(COLOR_LOCATION, NumElements, GL_FLOAT, GL_FALSE, sizeof(VertexType), (const void*)(NumFloats * sizeof(float)));
    NumFloats += NumElements;

    if constexpr (std::is_same_v<VertexType, SkinnedVertex>) {
        NumElements = MAX_NUM_BONES_PER_VERTEX;
        glEnableVertexAttribArray(BONE_ID_LOCATION);
        glVertexAttribIPointer(BONE_ID_LOCATION, NumElements, GL_INT, sizeof(VertexType), (const void*)(NumFloats * sizeof(float)));
        NumFloats += NumElements;

        NumElements = MAX_NUM_BONES_PER_VERTEX;
        glEnableVertexAttribArray(BONE_WEIGHT_LOCATION);
        glVertexAttribPointer(BONE_WEIGHT_LOCATION, NumElements, GL_FLOAT, GL_FALSE, sizeof(VertexType), (const void*)(NumFloats * sizeof(float)));
        NumFloats += NumElements;
    }
}


template<typename VertexType>
void GLModel::PopulateBuffersDSA(std::vector<VertexType>& Vertices)
{
    // for (int i = 0; i < Vertices.size(); i++) {
    //     Vertices[i].Print();
   //  }
    glNamedBufferStorage(m_Buffers[VERTEX_BUFFER], sizeof(VertexType) * Vertices.size(), Vertices.data(), 0);
    glNamedBufferStorage(m_Buffers[INDEX_BUFFER], sizeof(m_Indices[0]) * m_Indices.size(), m_Indices.data(), 0);

    glVertexArrayVertexBuffer(m_VAO, 0, m_Buffers[VERTEX_BUFFER], 0, sizeof(VertexType));
    glVertexArrayElementBuffer(m_VAO, m_Buffers[INDEX_BUFFER]);

    size_t NumFloats = 0;

    int NumElements = 3;
    glEnableVertexArrayAttrib(m_VAO, POSITION_LOCATION);
    glVertexArrayAttribFormat(m_VAO, POSITION_LOCATION, NumElements, GL_FLOAT, GL_FALSE, (GLuint)(NumFloats * sizeof(float)));
    glVertexArrayAttribBinding(m_VAO, POSITION_LOCATION, 0);
    NumFloats += NumElements;

    NumElements = 2;
    glEnableVertexArrayAttrib(m_VAO, TEX_COORD0_LOCATION);
    glVertexArrayAttribFormat(m_VAO, TEX_COORD0_LOCATION, NumElements, GL_FLOAT, GL_FALSE, (GLuint)(NumFloats * sizeof(float)));
    glVertexArrayAttribBinding(m_VAO, TEX_COORD0_LOCATION, 0);
    NumFloats += NumElements;

    NumElements = 2;
    glEnableVertexArrayAttrib(m_VAO, TEX_COORD1_LOCATION);
    glVertexArrayAttribFormat(m_VAO, TEX_COORD1_LOCATION, NumElements, GL_FLOAT, GL_FALSE, (GLuint)(NumFloats * sizeof(float)));
    glVertexArrayAttribBinding(m_VAO, TEX_COORD1_LOCATION, 0);
    NumFloats += NumElements;

    NumElements = 3;
    glEnableVertexArrayAttrib(m_VAO, NORMAL_LOCATION);
    glVertexArrayAttribFormat(m_VAO, NORMAL_LOCATION, NumElements, GL_FLOAT, GL_FALSE, (GLuint)(NumFloats * sizeof(float)));
    glVertexArrayAttribBinding(m_VAO, NORMAL_LOCATION, 0);
    NumFloats += NumElements;

    NumElements = 3;
    glEnableVertexArrayAttrib(m_VAO, TANGENT_LOCATION);
    glVertexArrayAttribFormat(m_VAO, TANGENT_LOCATION, NumElements, GL_FLOAT, GL_FALSE, (GLuint)(NumFloats * sizeof(float)));
    glVertexArrayAttribBinding(m_VAO, TANGENT_LOCATION, 0);
    NumFloats += NumElements;

    NumElements = 3;
    glEnableVertexArrayAttrib(m_VAO, BITANGENT_LOCATION);
    glVertexArrayAttribFormat(m_VAO, BITANGENT_LOCATION, NumElements, GL_FLOAT, GL_FALSE, (GLuint)(NumFloats * sizeof(float)));
    glVertexArrayAttribBinding(m_VAO, BITANGENT_LOCATION, 0);
    NumFloats += NumElements;

    NumElements = 4;
    glEnableVertexArrayAttrib(m_VAO, COLOR_LOCATION);
    glVertexArrayAttribFormat(m_VAO, COLOR_LOCATION, NumElements, GL_FLOAT, GL_FALSE, (GLuint)(NumFloats * sizeof(float)));
    glVertexArrayAttribBinding(m_VAO, COLOR_LOCATION, 0);
    NumFloats += NumElements;

    if constexpr (std::is_same_v<VertexType, SkinnedVertex>) {
        NumElements = MAX_NUM_BONES_PER_VERTEX;
        glEnableVertexArrayAttrib(m_VAO, BONE_ID_LOCATION);
        glVertexArrayAttribIFormat(m_VAO, BONE_ID_LOCATION, NumElements, GL_INT, (GLuint)(NumFloats * sizeof(float)));
        glVertexArrayAttribBinding(m_VAO, BONE_ID_LOCATION, 0);
        NumFloats += NumElements;

        NumElements = MAX_NUM_BONES_PER_VERTEX;
        glEnableVertexArrayAttrib(m_VAO, BONE_WEIGHT_LOCATION);
        glVertexArrayAttribFormat(m_VAO, BONE_WEIGHT_LOCATION, NumElements, GL_FLOAT, GL_FALSE, (GLuint)(NumFloats * sizeof(float)));
        glVertexArrayAttribBinding(m_VAO, BONE_WEIGHT_LOCATION, 0);
        NumFloats += NumElements;
    }
}


void GLModel::Render(DemolitionRenderCallbacks* pRenderCallbacks)
{
    assert(!UseIndirectRender);

    if (m_isPBR) {
        SetupRenderMaterialsPBR();
    }

    glBindVertexArray(m_VAO);

    if (UsePVP) {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_INDEX_VERTICES, m_Buffers[VERTEX_BUFFER]);
    }

    for (unsigned int i = 0; i < m_Meshes.size(); i++) {
        RenderMesh(i, pRenderCallbacks);
    }

    // Make sure the VAO is not changed from the outside
    glBindVertexArray(0);
}


void GLModel::RenderMesh(int MeshIndex, DemolitionRenderCallbacks* pRenderCallbacks)
{
    int MaterialIndex = m_Meshes[MeshIndex].MaterialIndex;
    assert((MaterialIndex >= 0) && (MaterialIndex < m_Materials.size()));

    if (m_Materials[MaterialIndex].pTextures[TEX_TYPE_BASE]) {
        m_Materials[MaterialIndex].pTextures[TEX_TYPE_BASE]->Bind(COLOR_TEXTURE_UNIT);
    }

    if (m_Materials[MaterialIndex].pTextures[TEX_TYPE_SPECULAR]) {
        m_Materials[MaterialIndex].pTextures[TEX_TYPE_SPECULAR]->Bind(SPECULAR_EXPONENT_UNIT);

        if (pRenderCallbacks) {
            pRenderCallbacks->ControlSpecularExponent_CB(true);
        }
    } else {
        if (pRenderCallbacks) {
            pRenderCallbacks->ControlSpecularExponent_CB(false);
        }
    }

    if (m_Materials[MaterialIndex].pTextures[TEX_TYPE_EMISSIVE]) {
        m_Materials[MaterialIndex].pTextures[TEX_TYPE_EMISSIVE]->Bind(EMISSIVE_TEXTURE_UNIT);
    }

    if (m_Materials[MaterialIndex].pTextures[TEX_TYPE_ROUGHNESS]) {
        m_Materials[MaterialIndex].pTextures[TEX_TYPE_ROUGHNESS]->Bind(ROUGHNESS_TEXTURE_UNIT);
    }

    if (m_Materials[MaterialIndex].pTextures[TEX_TYPE_NORMAL]) {
        m_Materials[MaterialIndex].pTextures[TEX_TYPE_NORMAL]->Bind(NORMAL_TEXTURE_UNIT);
    } else if (m_pNormalMap) {
        m_pNormalMap->Bind(NORMAL_TEXTURE_UNIT);
    }

    if (m_pHeightMap) {
        m_pHeightMap->Bind(HEIGHT_TEXTURE_UNIT);
    }

    if (pRenderCallbacks) {
        pRenderCallbacks->DrawStart_CB(MeshIndex);
        pRenderCallbacks->SetMaterial_CB(m_Materials[MaterialIndex]);
        pRenderCallbacks->SetWorldMatrix_CB(m_Meshes[MeshIndex].Transformation);
    }

    glDrawElementsBaseVertex(GL_TRIANGLES,
                        m_Meshes[MeshIndex].NumIndices,
                        GL_UNSIGNED_INT,
                        (void*)(sizeof(unsigned int) * m_Meshes[MeshIndex].BaseIndex),
                        m_Meshes[MeshIndex].BaseVertex);
}


void GLModel::Render(unsigned int DrawIndex, unsigned int PrimID)
{
    glBindVertexArray(m_VAO);

    if (UsePVP) {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_INDEX_VERTICES, m_Buffers[VERTEX_BUFFER]);
    }

    int MaterialIndex = m_Meshes[DrawIndex].MaterialIndex;
    assert((MaterialIndex >= 0) && (MaterialIndex < m_Materials.size()));

    if (m_Materials[MaterialIndex].pTextures[TEX_TYPE_BASE]) {
        m_Materials[MaterialIndex].pTextures[TEX_TYPE_BASE]->Bind(COLOR_TEXTURE_UNIT);
    }

    if (m_Materials[MaterialIndex].pTextures[TEX_TYPE_SPECULAR]) {
        m_Materials[MaterialIndex].pTextures[TEX_TYPE_SPECULAR]->Bind(SPECULAR_EXPONENT_UNIT);
    }

    glDrawElementsBaseVertex(GL_TRIANGLES,
        3,
        GL_UNSIGNED_INT,
        (void*)(sizeof(unsigned int) * (m_Meshes[DrawIndex].BaseIndex + PrimID * 3)),
        m_Meshes[DrawIndex].BaseVertex);

    // Make sure the VAO is not changed from the outside
    glBindVertexArray(0);
}



// Used only by instancing
void GLModel::Render(unsigned int NumInstances, const Matrix4f* WVPMats, const Matrix4f* WorldMats)
{
    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[WVP_MAT_BUFFER]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Matrix4f) * NumInstances, WVPMats, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[WORLD_MAT_BUFFER]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Matrix4f) * NumInstances, WorldMats, GL_DYNAMIC_DRAW);

    glBindVertexArray(m_VAO);

    if (UsePVP) {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_INDEX_VERTICES, m_Buffers[VERTEX_BUFFER]);
    }

    for (unsigned int i = 0; i < m_Meshes.size(); i++) {
        int MaterialIndex = m_Meshes[i].MaterialIndex;

        assert((MaterialIndex >= 0) && (MaterialIndex < m_Materials.size()));

        if (m_Materials[MaterialIndex].pTextures[TEX_TYPE_BASE]) {
            m_Materials[MaterialIndex].pTextures[TEX_TYPE_BASE]->Bind(COLOR_TEXTURE_UNIT);
        }

        if (m_Materials[MaterialIndex].pTextures[TEX_TYPE_SPECULAR]) {
            m_Materials[MaterialIndex].pTextures[TEX_TYPE_SPECULAR]->Bind(SPECULAR_EXPONENT_UNIT);
        }

        glDrawElementsInstancedBaseVertex(GL_TRIANGLES,
            m_Meshes[i].NumIndices,
            GL_UNSIGNED_INT,
            (void*)(sizeof(unsigned int) * m_Meshes[i].BaseIndex),
            NumInstances,
            m_Meshes[i].BaseVertex);
    }

    // Make sure the VAO is not changed from the outside
    glBindVertexArray(0);
}


void GLModel::RenderIndirect(const Matrix4f& ObjectMatrix)
{
    assert(UseIndirectRender);

    glBindVertexArray(m_VAO);

    if (UsePVP) {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_INDEX_VERTICES, m_Buffers[VERTEX_BUFFER]);
    }

    m_indirectRender.Render(ObjectMatrix);

    // Make sure the VAO is not changed from the outside
    glBindVertexArray(0);
}


void GLModel::SetupRenderMaterialsPBR()
{
    int PBRMaterialIndex = 0;

    // Hack...
    m_Materials[PBRMaterialIndex].PBRmaterial.pAlbedo = m_PBRmaterial.pAlbedo;
    m_Materials[PBRMaterialIndex].PBRmaterial.pRoughness = m_PBRmaterial.pRoughness;
    m_Materials[PBRMaterialIndex].PBRmaterial.pMetallic = m_PBRmaterial.pMetallic;
    m_Materials[PBRMaterialIndex].PBRmaterial.pNormalMap = m_PBRmaterial.pNormalMap;
    m_Materials[PBRMaterialIndex].PBRmaterial.pAO = m_PBRmaterial.pAO;
    m_Materials[PBRMaterialIndex].PBRmaterial.pEmissive = m_PBRmaterial.pEmissive;

    if (m_Materials[PBRMaterialIndex].PBRmaterial.pAlbedo) {
        m_Materials[PBRMaterialIndex].PBRmaterial.pAlbedo->Bind(ALBEDO_TEXTURE_UNIT);
    }

    if (m_Materials[PBRMaterialIndex].PBRmaterial.pRoughness) {
        m_Materials[PBRMaterialIndex].PBRmaterial.pRoughness->Bind(ROUGHNESS_TEXTURE_UNIT);
    }

    if (m_Materials[PBRMaterialIndex].PBRmaterial.pMetallic) {
        m_Materials[PBRMaterialIndex].PBRmaterial.pMetallic->Bind(METALLIC_TEXTURE_UNIT);
    }

    if (m_Materials[PBRMaterialIndex].PBRmaterial.pNormalMap) {
        m_Materials[PBRMaterialIndex].PBRmaterial.pNormalMap->Bind(NORMAL_TEXTURE_UNIT);
    }

    if (m_Materials[PBRMaterialIndex].PBRmaterial.pAO) {
        m_Materials[PBRMaterialIndex].PBRmaterial.pAO->Bind(AO_TEXTURE_UNIT);
    }

    if (m_Materials[PBRMaterialIndex].PBRmaterial.pEmissive) {
        m_Materials[PBRMaterialIndex].PBRmaterial.pEmissive->Bind(EMISSIVE_TEXTURE_UNIT);
    }
}


void GLModel::SetColorTexture(int TextureHandle)
{
    BaseTexture* pTexture = NULL;

    if (TextureHandle >= 0) {
        pTexture = m_pCoreRenderingSystem->GetTexture(TextureHandle);
    }

    if (m_Materials.size() == 0) {
        printf("SetColorTexture: no materials\n");
        assert(0);
    }

    if (m_Materials[0].pTextures[TEX_TYPE_BASE]) {
        delete m_Materials[0].pTextures[TEX_TYPE_BASE];
    }

    m_Materials[0].pTextures[TEX_TYPE_BASE] = (Texture*)pTexture;
}



void GLModel::SetNormalMap(int TextureHandle)
{
    if (TextureHandle < 0) {
        m_pNormalMap = NULL;
    }
    else {
        BaseTexture* pTexture = m_pCoreRenderingSystem->GetTexture(TextureHandle);
        m_pNormalMap = (Texture*)pTexture;

        // Hack 
        if (m_Materials.size() > 0) {
            if (m_Materials[0].pTextures[TEX_TYPE_NORMAL]) {
                delete m_Materials[0].pTextures[TEX_TYPE_NORMAL];
            }

            m_Materials[0].pTextures[TEX_TYPE_NORMAL] = m_pNormalMap;

            if (UseIndirectRender) {
                m_indirectRender.RefreshMaterials(m_Materials);
            }
        }
    }
}


void GLModel::SetHeightMap(int TextureHandle)
{
    if (TextureHandle < 0) {
        m_pHeightMap = NULL;
    }
    else {
        BaseTexture* pTexture = m_pCoreRenderingSystem->GetTexture(TextureHandle);
        m_pHeightMap = (Texture*)pTexture;
    }
}


Texture* GLModel::AllocTexture2D()
{
    return new Texture(GL_TEXTURE_2D);
}

