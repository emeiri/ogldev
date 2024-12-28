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

#include "3rdparty/meshoptimizer/src/meshoptimizer.h"

using namespace std;

#define POSITION_LOCATION  0
#define TEX_COORD_LOCATION 1
#define NORMAL_LOCATION    2


std::string GetFullPath(const string& Dir, const aiString& Path)
{
    string p(Path.data);

    if (p == "C:\\\\") {
        p = "";
    }
    else if (p.substr(0, 2) == ".\\") {
        p = p.substr(2, p.size() - 2);
    }

    string FullPath = Dir + "/" + p;

    return FullPath;
}


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


bool BasicMesh::LoadMesh(const string& Filename, int AssimpFlags)
{
    // Release the previously loaded mesh (if it exists)
    Clear();

    // Create the VAO
    if (IsGLVersionHigher(4, 5)) {
        glCreateVertexArrays(1, &m_VAO);
        glCreateBuffers(ARRAY_SIZE_IN_ELEMENTS(m_Buffers), m_Buffers);
    } else {
        glGenVertexArrays(1, &m_VAO);
        glBindVertexArray(m_VAO);
        glGenBuffers(ARRAY_SIZE_IN_ELEMENTS(m_Buffers), m_Buffers);
    }

    bool Ret = false;

    m_pScene = m_Importer.ReadFile(Filename.c_str(), AssimpFlags);

    if (m_pScene) {
        m_GlobalInverseTransform = m_pScene->mRootNode->mTransformation;
        m_GlobalInverseTransform = m_GlobalInverseTransform.Inverse();
        Ret = InitFromScene(m_pScene, Filename);
    }
    else {
        printf("Error parsing '%s': '%s'\n", Filename.c_str(), m_Importer.GetErrorString());
    }

    // Make sure the VAO is not changed from the outside
    if (!IsGLVersionHigher(4, 5)) {
        glBindVertexArray(0);
    }

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
    m_Vertices.reserve(NumVertices);
    m_Indices.reserve(NumIndices);
}


void BasicMesh::InitAllMeshes(const aiScene* pScene)
{
    for (unsigned int i = 0 ; i < m_Meshes.size() ; i++) {
        const aiMesh* paiMesh = pScene->mMeshes[i];
#ifdef USE_MESH_OPTIMIZER
        InitSingleMeshOpt(i, paiMesh);
#else
        InitSingleMesh(i, paiMesh);
#endif
    }
}


void BasicMesh::InitSingleMesh(uint MeshIndex, const aiMesh* paiMesh)
{
    const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

    // printf("Mesh %d\n", MeshIndex);
    // Populate the vertex attribute vectors
    Vertex v;

    for (unsigned int i = 0; i < paiMesh->mNumVertices; i++) {
        const aiVector3D& pPos = paiMesh->mVertices[i];
       // printf("%d: ", i); Vector3f t(pPos.x, pPos.y, pPos.z); t.Print();
        v.Position = Vector3f(pPos.x, pPos.y, pPos.z);

        if (paiMesh->mNormals) {
            const aiVector3D& pNormal = paiMesh->mNormals[i];
            v.Normal = Vector3f(pNormal.x, pNormal.y, pNormal.z);
        } else {
            aiVector3D Normal(0.0f, 1.0f, 0.0f);
            v.Normal = Vector3f(Normal.x, Normal.y, Normal.z);
        }

        const aiVector3D& pTexCoord = paiMesh->HasTextureCoords(0) ? paiMesh->mTextureCoords[0][i] : Zero3D;
        v.TexCoords = Vector2f(pTexCoord.x, pTexCoord.y);

        m_Vertices.push_back(v);
    }

    // Populate the index buffer
    for (unsigned int i = 0; i < paiMesh->mNumFaces; i++) {
        const aiFace& Face = paiMesh->mFaces[i];
        m_Indices.push_back(Face.mIndices[0]);
        m_Indices.push_back(Face.mIndices[1]);
        m_Indices.push_back(Face.mIndices[2]);
    }
}


