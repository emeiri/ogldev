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

#include "ogldev_engine_common.h"
#include "skinned_mesh.h"

using namespace std;

#define POSITION_LOCATION    0
#define TEX_COORD_LOCATION   1
#define NORMAL_LOCATION      2
#define BONE_ID_LOCATION     3
#define BONE_WEIGHT_LOCATION 4


SkinnedMesh::~SkinnedMesh()
{
    Clear();
}


void SkinnedMesh::Clear()
{
    if (m_Buffers[0] != 0) {
        glDeleteBuffers(ARRAY_SIZE_IN_ELEMENTS(m_Buffers), m_Buffers);
    }

    if (m_VAO != 0) {
        glDeleteVertexArrays(1, &m_VAO);
        m_VAO = 0;
    }
}


bool SkinnedMesh::LoadMesh(const string& Filename)
{
    // Release the previously loaded mesh (if it exists)
    Clear();

    // Create the VAO
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    // Create the buffers for the vertices attributes
    glGenBuffers(ARRAY_SIZE_IN_ELEMENTS(m_Buffers), m_Buffers);

    bool Ret = false;
    Assimp::Importer Importer;

    const aiScene* pScene = Importer.ReadFile(Filename.c_str(), ASSIMP_LOAD_FLAGS);

    if (pScene) {
        Ret = InitFromScene(pScene, Filename);
    }
    else {
        printf("Error parsing '%s': '%s'\n", Filename.c_str(), Importer.GetErrorString());
    }

    // Make sure the VAO is not changed from the outside
    glBindVertexArray(0);

    return Ret;
}

bool SkinnedMesh::InitFromScene(const aiScene* pScene, const string& Filename)
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


void SkinnedMesh::CountVerticesAndIndices(const aiScene* pScene, unsigned int& NumVertices, unsigned int& NumIndices)
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


void SkinnedMesh::ReserveSpace(unsigned int NumVertices, unsigned int NumIndices)
{
    m_Positions.reserve(NumVertices);
    m_Normals.reserve(NumVertices);
    m_TexCoords.reserve(NumVertices);
    m_Indices.reserve(NumIndices);
    m_Bones.resize(NumVertices);
}


void SkinnedMesh::InitAllMeshes(const aiScene* pScene)
{
    for (unsigned int i = 0 ; i < m_Meshes.size() ; i++) {
        const aiMesh* paiMesh = pScene->mMeshes[i];
        InitSingleMesh(i, paiMesh);
    }
}


void SkinnedMesh::InitSingleMesh(uint MeshIndex, const aiMesh* paiMesh)
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

    LoadMeshBones(MeshIndex, paiMesh);

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


void SkinnedMesh::LoadMeshBones(uint MeshIndex, const aiMesh* pMesh)
{
    for (uint i = 0 ; i < pMesh->mNumBones ; i++) {
        LoadSingleBone(MeshIndex, pMesh->mBones[i]);
    }
}


void SkinnedMesh::LoadSingleBone(uint MeshIndex, const aiBone* pBone)
{
    int BoneId = GetBoneId(pBone);

    for (uint i = 0 ; i < pBone->mNumWeights ; i++) {
        const aiVertexWeight& vw = pBone->mWeights[i];
        uint GlobalVertexID = m_Meshes[MeshIndex].BaseVertex + pBone->mWeights[i].mVertexId;
        m_Bones[GlobalVertexID].AddBoneData(BoneId, vw.mWeight);
    }
}


int SkinnedMesh::GetBoneId(const aiBone* pBone)
{
    int BoneIndex = 0;
    string BoneName(pBone->mName.C_Str());

    if (m_BoneNameToIndexMap.find(BoneName) == m_BoneNameToIndexMap.end()) {
        // Allocate an index for a new bone
        BoneIndex = (int)m_BoneNameToIndexMap.size();
        m_BoneNameToIndexMap[BoneName] = BoneIndex;
    }
    else {
        BoneIndex = m_BoneNameToIndexMap[BoneName];
    }

    return BoneIndex;
}


string GetDirFromFilename(const string& Filename)
{
    // Extract the directory part from the file name
    string::size_type SlashIndex;

#ifdef _WIN64
    SlashIndex = Filename.find_last_of("\\");

    if (SlashIndex == -1) {
        SlashIndex = Filename.find_last_of("/");
    }
#else
    SlashIndex = Filename.find_last_of("/");
#endif

    string Dir;

    if (SlashIndex == string::npos) {
        Dir = ".";
    }
    else if (SlashIndex == 0) {
        Dir = "/";
    }
    else {
        Dir = Filename.substr(0, SlashIndex);
    }

    return Dir;
}


bool SkinnedMesh::InitMaterials(const aiScene* pScene, const string& Filename)
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


void SkinnedMesh::LoadTextures(const string& Dir, const aiMaterial* pMaterial, int index)
{
    LoadDiffuseTexture(Dir, pMaterial, index);
    LoadSpecularTexture(Dir, pMaterial, index);
}


void SkinnedMesh::LoadDiffuseTexture(const string& Dir, const aiMaterial* pMaterial, int index)
{
    m_Materials[index].pDiffuse = NULL;

    if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
        aiString Path;

        if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
            string p(Path.data);

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
                printf("Loaded diffuse texture '%s'\n", FullPath.c_str());
            }
        }
    }
}


void SkinnedMesh::LoadSpecularTexture(const string& Dir, const aiMaterial* pMaterial, int index)
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

void SkinnedMesh::LoadColors(const aiMaterial* pMaterial, int index)
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


void SkinnedMesh::PopulateBuffers()
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

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[BONE_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_Bones[0]) * m_Bones.size(), &m_Bones[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(BONE_ID_LOCATION);
    glVertexAttribIPointer(BONE_ID_LOCATION, MAX_NUM_BONES_PER_VERTEX, GL_INT, sizeof(VertexBoneData), (const GLvoid*)0);
    glEnableVertexAttribArray(BONE_WEIGHT_LOCATION);
    glVertexAttribPointer(BONE_WEIGHT_LOCATION, MAX_NUM_BONES_PER_VERTEX, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData),
                          (const GLvoid*)(MAX_NUM_BONES_PER_VERTEX * sizeof(int32_t)));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[INDEX_BUFFER]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_Indices[0]) * m_Indices.size(), &m_Indices[0], GL_STATIC_DRAW);
}


// Introduced in youtube tutorial #18
void SkinnedMesh::Render()
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


const Material& SkinnedMesh::GetMaterial()
{
    for (unsigned int i = 0 ; i < m_Materials.size() ; i++) {
        if (m_Materials[i].AmbientColor != Vector3f(0.0f, 0.0f, 0.0f)) {
            return m_Materials[i];
        }
    }

    return m_Materials[0];
}
