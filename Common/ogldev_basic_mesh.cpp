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

#include "ogldev_basic_mesh.h"
#include "ogldev_engine_common.h"

using namespace std;

#define POSITION_LOCATION  0
#define TEX_COORD_LOCATION 1
#define NORMAL_LOCATION    2


BasicMesh::~BasicMesh()
{
    Clear();
}


void BasicMesh::Clear()
{
    if (m_Buffers[0] != 0) {
        glDeleteBuffers(ARRAY_SIZE_IN_ELEMENTS(m_Buffers), m_Buffers);
    }

    if (m_VAO != 0) {
        glDeleteVertexArrays(1, &m_VAO);
        m_VAO = 0;
    }
}


bool BasicMesh::LoadMesh(const string& Filename)
{
    // Release the previously loaded mesh (if it exists)
    Clear();

    // Create the VAO
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    // Create the buffers for the vertices attributes
    glGenBuffers(ARRAY_SIZE_IN_ELEMENTS(m_Buffers), m_Buffers);

    bool Ret = false;

    m_pScene = m_Importer.ReadFile(Filename.c_str(), ASSIMP_LOAD_FLAGS);

    if (m_pScene) {
        m_GlobalInverseTransform = m_pScene->mRootNode->mTransformation;
        m_GlobalInverseTransform = m_GlobalInverseTransform.Inverse();
        Ret = InitFromScene(m_pScene, Filename);
    }
    else {
        printf("Error parsing '%s': '%s'\n", Filename.c_str(), m_Importer.GetErrorString());
    }

    // Make sure the VAO is not changed from the outside
    glBindVertexArray(0);

    return Ret;
}

bool BasicMesh::InitFromScene(const aiScene* pScene, const string& Filename)
{
    m_Meshes.resize(pScene->mNumMeshes);
    m_Materials.resize(pScene->mNumMaterials);

    unsigned int NumVertices = 0;
    unsigned int NumIndices = 0;

    CountVerticesAndIndices(pScene, NumVertices, NumIndices);

    ReserveSpace(NumVertices, NumIndices);

    InitAllMeshes(pScene);

    if (!InitMaterials(pScene, Filename)) {
        return false;
    }

    PopulateBuffers();

    return GLCheckError();
}


void BasicMesh::CountVerticesAndIndices(const aiScene* pScene, unsigned int& NumVertices, unsigned int& NumIndices)
{
    for (unsigned int i = 0 ; i < m_Meshes.size() ; i++) {
        m_Meshes[i].MaterialIndex = pScene->mMeshes[i]->mMaterialIndex;
        m_Meshes[i].NumIndices = pScene->mMeshes[i]->mNumFaces * 3;
        m_Meshes[i].BaseVertex = NumVertices;
        m_Meshes[i].BaseIndex = NumIndices;

        NumVertices += pScene->mMeshes[i]->mNumVertices;
        NumIndices  += m_Meshes[i].NumIndices;
    }
}


void BasicMesh::ReserveSpace(unsigned int NumVertices, unsigned int NumIndices)
{
    m_Positions.reserve(NumVertices);
    m_Normals.reserve(NumVertices);
    m_TexCoords.reserve(NumVertices);
    m_Indices.reserve(NumIndices);
}


void BasicMesh::InitAllMeshes(const aiScene* pScene)
{
    for (unsigned int i = 0 ; i < m_Meshes.size() ; i++) {
        const aiMesh* paiMesh = pScene->mMeshes[i];
        InitSingleMesh(i, paiMesh);
    }
}


void BasicMesh::InitSingleMesh(uint MeshIndex, const aiMesh* paiMesh)
{
    const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

    // Populate the vertex attribute vectors
    for (unsigned int i = 0 ; i < paiMesh->mNumVertices ; i++) {

        const aiVector3D& pPos      = paiMesh->mVertices[i];
        m_Positions.push_back(Vector3f(pPos.x, pPos.y, pPos.z));

        if (paiMesh->mNormals) {
            const aiVector3D& pNormal   = paiMesh->mNormals[i];
            m_Normals.push_back(Vector3f(pNormal.x, pNormal.y, pNormal.z));
        } else {
            aiVector3D Normal(0.0f, 1.0f, 0.0f);
            m_Normals.push_back(Vector3f(Normal.x, Normal.y, Normal.z));
        }

        const aiVector3D& pTexCoord = paiMesh->HasTextureCoords(0) ? paiMesh->mTextureCoords[0][i] : Zero3D;
        m_TexCoords.push_back(Vector2f(pTexCoord.x, pTexCoord.y));
    }

    // Populate the index buffer
    for (unsigned int i = 0 ; i < paiMesh->mNumFaces ; i++) {
        const aiFace& Face = paiMesh->mFaces[i];
        //        printf("num indices %d\n", Face.mNumIndices);
        //        assert(Face.mNumIndices == 3);
        m_Indices.push_back(Face.mIndices[0]);
        m_Indices.push_back(Face.mIndices[1]);
        m_Indices.push_back(Face.mIndices[2]);
    }
}