void BasicMesh::InitSingleMeshOpt(uint MeshIndex, const aiMesh* paiMesh)
{
    const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

    // printf("Mesh %d\n", MeshIndex);
    // Populate the vertex attribute vectors
    Vertex v;

    std::vector<Vertex> Vertices(paiMesh->mNumVertices);

    for (unsigned int i = 0; i < paiMesh->mNumVertices; i++) {
        const aiVector3D& pPos = paiMesh->mVertices[i];
        // printf("%d: ", i); Vector3f v(pPos.x, pPos.y, pPos.z); v.Print();
        v.Position = Vector3f(pPos.x, pPos.y, pPos.z);

        if (paiMesh->mNormals) {
            const aiVector3D& pNormal = paiMesh->mNormals[i];
            v.Normal = Vector3f(pNormal.x, pNormal.y, pNormal.z);
        }
        else {
            aiVector3D Normal(0.0f, 1.0f, 0.0f);
            v.Normal = Vector3f(Normal.x, Normal.y, Normal.z);
        }

        const aiVector3D& pTexCoord = paiMesh->HasTextureCoords(0) ? paiMesh->mTextureCoords[0][i] : Zero3D;
        v.TexCoords = Vector2f(pTexCoord.x, pTexCoord.y);

        Vertices[i] = v;
    }

    m_Meshes[MeshIndex].BaseVertex = (uint)m_Vertices.size();
    m_Meshes[MeshIndex].BaseIndex = (uint)m_Indices.size();

    int NumIndices = paiMesh->mNumFaces * 3;

    std::vector<uint> Indices;
    Indices.resize(NumIndices);

    // Populate the index buffer
    for (unsigned int i = 0; i < paiMesh->mNumFaces; i++) {
        const aiFace& Face = paiMesh->mFaces[i];
        Indices[i * 3 + 0] = Face.mIndices[0];
        Indices[i * 3 + 1] = Face.mIndices[1];
        Indices[i * 3 + 2] = Face.mIndices[2];
    }

    OptimizeMesh(MeshIndex, Indices, Vertices);
}


