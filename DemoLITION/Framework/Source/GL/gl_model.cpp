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
bool UseIndirectRender = false;

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


void GLModel::ConvertToMesh(const char* pFilename)
{
    assert(m_vertexBufferSizeBytes > 0);
    assert(m_indexBufferSizeBytes > 0);

    MeshData mesh;

    mesh.m_totalVertices = (int)(m_vertexBufferSizeBytes / sizeof(Vertex));
    mesh.m_totalIndices = (int)m_Indices.size();
    mesh.m_vertexSize = (int)sizeof(Vertex);

    size_t NumMeshes = m_Meshes.size();
    mesh.m_meshes.resize(NumMeshes);
    mesh.m_boxes.resize(NumMeshes);

    mesh.m_indexData.Size = m_indexBufferSizeBytes;
    mesh.m_indexData.pMem = (char*)glMapNamedBufferRange(m_Buffers[INDEX_BUFFER], 0, mesh.m_indexData.Size, GL_MAP_READ_BIT);

    mesh.m_vertexData.Size = m_vertexBufferSizeBytes;
    mesh.m_vertexData.pMem = (char*)glMapNamedBufferRange(m_Buffers[VERTEX_BUFFER], 0, mesh.m_vertexData.Size, GL_MAP_READ_BIT);

    for (size_t i = 0; i != NumMeshes; i++) {
        printf("\nConverting meshes %d/%d\n", (int)i + 1, (int)NumMeshes);
        Mesh m;

        // TODO: lod
        m.m_baseIndex = m_Meshes[i].BaseIndex;
        m.m_baseVertex = m_Meshes[i].BaseVertex;
        m.m_numIndices = m_Meshes[i].NumIndices;
        m.m_numVertices = m_Meshes[i].NumVertices;
        m.m_lodOffsets[0] = 0;
        m.m_lodOffsets[1] = m_Meshes[i].NumIndices;
        
        mesh.m_meshes[i] = m;
    }

    SaveMeshData(pFilename, mesh);

    // recalculateBoundingBoxes(meshData);

    glUnmapNamedBuffer(m_Buffers[INDEX_BUFFER]);
    glUnmapNamedBuffer(m_Buffers[VERTEX_BUFFER]);
}


bool GLModel::LoadMesh(const std::string& Filename)
{
    MeshData mesh;

    LoadMeshData(Filename.c_str(), mesh);

    AllocBuffers();

    m_Meshes.resize(mesh.m_meshes.size());

    for (int i = 0 ; i < m_Meshes.size() ; i++) {
        m_Meshes[i].NumIndices = mesh.m_meshes[i].m_numIndices;
        m_Meshes[i].NumVertices = mesh.m_meshes[i].m_numVertices;
        m_Meshes[i].BaseVertex = mesh.m_meshes[i].m_baseVertex;
        m_Meshes[i].BaseIndex = mesh.m_meshes[i].m_baseIndex;
        //m_Meshes[i].ValidFaces =
        m_Meshes[i].MaterialIndex = 0;
        m_Meshes[i].Transformation.InitIdentity();
    }

    m_Indices.resize(mesh.m_totalIndices);
    memcpy(m_Indices.data(), mesh.m_indexData.pMem, mesh.m_indexData.Size);

    std::vector<Vertex> Vertices(mesh.m_totalVertices);
    memcpy(Vertices.data(), mesh.m_vertexData.pMem, mesh.m_vertexData.Size);

    PopulateBuffers(Vertices);

    m_Materials.resize(1);


    mesh.Destroy();

    glBindVertexArray(0);

    return true;
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
    m_vertexBufferSizeBytes = sizeof(VertexType) * Vertices.size();
    m_indexBufferSizeBytes = sizeof(m_Indices[0]) * m_Indices.size();

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
    glNamedBufferStorage(m_Buffers[VERTEX_BUFFER], m_vertexBufferSizeBytes, Vertices.data(), GL_MAP_READ_BIT);
    glNamedBufferStorage(m_Buffers[INDEX_BUFFER], m_indexBufferSizeBytes, m_Indices.data(), GL_MAP_READ_BIT);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_INDEX_VERTICES, m_Buffers[VERTEX_BUFFER]);

    glVertexArrayElementBuffer(m_VAO, m_Buffers[INDEX_BUFFER]);
}


template<typename VertexType>
void GLModel::PopulateBuffersNonDSA(std::vector<VertexType>& Vertices)
{
    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[VERTEX_BUFFER]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[INDEX_BUFFER]);

    glBufferData(GL_ARRAY_BUFFER, m_vertexBufferSizeBytes, Vertices.data(), GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferSizeBytes, &m_Indices[0], GL_STATIC_DRAW);

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
    glNamedBufferStorage(m_Buffers[VERTEX_BUFFER], m_vertexBufferSizeBytes, Vertices.data(), 0);
    glNamedBufferStorage(m_Buffers[INDEX_BUFFER], m_indexBufferSizeBytes, m_Indices.data(), 0);

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

    BindTextures(MaterialIndex);

    if (pRenderCallbacks) {
        bool HasSpecularTexure = m_Materials[MaterialIndex].pTextures[TEX_TYPE_SPECULAR] != NULL;
        pRenderCallbacks->ControlSpecularExponent_CB(HasSpecularTexure);
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


void GLModel::BindTextures(int MaterialIndex)
{
    if (m_Materials[MaterialIndex].pTextures[TEX_TYPE_BASE]) {
        m_Materials[MaterialIndex].pTextures[TEX_TYPE_BASE]->Bind(COLOR_TEXTURE_UNIT);
    }

    if (m_Materials[MaterialIndex].pTextures[TEX_TYPE_SPECULAR]) {
        m_Materials[MaterialIndex].pTextures[TEX_TYPE_SPECULAR]->Bind(SPECULAR_EXPONENT_UNIT);
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


void GLModel::SetAmbientOcclusionMap(int TextureHandle)
{
    if (TextureHandle < 0) {
        m_pAOMap = NULL;
    } else {
        BaseTexture* pTexture = m_pCoreRenderingSystem->GetTexture(TextureHandle);
        m_pAOMap = (Texture*)pTexture;
    }
}


Texture* GLModel::AllocTexture2D()
{
    return new Texture(GL_TEXTURE_2D);
}


void GLModel::DestroyTexture(Texture* pTexture)
{
    delete pTexture;
}