bool BasicMesh::InitMaterials(const aiScene* pScene, const string& Filename)
{
    string Dir = GetDirFromFilename(Filename);

    bool Ret = true;

    printf("Num materials: %d\n", pScene->mNumMaterials);

    // Initialize the materials
    for (unsigned int i = 0 ; i < pScene->mNumMaterials ; i++) {
        const aiMaterial* pMaterial = pScene->mMaterials[i];

        LoadTextures(Dir, pMaterial, i);

        LoadColors(pMaterial, i);
    }

    return Ret;
}


void BasicMesh::LoadTextures(const string& Dir, const aiMaterial* pMaterial, int index)
{
    LoadDiffuseTexture(Dir, pMaterial, index);
    LoadSpecularTexture(Dir, pMaterial, index);
}


void BasicMesh::LoadDiffuseTexture(const string& Dir, const aiMaterial* pMaterial, int index)
{
    m_Materials[index].pDiffuse = NULL;

    if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
        aiString Path;

        if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
            string p(Path.data);

            for (int i = 0 ; i < p.length() ; i++) {
                if (p[i] == '\\') {
                    p[i] = '/';
                }
            }

            if (p.substr(0, 2) == ".\\") {
                p = p.substr(2, p.size() - 2);
            }

            string FullPath = Dir + "/" + p;

            m_Materials[index].pDiffuse = new Texture(GL_TEXTURE_2D, FullPath.c_str());

            if (!m_Materials[index].pDiffuse->Load()) {
                printf("Error loading diffuse texture '%s'\n", FullPath.c_str());
                exit(0);
            }
            else {
                printf("Loaded diffuse texture '%s' at index %d\n", FullPath.c_str(), index);
            }
        }
    }
}


void BasicMesh::LoadSpecularTexture(const string& Dir, const aiMaterial* pMaterial, int index)
{
    m_Materials[index].pSpecularExponent = NULL;

    if (pMaterial->GetTextureCount(aiTextureType_SHININESS) > 0) {
        aiString Path;

        if (pMaterial->GetTexture(aiTextureType_SHININESS, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
            string p(Path.data);

            if (p == "C:\\\\") {
                p = "";
            } else if (p.substr(0, 2) == ".\\") {
                p = p.substr(2, p.size() - 2);
            }

            string FullPath = Dir + "/" + p;

            m_Materials[index].pSpecularExponent = new Texture(GL_TEXTURE_2D, FullPath.c_str());

            if (!m_Materials[index].pSpecularExponent->Load()) {
                printf("Error loading specular texture '%s'\n", FullPath.c_str());
                exit(0);
            }
            else {
                printf("Loaded specular texture '%s'\n", FullPath.c_str());
            }
        }
    }
}

void BasicMesh::LoadColors(const aiMaterial* pMaterial, int index)
{
    aiColor3D AmbientColor(0.0f, 0.0f, 0.0f);
    Vector3f AllOnes(1.0f, 1.0f, 1.0f);

    int ShadingModel = 0;
    if (pMaterial->Get(AI_MATKEY_SHADING_MODEL, ShadingModel) == AI_SUCCESS) {
        printf("Shading model %d\n", ShadingModel);
    }

    if (pMaterial->Get(AI_MATKEY_COLOR_AMBIENT, AmbientColor) == AI_SUCCESS) {
        printf("Loaded ambient color [%f %f %f]\n", AmbientColor.r, AmbientColor.g, AmbientColor.b);
        m_Materials[index].AmbientColor.r = AmbientColor.r;
        m_Materials[index].AmbientColor.g = AmbientColor.g;
        m_Materials[index].AmbientColor.b = AmbientColor.b;
    } else {
        m_Materials[index].AmbientColor = AllOnes;
    }

    aiColor3D DiffuseColor(0.0f, 0.0f, 0.0f);

    if (pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, DiffuseColor) == AI_SUCCESS) {
        printf("Loaded diffuse color [%f %f %f]\n", DiffuseColor.r, DiffuseColor.g, DiffuseColor.b);
        m_Materials[index].DiffuseColor.r = DiffuseColor.r;
        m_Materials[index].DiffuseColor.g = DiffuseColor.g;
        m_Materials[index].DiffuseColor.b = DiffuseColor.b;
    }

    aiColor3D SpecularColor(0.0f, 0.0f, 0.0f);

    if (pMaterial->Get(AI_MATKEY_COLOR_SPECULAR, SpecularColor) == AI_SUCCESS) {
        printf("Loaded specular color [%f %f %f]\n", SpecularColor.r, SpecularColor.g, SpecularColor.b);
        m_Materials[index].SpecularColor.r = SpecularColor.r;
        m_Materials[index].SpecularColor.g = SpecularColor.g;
        m_Materials[index].SpecularColor.b = SpecularColor.b;
    }
}