void BasicMesh::OptimizeMesh(int MeshIndex, std::vector<uint>&Indices, std::vector<Vertex>&Vertices)
{
    size_t NumIndices = Indices.size();
    size_t NumVertices = Vertices.size();

    // Create a remap table
    std::vector<unsigned int> remap(NumIndices);
    size_t OptVertexCount = meshopt_generateVertexRemap(remap.data(),    // dst addr
                                                        Indices.data(),  // src indices
                                                        NumIndices,      // ...and size
                                                        Vertices.data(), // src vertices
                                                        NumVertices,     // ...and size
                                                        sizeof(Vertex)); // stride
    // Allocate a local index/vertex arrays
    std::vector<uint> OptIndices;
    std::vector<Vertex> OptVertices;
    OptIndices.resize(NumIndices);
    OptVertices.resize(OptVertexCount);

    // Optimization #1: remove duplicate vertices    
    meshopt_remapIndexBuffer(OptIndices.data(), Indices.data(), NumIndices, remap.data());

    meshopt_remapVertexBuffer(OptVertices.data(), Vertices.data(), NumVertices, sizeof(Vertex), remap.data());

    // Optimization #2: improve the locality of the vertices
    meshopt_optimizeVertexCache(OptIndices.data(), OptIndices.data(), NumIndices, OptVertexCount);

    // Optimization #3: reduce pixel overdraw
    meshopt_optimizeOverdraw(OptIndices.data(), OptIndices.data(), NumIndices, &(OptVertices[0].Position.x), OptVertexCount, sizeof(Vertex), 1.05f);

    // Optimization #4: optimize access to the vertex buffer
    meshopt_optimizeVertexFetch(OptVertices.data(), OptIndices.data(), NumIndices, OptVertices.data(), OptVertexCount, sizeof(Vertex));

    // Optimization #5: create a simplified version of the model
    float Threshold = 1.0f;
    size_t TargetIndexCount = (size_t)(NumIndices * Threshold);
    
    float TargetError = 0.0f;
    std::vector<unsigned int> SimplifiedIndices(OptIndices.size());
    size_t OptIndexCount = meshopt_simplify(SimplifiedIndices.data(), OptIndices.data(), NumIndices,
                                            &OptVertices[0].Position.x, OptVertexCount, sizeof(Vertex), TargetIndexCount, TargetError);

    static int num_indices = 0;
    num_indices += (int)NumIndices;
    static int opt_indices = 0;
    opt_indices += (int)OptIndexCount;
    printf("Num indices %d\n", num_indices);
    //printf("Target num indices %d\n", TargetIndexCount);
    printf("Optimized number of indices %d\n", opt_indices);
    SimplifiedIndices.resize(OptIndexCount);
    
    // Concatenate the local arrays into the class attributes arrays
    m_Indices.insert(m_Indices.end(), SimplifiedIndices.begin(), SimplifiedIndices.end());

    m_Vertices.insert(m_Vertices.end(), OptVertices.begin(), OptVertices.end());

    m_Meshes[MeshIndex].NumIndices = (uint)OptIndexCount;
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


void BasicMesh::LoadTextures(const string& Dir, const aiMaterial* pMaterial, int Index)
{
    LoadDiffuseTexture(Dir, pMaterial, Index);
    LoadSpecularTexture(Dir, pMaterial, Index);

    // PBR
    LoadAlbedoTexture(Dir, pMaterial, Index);
    LoadMetalnessTexture(Dir, pMaterial, Index);
    LoadRoughnessTexture(Dir, pMaterial, Index);
}


void BasicMesh::LoadDiffuseTexture(const string& Dir, const aiMaterial* pMaterial, int MaterialIndex)
{
    m_Materials[MaterialIndex].pDiffuse = NULL;

    if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
        aiString Path;

        if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
            const aiTexture* paiTexture = m_pScene->GetEmbeddedTexture(Path.C_Str());

            if (paiTexture) {
                LoadDiffuseTextureEmbedded(paiTexture, MaterialIndex);
            } else {
                LoadDiffuseTextureFromFile(Dir, Path, MaterialIndex);
            }
        }
    }
}


void BasicMesh::LoadDiffuseTextureEmbedded(const aiTexture* paiTexture, int MaterialIndex)
{
    printf("Embeddeded diffuse texture type '%s'\n", paiTexture->achFormatHint);
    m_Materials[MaterialIndex].pDiffuse = new Texture(GL_TEXTURE_2D);
    int buffer_size = paiTexture->mWidth;
    m_Materials[MaterialIndex].pDiffuse->Load(buffer_size, paiTexture->pcData);
}


void BasicMesh::LoadDiffuseTextureFromFile(const string& Dir, const aiString& Path, int MaterialIndex)
{
    string FullPath = GetFullPath(Dir, Path);

    m_Materials[MaterialIndex].pDiffuse = new Texture(GL_TEXTURE_2D, FullPath.c_str());

    if (!m_Materials[MaterialIndex].pDiffuse->Load()) {
        printf("Error loading diffuse texture '%s'\n", FullPath.c_str());
        exit(0);
    }
    else {
        printf("Loaded diffuse texture '%s' at index %d\n", FullPath.c_str(), MaterialIndex);
    }
}


void BasicMesh::LoadSpecularTexture(const string& Dir, const aiMaterial* pMaterial, int MaterialIndex)
{
    m_Materials[MaterialIndex].pSpecularExponent = NULL;

    if (pMaterial->GetTextureCount(aiTextureType_SHININESS) > 0) {
        aiString Path;

        if (pMaterial->GetTexture(aiTextureType_SHININESS, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
            const aiTexture* paiTexture = m_pScene->GetEmbeddedTexture(Path.C_Str());

            if (paiTexture) {
                LoadSpecularTextureEmbedded(paiTexture, MaterialIndex);
            } else {
                LoadSpecularTextureFromFile(Dir, Path, MaterialIndex);
            }
        }
    }
}


void BasicMesh::LoadSpecularTextureEmbedded(const aiTexture* paiTexture, int MaterialIndex)
{
    printf("Embeddeded specular texture type '%s'\n", paiTexture->achFormatHint);
    m_Materials[MaterialIndex].pSpecularExponent = new Texture(GL_TEXTURE_2D);
    int buffer_size = paiTexture->mWidth;
    m_Materials[MaterialIndex].pSpecularExponent->Load(buffer_size, paiTexture->pcData);
}


void BasicMesh::LoadSpecularTextureFromFile(const string& Dir, const aiString& Path, int MaterialIndex)
{
    string FullPath = GetFullPath(Dir, Path);

    m_Materials[MaterialIndex].pSpecularExponent = new Texture(GL_TEXTURE_2D, FullPath.c_str());

    if (!m_Materials[MaterialIndex].pSpecularExponent->Load()) {
        printf("Error loading specular texture '%s'\n", FullPath.c_str());
        exit(0);
    }
    else {
        printf("Loaded specular texture '%s'\n", FullPath.c_str());
    }
}


void BasicMesh::LoadAlbedoTexture(const string& Dir, const aiMaterial* pMaterial, int MaterialIndex)
{
    m_Materials[MaterialIndex].PBRmaterial.pAlbedo = NULL;

    if (pMaterial->GetTextureCount(aiTextureType_BASE_COLOR) > 0) {
        aiString Path;

        if (pMaterial->GetTexture(aiTextureType_BASE_COLOR, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
            const aiTexture* paiTexture = m_pScene->GetEmbeddedTexture(Path.C_Str());

            if (paiTexture) {
                LoadAlbedoTextureEmbedded(paiTexture, MaterialIndex);
            } else {
                LoadAlbedoTextureFromFile(Dir, Path, MaterialIndex);
            }
        }
    }
}


void BasicMesh::LoadAlbedoTextureEmbedded(const aiTexture* paiTexture, int MaterialIndex)
{
    printf("Embeddeded albedo texture type '%s'\n", paiTexture->achFormatHint);
    m_Materials[MaterialIndex].PBRmaterial.pAlbedo = new Texture(GL_TEXTURE_2D);
    int buffer_size = paiTexture->mWidth;
    m_Materials[MaterialIndex].PBRmaterial.pAlbedo->Load(buffer_size, paiTexture->pcData);
}


void BasicMesh::LoadAlbedoTextureFromFile(const string& Dir, const aiString& Path, int MaterialIndex)
{
    string FullPath = GetFullPath(Dir, Path);

    m_Materials[MaterialIndex].PBRmaterial.pAlbedo = new Texture(GL_TEXTURE_2D, FullPath.c_str());

    if (!m_Materials[MaterialIndex].PBRmaterial.pAlbedo->Load()) {
        printf("Error loading albedo texture '%s'\n", FullPath.c_str());
        exit(0);
    }
    else {
        printf("Loaded albedo texture '%s'\n", FullPath.c_str());
    }
}


void BasicMesh::LoadMetalnessTexture(const string& Dir, const aiMaterial* pMaterial, int MaterialIndex)
{
    m_Materials[MaterialIndex].PBRmaterial.pMetallic = NULL;

    int NumTextures = pMaterial->GetTextureCount(aiTextureType_METALNESS);

    if (NumTextures > 0) {
        printf("Num metalness textures %d\n", NumTextures);

        aiString Path;

        if (pMaterial->GetTexture(aiTextureType_METALNESS, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
            const aiTexture* paiTexture = m_pScene->GetEmbeddedTexture(Path.C_Str());

            if (paiTexture) {
                LoadMetalnessTextureEmbedded(paiTexture, MaterialIndex);
            }
            else {
                LoadMetalnessTextureFromFile(Dir, Path, MaterialIndex);
            }
        }
    }
}


void BasicMesh::LoadMetalnessTextureEmbedded(const aiTexture* paiTexture, int MaterialIndex)
{
    printf("Embeddeded metalness texture type '%s'\n", paiTexture->achFormatHint);
    m_Materials[MaterialIndex].PBRmaterial.pMetallic = new Texture(GL_TEXTURE_2D);
    int buffer_size = paiTexture->mWidth;
    m_Materials[MaterialIndex].PBRmaterial.pMetallic->Load(buffer_size, paiTexture->pcData);
}


void BasicMesh::LoadMetalnessTextureFromFile(const string& Dir, const aiString& Path, int MaterialIndex)
{
    string FullPath = GetFullPath(Dir, Path);

    m_Materials[MaterialIndex].PBRmaterial.pMetallic = new Texture(GL_TEXTURE_2D, FullPath.c_str());

    if (!m_Materials[MaterialIndex].PBRmaterial.pMetallic->Load()) {
        printf("Error loading metalness texture '%s'\n", FullPath.c_str());
        exit(0);
    }
    else {
        printf("Loaded metalness texture '%s'\n", FullPath.c_str());
    }
}


void BasicMesh::LoadRoughnessTexture(const string& Dir, const aiMaterial* pMaterial, int MaterialIndex)
{
    m_Materials[MaterialIndex].PBRmaterial.pRoughness = NULL;

    int NumTextures = pMaterial->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS);

    if (NumTextures > 0) {
        printf("Num roughness textures %d\n", NumTextures);

        aiString Path;

        if (pMaterial->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
            const aiTexture* paiTexture = m_pScene->GetEmbeddedTexture(Path.C_Str());

            if (paiTexture) {
                LoadRoughnessTextureEmbedded(paiTexture, MaterialIndex);
            }
            else {
                LoadRoughnessTextureFromFile(Dir, Path, MaterialIndex);
            }
        }
    }
}


void BasicMesh::LoadRoughnessTextureEmbedded(const aiTexture* paiTexture, int MaterialIndex)
{
    printf("Embeddeded roughness texture type '%s'\n", paiTexture->achFormatHint);
    m_Materials[MaterialIndex].PBRmaterial.pRoughness = new Texture(GL_TEXTURE_2D);
    int buffer_size = paiTexture->mWidth;
    m_Materials[MaterialIndex].PBRmaterial.pRoughness->Load(buffer_size, paiTexture->pcData);
}


void BasicMesh::LoadRoughnessTextureFromFile(const string& Dir, const aiString& Path, int MaterialIndex)
{
    string FullPath = GetFullPath(Dir, Path);

    m_Materials[MaterialIndex].PBRmaterial.pRoughness = new Texture(GL_TEXTURE_2D, FullPath.c_str());

    if (!m_Materials[MaterialIndex].PBRmaterial.pRoughness->Load()) {
        printf("Error loading roughness texture '%s'\n", FullPath.c_str());
        exit(0);
    }
    else {
        printf("Loaded roughness texture '%s'\n", FullPath.c_str());
    }
}


void BasicMesh::LoadColors(const aiMaterial* pMaterial, int index)
{
    aiColor4D AmbientColor(0.0f, 0.0f, 0.0f, 0.0f);
    Vector4f AllOnes(1.0f);

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
    if (IsGLVersionHigher(4, 5)) {
        PopulateBuffersDSA();
    } else {
        PopulateBuffersNonDSA();
    }
}


void BasicMesh::PopulateBuffersNonDSA()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[VERTEX_BUFFER]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[INDEX_BUFFER]);

    glBufferData(GL_ARRAY_BUFFER, sizeof(m_Vertices[0]) * m_Vertices.size(), &m_Vertices[0], GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_Indices[0]) * m_Indices.size(), &m_Indices[0], GL_STATIC_DRAW);
    
    size_t NumFloats = 0;

    glEnableVertexAttribArray(POSITION_LOCATION);
    glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(NumFloats * sizeof(float)));
    NumFloats += 3;

    glEnableVertexAttribArray(TEX_COORD_LOCATION);
    glVertexAttribPointer(TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(NumFloats * sizeof(float)));
    NumFloats += 2;

    glEnableVertexAttribArray(NORMAL_LOCATION);
    glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(NumFloats * sizeof(float)));
}