void BasicMesh::PopulateBuffers()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[POS_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_Positions[0]) * m_Positions.size(), &m_Positions[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(POSITION_LOCATION);
    glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[TEXCOORD_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_TexCoords[0]) * m_TexCoords.size(), &m_TexCoords[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(TEX_COORD_LOCATION);
    glVertexAttribPointer(TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[NORMAL_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_Normals[0]) * m_Normals.size(), &m_Normals[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(NORMAL_LOCATION);
    glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[INDEX_BUFFER]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_Indices[0]) * m_Indices.size(), &m_Indices[0], GL_STATIC_DRAW);
}


// Introduced in youtube tutorial #18
void BasicMesh::Render(IRenderCallbacks* pRenderCallbacks)
{
    glBindVertexArray(m_VAO);

    for (unsigned int i = 0 ; i < m_Meshes.size() ; i++) {
        unsigned int MaterialIndex = m_Meshes[i].MaterialIndex;
        assert(MaterialIndex < m_Materials.size());

        if (m_Materials[MaterialIndex].pDiffuse) {
            m_Materials[MaterialIndex].pDiffuse->Bind(COLOR_TEXTURE_UNIT);
        }

        if (m_Materials[MaterialIndex].pSpecularExponent) {
            m_Materials[MaterialIndex].pSpecularExponent->Bind(SPECULAR_EXPONENT_UNIT);

            if (pRenderCallbacks) {
                pRenderCallbacks->ControlSpecularExponent(true);
            }
        } else {
            if (pRenderCallbacks) {
                pRenderCallbacks->ControlSpecularExponent(false);
            }
        }

        if (pRenderCallbacks) {
            pRenderCallbacks->DrawStartCB(i);
            pRenderCallbacks->SetMaterial(m_Materials[MaterialIndex]);
        }

        glDrawElementsBaseVertex(GL_TRIANGLES,
                                 m_Meshes[i].NumIndices,
                                 GL_UNSIGNED_INT,
                                 (void*)(sizeof(unsigned int) * m_Meshes[i].BaseIndex),
                                 m_Meshes[i].BaseVertex);
    }

    // Make sure the VAO is not changed from the outside
    glBindVertexArray(0);
}


void BasicMesh::Render(unsigned int DrawIndex, unsigned int PrimID)
{
    glBindVertexArray(m_VAO);

    unsigned int MaterialIndex = m_Meshes[DrawIndex].MaterialIndex;
    assert(MaterialIndex < m_Materials.size());

    if (m_Materials[MaterialIndex].pDiffuse) {
        m_Materials[MaterialIndex].pDiffuse->Bind(COLOR_TEXTURE_UNIT);
    }

    if (m_Materials[MaterialIndex].pSpecularExponent) {
        m_Materials[MaterialIndex].pSpecularExponent->Bind(SPECULAR_EXPONENT_UNIT);
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
void BasicMesh::Render(unsigned int NumInstances, const Matrix4f* WVPMats, const Matrix4f* WorldMats)
{
    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[WVP_MAT_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Matrix4f) * NumInstances, WVPMats, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[WORLD_MAT_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Matrix4f) * NumInstances, WorldMats, GL_DYNAMIC_DRAW);

    glBindVertexArray(m_VAO);

    for (unsigned int i = 0 ; i < m_Meshes.size() ; i++) {
        const unsigned int MaterialIndex = m_Meshes[i].MaterialIndex;

        assert(MaterialIndex < m_Materials.size());

        if (m_Materials[MaterialIndex].pDiffuse) {
            m_Materials[MaterialIndex].pDiffuse->Bind(COLOR_TEXTURE_UNIT);
        }

        if (m_Materials[MaterialIndex].pSpecularExponent) {
            m_Materials[MaterialIndex].pSpecularExponent->Bind(SPECULAR_EXPONENT_UNIT);
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


const Material& BasicMesh::GetMaterial()
{
    for (unsigned int i = 0 ; i < m_Materials.size() ; i++) {
        if (m_Materials[i].AmbientColor != Vector3f(0.0f, 0.0f, 0.0f)) {
            return m_Materials[i];
        }
    }

    return m_Materials[0];
}


void BasicMesh::GetLeadingVertex(uint DrawIndex, uint PrimID, Vector3f& Vertex)
{
    uint MeshIndex = DrawIndex; // Each mesh is rendered in its own draw call

    assert(MeshIndex < m_pScene->mNumMeshes);
    const aiMesh* paiMesh = m_pScene->mMeshes[MeshIndex];

    assert(PrimID < paiMesh->mNumFaces);
    const aiFace& Face = paiMesh->mFaces[PrimID];

    uint LeadingIndex = Face.mIndices[0];

    assert(LeadingIndex < paiMesh->mNumVertices);
    const aiVector3D& Pos = paiMesh->mVertices[LeadingIndex];
    Vertex.x = Pos.x;
    Vertex.y = Pos.y;
    Vertex.z = Pos.z;
}