void BasicMesh::PopulateBuffersDSA()
{
    glNamedBufferStorage(m_Buffers[VERTEX_BUFFER], sizeof(m_Vertices[0]) * m_Vertices.size(), m_Vertices.data(), 0);
    glNamedBufferStorage(m_Buffers[INDEX_BUFFER], sizeof(m_Indices[0]) * m_Indices.size(), m_Indices.data(), 0);

    glVertexArrayVertexBuffer(m_VAO, 0, m_Buffers[VERTEX_BUFFER], 0, sizeof(Vertex));
    glVertexArrayElementBuffer(m_VAO, m_Buffers[INDEX_BUFFER]);

    size_t NumFloats = 0;

    glEnableVertexArrayAttrib(m_VAO, POSITION_LOCATION);
    glVertexArrayAttribFormat(m_VAO, POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, (GLuint)(NumFloats * sizeof(float)));
    glVertexArrayAttribBinding(m_VAO, POSITION_LOCATION, 0);
    NumFloats += 3;

    glEnableVertexArrayAttrib(m_VAO, TEX_COORD_LOCATION);
    glVertexArrayAttribFormat(m_VAO, TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE, (GLuint)(NumFloats * sizeof(float)));
    glVertexArrayAttribBinding(m_VAO, TEX_COORD_LOCATION, 0);
    NumFloats += 2;

    glEnableVertexArrayAttrib(m_VAO, NORMAL_LOCATION);
    glVertexArrayAttribFormat(m_VAO, NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, (GLuint)(NumFloats * sizeof(float)));
    glVertexArrayAttribBinding(m_VAO, NORMAL_LOCATION, 0);
}


// Introduced in youtube tutorial #18
void BasicMesh::Render(IRenderCallbacks* pRenderCallbacks)
{
    if (m_isPBR) {
        SetupRenderMaterialsPBR();
    }

    glBindVertexArray(m_VAO);

    for (unsigned int MeshIndex = 0 ; MeshIndex < m_Meshes.size() ; MeshIndex++) {
        unsigned int MaterialIndex = m_Meshes[MeshIndex].MaterialIndex;
        assert(MaterialIndex < m_Materials.size());

        if (!m_isPBR) {
            SetupRenderMaterialsPhong(MeshIndex, MaterialIndex, pRenderCallbacks);
        }

        glDrawElementsBaseVertex(GL_TRIANGLES,
                                 m_Meshes[MeshIndex].NumIndices,
                                 GL_UNSIGNED_INT,
                                 (void*)(sizeof(unsigned int) * m_Meshes[MeshIndex].BaseIndex),
                                 m_Meshes[MeshIndex].BaseVertex);
    }

    // Make sure the VAO is not changed from the outside
    glBindVertexArray(0);
}


void BasicMesh::SetupRenderMaterialsPhong(unsigned int MeshIndex, unsigned int MaterialIndex, IRenderCallbacks* pRenderCallbacks)
{
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
        if (m_Materials[MaterialIndex].pDiffuse) {
            pRenderCallbacks->DrawStartCB(MeshIndex);
            pRenderCallbacks->SetMaterial(m_Materials[MaterialIndex]);
        }  else {
            pRenderCallbacks->DisableDiffuseTexture();
        }
    }
}


void BasicMesh::SetupRenderMaterialsPBR()
{
    int PBRMaterialIndex = 0;

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
    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[WVP_MAT_BUFFER]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Matrix4f) * NumInstances, WVPMats, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[WORLD_MAT_BUFFER]);
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
        if (m_Materials[i].AmbientColor != Vector4f(0.0f)) {
            return m_Materials[i];
        }
    }

    if (m_Materials.size() == 0) {
        printf("No materials\n");
        exit(0);
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
