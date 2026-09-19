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

#include <algorithm>
#include <filesystem>
#include <assimp/GltfMaterial.h>

#include "Int/core_rendering_system.h"
#include "Int/core_model.h"
#include "3rdparty/meshoptimizer/src/meshoptimizer.h"

//#define DEBUG_COLORS
//#define DEBUG_MATERIALS
//#define DEBUG_TEXTURES
//#define DEBUG_BONES
#define DEBUG_SCENE_HIERARCHY

// config flags
static bool UseMeshOptimizer = false;
static bool MissingTextureDetection = false;

#ifdef OGLDEV_VULKAN

#if 0
#define DEMOLITION_ASSIMP_LOAD_FLAGS (\
aiProcess_Triangulate |         \
aiProcess_GenSmoothNormals |     \
aiProcess_CalcTangentSpace |      \
aiProcess_JoinIdenticalVertices |  \
aiProcess_RemoveRedundantMaterials | \
aiProcess_SortByPType |              \
aiProcess_GenUVCoords |              \
aiProcess_TransformUVCoords |        \
aiProcess_FlipUVs |                  \
aiProcess_ValidateDataStructure |    \
aiProcess_OptimizeMeshes |           \
aiProcess_FlipWindingOrder | \
aiProcess_OptimizeGraph)            
#endif

#define DEMOLITION_ASSIMP_LOAD_FLAGS (\
aiProcess_Triangulate | \
aiProcess_GenSmoothNormals |    \
aiProcess_CalcTangentSpace |    \
aiProcess_JoinIdenticalVertices |   \
aiProcess_SortByPType |              \
aiProcess_GenUVCoords | \
aiProcess_ValidateDataStructure | \
aiProcess_RemoveRedundantMaterials | \
aiProcess_FindDegenerates | \
aiProcess_FindInvalidData | \
aiProcess_LimitBoneWeights | \
aiProcess_ImproveCacheLocality)

#else

#define DEMOLITION_ASSIMP_LOAD_FLAGS (aiProcess_JoinIdenticalVertices | \
                                      aiProcess_Triangulate | \
                                      aiProcess_GenSmoothNormals | \
                                      aiProcess_LimitBoneWeights | \
                                      aiProcess_SplitLargeMeshes | \
                                      aiProcess_ImproveCacheLocality | \
                                      aiProcess_RemoveRedundantMaterials | \
                                      aiProcess_FindDegenerates | \
                                      aiProcess_FindInvalidData | \
                                      aiProcess_GenUVCoords | \
                                      aiProcess_CalcTangentSpace)
#endif
//aiProcess_MakeLeftHanded | \


static void traverse(int depth, aiNode* pNode);
static bool GetFullTransformation(const aiNode* pRootNode, const char* pName, Matrix4f& Transformation);

inline Vector3f VectorFromAssimpVector(const aiVector3D& v)
{
    Vector3f ret;

    ret.x = v.x;
    ret.y = v.y;
    ret.z = v.z;

    return ret;
}


static std::string GetFullPath(const std::string& Dir, const aiString& Path) 
{
    std::filesystem::path p(Path.C_Str());
    std::string ret = (std::filesystem::path(Dir) / p).generic_string();
    // .generic_string() automatically converts windows \ to /
    return ret;
}


static void SetMaterialType(const aiMaterial* pMaterial, CoreMaterial& MyMaterial)
{
    int ShadingModel = 0;

    if (pMaterial->Get(AI_MATKEY_SHADING_MODEL, ShadingModel) != AI_SUCCESS) {
        printf("Cannot get the shading model\n");
        assert(0);
    }

    MaterialTypeFlags mt = MaterialType_Invalid;

    switch (ShadingModel) {
        case aiShadingMode_Unlit:
#ifdef DEBUG_MATERIALS
            printf("Shading model: Unlit\n");
#endif
            mt = MaterialType_Unlit;
            break;

        case aiShadingMode_PBR_BRDF:
        {
#ifdef DEBUG_MATERIALS
            printf("Shading model: PBR BRDF\n");
#endif
            MyMaterial.m_isPBR = true;            
            
            float factor = 0;
            if (pMaterial->Get(AI_MATKEY_METALLIC_FACTOR, factor) == AI_SUCCESS) {
                mt = MaterialType_MetallicRoughness;
            }
        }
            break;

        case aiShadingMode_Gouraud:
#ifdef DEBUG_MATERIALS
            printf("Shading model: Gouraud\n");
#endif
            break;

        case aiShadingMode_Phong:
#ifdef DEBUG_MATERIALS
            printf("Shading model: Phong\n");
#endif
            break;

        default:
            assert(0);
    }

    MyMaterial.m_materialType = mt;
}


bool CoreModel::IsPBR() const
{
    bool ret = false;

    if (m_Materials.size() > 0) {
        // Currently checking only the first material
        ret = m_Materials[0].m_isPBR;
    }
        
    return ret;
}


void CoreModel::DestroyModel() 
{
    for (CoreMaterial& material : m_Materials) {
        for (Texture* pTexture : material.pTextures) {
            if (pTexture) {
                DestroyTexture(pTexture); // Assumes this handles GPU memory only!
            }
        }
    }
}


bool CoreModel::LoadAssimpModel(const std::string& Filename, const ModelLoadFlags& Flags)
{
    m_fileName = Filename;

#ifndef OGLDEV_VULKAN
    std::string FileExt = Filename.substr(Filename.find_last_of(".") + 1);
    m_flipTexCoordV = (FileExt == "glb" || FileExt == "gltf");
#endif

    AllocBuffers();

    bool Ret = false;

    uint LoadFlags = DEMOLITION_ASSIMP_LOAD_FLAGS;

    if (Flags.ConvertToLeftHanded) {
        LoadFlags |= aiProcess_ConvertToLeftHanded;
    }

    m_pScene = m_Importer.ReadFile(Filename.c_str(), LoadFlags);

    if (m_pScene) {
        printf("--- START Node Hierarchy ---\n");
        traverse(0, m_pScene->mRootNode);
        printf("--- END Node Hierarchy ---\n");
        m_GlobalInverseTransform = m_pScene->mRootNode->mTransformation;
        m_GlobalInverseTransform = m_GlobalInverseTransform.Inverse();
        Ret = InitFromScene(m_pScene, Filename);
    }
    else {
        printf("Error parsing '%s': '%s'\n", Filename.c_str(), m_Importer.GetErrorString());
    }

#ifndef OGLDEV_VULKAN // TODO: move to GLModel using virtual function
    // Make sure the VAO is not changed from the outside
    glBindVertexArray(0);
#endif

    return Ret;
}


bool CoreModel::InitFromScene(const aiScene* pScene, const std::string& Filename)
{
    if (!InitGeometry(pScene, Filename)) {
        return false;
    }    

    InitCameras(pScene);

    InitLights(pScene);

    return true;
}


bool CoreModel::InitGeometry(const aiScene* pScene, const std::string& Filename)
{
    printf("\n*** Initializing geometry ***\n");
    m_numAnimations = pScene->mNumAnimations;
    m_Meshes.resize(pScene->mNumMeshes);
    m_Materials.resize(pScene->mNumMaterials);

    unsigned int NumVertices = 0;
    unsigned int NumIndices = 0;

    CountVerticesAndIndices(pScene, NumVertices, NumIndices);

    InitBuffers(pScene, NumVertices, NumIndices);

    if (!InitMaterials(pScene, Filename)) {
        return false;
    }    

    CalculateMeshTransformations(pScene);

    InitGeometryPost();

#ifdef OGLDEV_VULKAN
    return true;
#else
    return GLCheckError();
#endif
}


void CoreModel::InitBuffers(const aiScene* pScene, unsigned int NumVertices, unsigned int NumIndices)
{
    printf("Num animations %d\n", pScene->mNumAnimations);

    if (pScene->mNumAnimations > 0) {
        std::vector<SkinnedVertex> Vertices;
        InitGeometryInternal<SkinnedVertex>(Vertices, NumVertices, NumIndices);
        PopulateBuffersSkinned(Vertices);
    }
    else {
        std::vector<Vertex> Vertices;
        InitGeometryInternal<Vertex>(Vertices, NumVertices, NumIndices);
        PopulateBuffers(Vertices);
    }
}


template<typename VertexType>
void CoreModel::InitGeometryInternal(std::vector<VertexType>& Vertices, int NumVertices, int NumIndices)
{
    ReserveSpace<VertexType>(Vertices, NumVertices, NumIndices);

    InitAllMeshes<VertexType>(m_pScene, Vertices);

    printf("Min pos: "); m_minPos.Print();
    printf("Max pos: "); m_maxPos.Print();
}


void CoreModel::CountVerticesAndIndices(const aiScene* pScene, uint& NumVertices, uint& NumIndices)
{
    for (unsigned int i = 0 ; i < m_Meshes.size() ; i++) {
        m_Meshes[i].MaterialIndex = pScene->mMeshes[i]->mMaterialIndex;
        m_Meshes[i].ValidFaces = CountValidFaces(*pScene->mMeshes[i]);

        NumVertices += pScene->mMeshes[i]->mNumVertices;
        NumIndices  += m_Meshes[i].NumIndices;
    }
}


uint CoreModel::CountValidFaces(const aiMesh& Mesh)
{
    uint NumValidFaces = 0;

    for (uint i = 0; i < Mesh.mNumFaces; i++) {
        if (Mesh.mFaces[i].mNumIndices == 3) {
            NumValidFaces++;
        }
    }

    return NumValidFaces;
}


template<typename VertexType>
void CoreModel::ReserveSpace(std::vector<VertexType>& Vertices, unsigned int NumVertices, unsigned int NumIndices)
{
    Vertices.reserve(NumVertices);
    m_Indices.reserve(NumIndices);
    //m_Bones.resize(NumVertices); // TODO: only if there are any bones
    InitializeRequiredNodeMap(m_pScene->mRootNode);	
}


template<typename VertexType>
void CoreModel::InitAllMeshes(const aiScene* pScene, std::vector<VertexType>& Vertices)
{
    for (unsigned int i = 0 ; i < m_Meshes.size() ; i++) {
        const aiMesh* paiMesh = pScene->mMeshes[i];

        if (UseMeshOptimizer) {
            InitSingleMeshOpt<VertexType>(Vertices, i, paiMesh);
        } else {
            InitSingleMesh<VertexType>(Vertices, i, paiMesh);
        }
    }
}


void CoreModel::CalculateMeshTransformations(const aiScene* pScene)
{
    printf("----------------------------------------\n");
    printf("Calculating mesh transformations\n");
    Matrix4f Transformation;
    Transformation.InitIdentity();

    TraverseNodeHierarchy(Transformation, pScene->mRootNode);
}


void CoreModel::TraverseNodeHierarchy(const Matrix4f& ParentTransformation, aiNode* pNode)
{
#ifdef DEBUG_SCENE_HIERARCHY
    printf("Traversing node '%s'\n", pNode->mName.C_Str());
#endif
    Matrix4f NodeTransformation(pNode->mTransformation);    

    Matrix4f CombinedTransformation = ParentTransformation * NodeTransformation;

    m_nodeMap[pNode->mName.C_Str()] = { pNode, &m_Meshes[0].TransformationDeprecated };//CombinedTransformation


#ifdef DEBUG_SCENE_HIERARCHY
    printf("Combined transformation:\n");
    CombinedTransformation.Print();
#endif
    
    if (pNode->mNumMeshes > 0) {
#ifdef DEBUG_SCENE_HIERARCHY
        printf("Num meshes: %d - ", pNode->mNumMeshes);
#endif
        for (int i = 0; i < (int)pNode->mNumMeshes; i++) {
            int MeshIndex = pNode->mMeshes[i];
#ifdef DEBUG_SCENE_HIERARCHY
            printf("%d ", MeshIndex);
#endif
            m_Meshes[MeshIndex].TransformationDeprecated = CombinedTransformation;
        }
#ifdef DEBUG_SCENE_HIERARCHY
        printf("\n");
#endif
    }
    else {
#ifdef DEBUG_SCENE_HIERARCHY
        printf("No meshes\n");
#endif
    }

    for (uint i = 0; i < pNode->mNumChildren; i++) {
        TraverseNodeHierarchy(CombinedTransformation, pNode->mChildren[i]);
    }
}


template<typename VertexType>
void CoreModel::InitSingleMesh(std::vector<VertexType>& Vertices, uint MeshIndex, const aiMesh* paiMesh) 
{
    printf("Mesh %d: %s\n", MeshIndex, paiMesh->mName.C_Str());

    // Update exact data tracking directly from current dynamic array size to prevent offset corruption
    m_Meshes[MeshIndex].BaseVertex = (uint)(Vertices.size());
    m_Meshes[MeshIndex].BaseIndex = (uint)(m_Indices.size());

    unsigned int AddedVertices = 0;
    unsigned int AddedIndices = 0;

    float minX = FLT_MAX, minY = FLT_MAX, minZ = FLT_MAX;
    float maxX = -FLT_MAX, maxY = -FLT_MAX, maxZ = -FLT_MAX;

    for (unsigned int i = 0; i < paiMesh->mNumVertices; i++) {
        VertexType v;
        const aiVector3D& Pos = paiMesh->mVertices[i];
        v.Position = Vector3f(Pos.x, Pos.y, Pos.z);

        minX = std::min(minX, v.Position.x);
        minY = std::min(minY, v.Position.y);
        minZ = std::min(minZ, v.Position.z);
        maxX = std::max(maxX, v.Position.x);
        maxY = std::max(maxY, v.Position.y);
        maxZ = std::max(maxZ, v.Position.z);

        m_minPos.x = std::min(m_minPos.x, v.Position.x);
        m_minPos.y = std::min(m_minPos.y, v.Position.y);
        m_minPos.z = std::min(m_minPos.z, v.Position.z);

        m_maxPos.x = std::max(m_maxPos.x, v.Position.x);
        m_maxPos.y = std::max(m_maxPos.y, v.Position.y);
        m_maxPos.z = std::max(m_maxPos.z, v.Position.z);

        if (paiMesh->mNormals) {
            v.Normal = Vector3f(paiMesh->mNormals[i].x, paiMesh->mNormals[i].y, paiMesh->mNormals[i].z);
        } else {
            v.Normal = Vector3f(0.0f, 1.0f, 0.0f);
        }

        v.TexCoords0 = paiMesh->HasTextureCoords(0) ? Vector2f(paiMesh->mTextureCoords[0][i].x, paiMesh->mTextureCoords[0][i].y) : Vector2f(0.0f);
        v.TexCoords1 = paiMesh->HasTextureCoords(1) ? Vector2f(paiMesh->mTextureCoords[1][i].x, paiMesh->mTextureCoords[1][i].y) : Vector2f(0.0f);

        if (m_flipTexCoordV) {
            v.TexCoords0.y = 1.0f - v.TexCoords0.y;
            v.TexCoords1.y = 1.0f - v.TexCoords1.y;
        }

        v.Tangent = paiMesh->mTangents ? Vector3f(paiMesh->mTangents[i].x, paiMesh->mTangents[i].y, paiMesh->mTangents[i].z) : Vector3f(0.0f);
        v.Bitangent = paiMesh->mBitangents ? Vector3f(paiMesh->mBitangents[i].x, paiMesh->mBitangents[i].y, paiMesh->mBitangents[i].z) : Vector3f(0.0f);

        if (paiMesh->mColors[0]) {
            const aiColor4D& Color = paiMesh->mColors[0][i];
            v.Color = Vector4f(Color.r, Color.g, Color.b, Color.a);
        } else {
            v.Color = Vector4f(1.0f);
        }

        //printf("Pos %d: ", i); v.Position.Print();
      //  printf("Normal: "); v.Normal.Print();
      //  printf("Tangent: "); v.Tangent.Print();
      //  printf("Bitangent: "); v.Bitangent.Print();
     //   printf("Color: "); v.Color.Print();

        Vertices.push_back(v);
        AddedVertices++;
    }

    // Populate the index buffer
    for (unsigned int i = 0; i < paiMesh->mNumFaces; i++) {
        const aiFace& Face = paiMesh->mFaces[i];
        //  printf("num indices %d\n", Face.mNumIndices);
        if (Face.mNumIndices != 3) {
            continue; // Safely skip non-triangle data without corrupting pre-calculated indices
        }
     /*   printf("%d: %d\n", i * 3, Face.mIndices[0]);
        printf("%d: %d\n", i * 3 + 1, Face.mIndices[1]);
        printf("%d: %d\n", i * 3 + 2, Face.mIndices[2]);*/
        m_Indices.push_back(Face.mIndices[0]);
        m_Indices.push_back(Face.mIndices[1]);
        m_Indices.push_back(Face.mIndices[2]);
        AddedIndices += 3;
    }

    printf("Min pos: "); Vector3f(minX, minY, minZ).Print();
    printf("Max pos: "); Vector3f(maxX, maxY, maxZ).Print();

    // Re-assign accurate post-parsing count figures
    m_Meshes[MeshIndex].NumVertices = AddedVertices;
    m_Meshes[MeshIndex].NumIndices = AddedIndices;

    if constexpr (std::is_same_v<VertexType, SkinnedVertex>) {
        LoadMeshBones(Vertices, MeshIndex, paiMesh);
    }
}


template<typename VertexType>
void CoreModel::InitSingleMeshOpt(std::vector<VertexType>& AllVertices, uint MeshIndex, const aiMesh* paiMesh) {
    const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

    std::vector<VertexType> Vertices(paiMesh->mNumVertices);
    for (unsigned int i = 0; i < paiMesh->mNumVertices; i++) {
        VertexType v;
        const aiVector3D& Pos = paiMesh->mVertices[i];

        v.Position = Vector3f(Pos.x, Pos.y, Pos.z);
        m_minPos.x = std::min(m_minPos.x, v.Position.x);
        m_minPos.y = std::min(m_minPos.y, v.Position.y);
        m_minPos.z = std::min(m_minPos.z, v.Position.z);
        m_maxPos.x = std::max(m_maxPos.x, v.Position.x);
        m_maxPos.y = std::max(m_maxPos.y, v.Position.y);
        m_maxPos.z = std::max(m_maxPos.z, v.Position.z);

        if (paiMesh->mNormals) {
            const aiVector3D& pNormal = paiMesh->mNormals[i];
            v.Normal = Vector3f(pNormal.x, pNormal.y, pNormal.z);
        } else {
            v.Normal = Vector3f(0.0f, 1.0f, 0.0f);
        }

        const aiVector3D& pTexCoord0 = paiMesh->HasTextureCoords(0) ? paiMesh->mTextureCoords[0][i] : Zero3D;
        v.TexCoords0 = Vector2f(pTexCoord0.x, pTexCoord0.y);

        const aiVector3D& pTexCoord1 = paiMesh->HasTextureCoords(1) ? paiMesh->mTextureCoords[1][i] : Zero3D;
        v.TexCoords1 = Vector2f(pTexCoord1.x, pTexCoord1.y);

        v.Tangent = paiMesh->mTangents ? Vector3f(paiMesh->mTangents[i].x, paiMesh->mTangents[i].y, paiMesh->mTangents[i].z) : Vector3f(0.0f);
        v.Bitangent = paiMesh->mBitangents ? Vector3f(paiMesh->mBitangents[i].x, paiMesh->mBitangents[i].y, paiMesh->mBitangents[i].z) : Vector3f(0.0f);

        if (paiMesh->mColors[0]) {
            const aiColor4D& Color = paiMesh->mColors[0][i];
            v.Color = Vector4f(Color.r, Color.g, Color.b, Color.a);
        } else {
            v.Color = Vector4f(1.0f);
        }

     /*   printf("Pos %d: ", i); v.Position.Print();
        printf("Normal: "); v.Normal.Print();
        printf("Tangent: "); v.Tangent.Print();
        printf("Bitangent: "); v.Bitangent.Print();*/
	
        Vertices[i] = v;
    }

    m_Meshes[MeshIndex].BaseVertex = (uint)AllVertices.size();
    m_Meshes[MeshIndex].BaseIndex = (uint)m_Indices.size();

    std::vector<uint> Indices;
    Indices.reserve(paiMesh->mNumFaces * 3);

    // Populate the index buffer
    for (unsigned int i = 0; i < paiMesh->mNumFaces; i++) {
        const aiFace& Face = paiMesh->mFaces[i];

        if (Face.mNumIndices != 3) {
            printf("Warning! face %d has %d indices\n", i, Face.mNumIndices);
            continue;
        }

        Indices.push_back(Face.mIndices[0]);
        Indices.push_back(Face.mIndices[1]);
        Indices.push_back(Face.mIndices[2]);
    }

    if constexpr (std::is_same_v<VertexType, SkinnedVertex>) {
        LoadMeshBones(Vertices, MeshIndex, paiMesh);
    }

    OptimizeMesh(MeshIndex, Indices, Vertices, AllVertices);
}


template<typename VertexType>
void CoreModel::OptimizeMesh(int MeshIndex, std::vector<uint>& Indices, std::vector<VertexType>& Vertices, std::vector<VertexType>& AllVertices) 
{
    size_t NumIndices = Indices.size();
    size_t NumVertices = Vertices.size();
    if (NumIndices == 0 || NumVertices == 0) return;

    // Create a remap table
    std::vector<unsigned int> remap(NumIndices);
    size_t OptVertexCount = meshopt_generateVertexRemap(remap.data(),    // dst addr
                                                        Indices.data(),  // src indices
                                                        NumIndices,      // ...and size
                                                        Vertices.data(), // src vertices
                                                        NumVertices,     // ...and size
                                                        sizeof(VertexType)); // stride
    // Allocate a local index/vertex arrays
    std::vector<uint> OptIndices(NumIndices);
    std::vector<VertexType> OptVertices(OptVertexCount);

    // Optimization #1: remove duplicate vertices    
    meshopt_remapIndexBuffer(OptIndices.data(), Indices.data(), NumIndices, remap.data());

    meshopt_remapVertexBuffer(OptVertices.data(), Vertices.data(), NumVertices, sizeof(VertexType), remap.data());

    // Optimization #2: improve the locality of the vertices
    meshopt_optimizeVertexCache(OptIndices.data(), OptIndices.data(), NumIndices, OptVertexCount);

    // Optimization #3: reduce pixel overdraw
    meshopt_optimizeOverdraw(OptIndices.data(), OptIndices.data(), NumIndices, &(OptVertices[0].Position.x), OptVertexCount, sizeof(VertexType), 1.05f);

    // Optimization #4: optimize access to the vertex buffer
    meshopt_optimizeVertexFetch(OptVertices.data(), OptIndices.data(), NumIndices, OptVertices.data(), OptVertexCount, sizeof(VertexType));

    m_Meshes[MeshIndex].NumVertices = (uint)OptVertexCount;

    float Threshold = 1.0f;
    if (Threshold < 1.0f) { // Only run simplification code if reduction threshold is requested
        size_t TargetIndexCount = (size_t)(NumIndices * Threshold);
        float TargetError = 0.01f;
        std::vector<unsigned int> SimplifiedIndices(OptIndices.size());
        size_t OptIndexCount = meshopt_simplify(SimplifiedIndices.data(), OptIndices.data(), NumIndices, &OptVertices[0].Position.x, OptVertexCount, sizeof(VertexType), TargetIndexCount, TargetError);
        SimplifiedIndices.resize(OptIndexCount);

        m_Indices.insert(m_Indices.end(), SimplifiedIndices.begin(), SimplifiedIndices.end());
        m_Meshes[MeshIndex].NumIndices = (uint)OptIndexCount;
    } else {
        m_Indices.insert(m_Indices.end(), OptIndices.begin(), OptIndices.end());
        m_Meshes[MeshIndex].NumIndices = (uint)NumIndices;
    }

    AllVertices.insert(AllVertices.end(), OptVertices.begin(), OptVertices.end());
}


bool CoreModel::InitMaterials(const aiScene* pScene, const std::string& Filename)
{
    string Dir = GetDirFromFilename(Filename);

    bool Ret = true;

    printf("Num materials: %d\n", pScene->mNumMaterials);

    // Initialize the materials
    for (unsigned int i = 0 ; i < pScene->mNumMaterials; i++) {
        const aiMaterial* pMaterial = pScene->mMaterials[i];

#ifdef DEBUG_MATERIALS
        printf("\nLoading material %d: '%s'\n", i, pMaterial->GetName().C_Str());
#endif
        SetMaterialType(pMaterial, m_Materials[i]);

        LoadTextures(Dir, pMaterial, i);

        LoadColors(pMaterial, i);
    }

    return Ret;
}


const CoreMaterial* CoreModel::GetMaterialForMesh(int MeshIndex) const
{
    if (MeshIndex >= m_Meshes.size()) {
        printf("Invalid mesh index %d, num meshes %d\n", MeshIndex, (int)m_Meshes.size());
        exit(1);
    }

    int MaterialIndex = m_Meshes[MeshIndex].MaterialIndex;

    const CoreMaterial* pMaterial = NULL;

    if ((MaterialIndex >= 0) && (MaterialIndex < m_Materials.size())) {
        pMaterial = & m_Materials[MaterialIndex];
    }

    return pMaterial;
}


void CoreModel::GetVertexSizesInBytes(size_t& VertexSize, size_t& SkinnedVertexSize)
{
    VertexSize = sizeof(Vertex);
    SkinnedVertexSize = sizeof(SkinnedVertex);
}


static int GetTextureCount(const aiMaterial* pMaterial)
{
    int TextureCount = 0;

    for (int i = 0; i <= AI_TEXTURE_TYPE_MAX; ++i) { // UNKNOWN is the last texture type in Assimp.
        aiTextureType ttype = (aiTextureType)(i);
        int Count = pMaterial->GetTextureCount(ttype);
        TextureCount += Count;

#ifdef DEBUG_MATERIALS
        if (Count > 0) {
#ifdef _WIN64
            printf("Found texture %s\n", aiTextureTypeToString(ttype));
#endif
        }
#endif
    }

    return TextureCount;
}


void CoreModel::LoadTextures(const std::string& Dir, const aiMaterial* pMaterial, int index)
{
    int TextureCount = GetTextureCount(pMaterial);

#ifdef DEBUG_MATERIALS
    printf("Number of textures %d\n", TextureCount);
#endif

    LoadDiffuseTexture(Dir, pMaterial, index);
    LoadSpecularTexture(Dir, pMaterial, index);
    LoadNormalTexture(Dir, pMaterial, index);
    LoadMetalnessTexture(Dir, pMaterial, index);
    LoadEmissiveTexture(Dir, pMaterial, index);
    LoadEmissionColorTexture(Dir, pMaterial, index);
    LoadNormalCameraTexture(Dir, pMaterial, index);
    LoadRoughnessTexture(Dir, pMaterial, index);
    LoadAmbientOcclusionTexture(Dir, pMaterial, index);
    LoadClearCoatTexture(Dir, pMaterial, index);
    LoadClearCoatRoughnessTexture(Dir, pMaterial, index);
    LoadClearCoatNormalTexture(Dir, pMaterial, index);
}


void CoreModel::LoadDiffuseTexture(const std::string& Dir, const aiMaterial* pMaterial, int MaterialIndex)
{
    if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
        LoadTexture(Dir, pMaterial, MaterialIndex, aiTextureType_DIFFUSE, 0, TEX_TYPE_BASE);
    } else {
        printf("Warning! no diffuse texture\n");

        aiString Path;

        if (MissingTextureDetection) {
            Path = ("no_texture.png");  // Big red texture with the text "no texture" to make it obvious when a texture is missing
        } else {
            Path = ("white.png");   // Single pixel texture to minimize the performance impact of missing textures
        }
        
#ifdef OGLDEV_VULKAN
        std::string DefaultTextureDir("../../Content/textures/");
#else
        std::string DefaultTextureDir("../Content/textures/");
#endif

        LoadTextureFromFile(DefaultTextureDir, Path, MaterialIndex, TEX_TYPE_BASE, false);
    }
}


void CoreModel::LoadSpecularTexture(const std::string& Dir, const aiMaterial* pMaterial, int MaterialIndex)
{
    LoadTexture(Dir, pMaterial, MaterialIndex, aiTextureType_SHININESS, 0, TEX_TYPE_SPECULAR);
}


void CoreModel::LoadNormalTexture(const std::string& Dir, const aiMaterial* pMaterial, int MaterialIndex)
{
    LoadTexture(Dir, pMaterial, MaterialIndex, aiTextureType_NORMALS, 0, TEX_TYPE_NORMAL);
}


void CoreModel::LoadMetalnessTexture(const std::string& Dir, const aiMaterial* pMaterial, int MaterialIndex)
{
    LoadTexture(Dir, pMaterial, MaterialIndex, aiTextureType_METALNESS, 0, TEX_TYPE_METALNESS);
}

void CoreModel::LoadEmissiveTexture(const std::string& Dir, const aiMaterial* pMaterial, int MaterialIndex)
{
    LoadTexture(Dir, pMaterial, MaterialIndex, aiTextureType_EMISSIVE, 0, TEX_TYPE_EMISSIVE);
}

void CoreModel::LoadEmissionColorTexture(const std::string& Dir, const aiMaterial* pMaterial, int MaterialIndex)
{
    LoadTexture(Dir, pMaterial, MaterialIndex, aiTextureType_EMISSION_COLOR, 0, TEX_TYPE_EMISSION_COLOR);
}

void CoreModel::LoadNormalCameraTexture(const std::string& Dir, const aiMaterial* pMaterial, int MaterialIndex)
{
    LoadTexture(Dir, pMaterial, MaterialIndex, aiTextureType_NORMAL_CAMERA, 0, TEX_TYPE_NORMAL_CAMERA);
}

void CoreModel::LoadRoughnessTexture(const std::string& Dir, const aiMaterial* pMaterial, int MaterialIndex)
{
    LoadTexture(Dir, pMaterial, MaterialIndex, aiTextureType_DIFFUSE_ROUGHNESS, 0, TEX_TYPE_ROUGHNESS);
}

void CoreModel::LoadAmbientOcclusionTexture(const std::string& Dir, const aiMaterial* pMaterial, int MaterialIndex)
{
    LoadTexture(Dir, pMaterial, MaterialIndex, aiTextureType_UNKNOWN, 0, TEX_TYPE_AMBIENT_OCCLUSION);
}


void CoreModel::LoadClearCoatTexture(const std::string& Dir, const aiMaterial* pMaterial, int MaterialIndex)
{
    LoadTexture(Dir, pMaterial, MaterialIndex, aiTextureType_CLEARCOAT, 0, TEX_TYPE_CLEARCOAT);
}


void CoreModel::LoadClearCoatRoughnessTexture(const std::string& Dir, const aiMaterial* pMaterial, int MaterialIndex)
{
    LoadTexture(Dir, pMaterial, MaterialIndex, aiTextureType_CLEARCOAT, 0, TEX_TYPE_CLEARCOAT_ROUGHNESS);
}


void CoreModel::LoadClearCoatNormalTexture(const std::string& Dir, const aiMaterial* pMaterial, int MaterialIndex)
{
    LoadTexture(Dir, pMaterial, MaterialIndex, aiTextureType_CLEARCOAT, 0, TEX_TYPE_CLEARCOAT_NORMAL);
}


void CoreModel::LoadTexture(const std::string& Dir, const aiMaterial* pMaterial, int MaterialIndex, 
                            aiTextureType AssimpType, int AssimpTexIndex, TEXTURE_TYPE MyType) 
{
    assert(MyType < TEX_TYPE_NUM);

    m_Materials[MaterialIndex].pTextures[MyType] = NULL;

    if (pMaterial->GetTextureCount(AssimpType) > 0) {
        aiString Path;

        if (pMaterial->GetTexture(AssimpType, AssimpTexIndex, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
            const aiTexture* paiTexture = m_pScene->GetEmbeddedTexture(Path.C_Str());

            bool IsSRGB = (MyType == TEX_TYPE_BASE);

            if (paiTexture) {
                LoadTextureEmbedded(paiTexture, MaterialIndex, MyType, IsSRGB);
            } else {
                LoadTextureFromFile(Dir, Path, MaterialIndex, MyType, IsSRGB);
            }
        }
    }
}


void CoreModel::LoadTextureEmbedded(const aiTexture* paiTexture, int MaterialIndex, TEXTURE_TYPE MyType, bool IsSRGB) 
{
#ifdef DEBUG_MATERIALS
    printf("Loaded embedded texture type '%s'\n", paiTexture->achFormatHint);
#endif
    // Raw assignment compiles successfully again
    m_Materials[MaterialIndex].pTextures[MyType] = AllocTexture2D();
    assert(paiTexture->mHeight == 0);
    int buffer_size = paiTexture->mWidth;   
    m_Materials[MaterialIndex].pTextures[MyType]->Load(buffer_size, paiTexture->pcData, IsSRGB);
}


void CoreModel::LoadTextureFromFile(const std::string& Dir, const aiString& Path, int MaterialIndex, TEXTURE_TYPE MyType, bool IsSRGB) 
{
    std::string FullPath = GetFullPath(Dir, Path);

    m_Materials[MaterialIndex].pTextures[MyType] = AllocTexture2D();
    m_Materials[MaterialIndex].pTextures[MyType]->Load(FullPath.c_str(), IsSRGB);

#ifdef DEBUG_MATERIALS
    printf("Loaded texture type %d from '%s'\n", MyType, FullPath.c_str());
#endif
}


void CoreModel::LoadColors(const aiMaterial* pMaterial, int index)
{
    CoreMaterial& material = m_Materials[index];

    material.m_name = pMaterial->GetName().C_Str();

    Vector4f AllOnes(1.0f, 1.0f, 1.0f, 1.0f);

    LoadColor(pMaterial, material.AmbientColor, AI_MATKEY_COLOR_AMBIENT, "ambient color");

    aiColor4D EmissiveColor(0.0f, 0.0f, 0.0f, 0.0f);

    if (pMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, EmissiveColor) == AI_SUCCESS) {
#ifdef DEBUG_COLORS
        printf("Loaded emissive color [%f %f %f]\n", EmissiveColor.r, EmissiveColor.g, EmissiveColor.b);
#endif
        material.AmbientColor.r += EmissiveColor.r;
        material.AmbientColor.g += EmissiveColor.g;
        material.AmbientColor.b += EmissiveColor.b;
        material.AmbientColor.a += EmissiveColor.a;
        material.AmbientColor.a = std::min(material.AmbientColor.a, 1.0f);
    } else {
        material.AmbientColor = AllOnes;
    }

    LoadColor(pMaterial, material.DiffuseColor, AI_MATKEY_COLOR_DIFFUSE, "diffuse color");

    LoadColor(pMaterial, material.SpecularColor, AI_MATKEY_COLOR_SPECULAR, "specular color");

    LoadColor(pMaterial, material.BaseColor, AI_MATKEY_BASE_COLOR, "base color");

    LoadColor(pMaterial, material.EmissiveColor, AI_MATKEY_COLOR_EMISSIVE, "emissive color");

    float EmissiveStrength = 1.0f;
    if (pMaterial->Get(AI_MATKEY_EMISSIVE_INTENSITY, EmissiveStrength) == AI_SUCCESS) {
        material.EmissiveColor = material.EmissiveColor * Vector4f(EmissiveStrength, EmissiveStrength, EmissiveStrength, 1.0f);
    }

    if (material.m_materialType & MaterialType_MetallicRoughness) {
        float MetallicFactor;

        if (pMaterial->Get(AI_MATKEY_METALLIC_FACTOR, MetallicFactor) == AI_SUCCESS) {
            material.MetallicRoughnessNormalOcclusion.x = MetallicFactor;
        }

        float RoughnessFactor;

        if (pMaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, RoughnessFactor) == AI_SUCCESS) {
            material.MetallicRoughnessNormalOcclusion.y = RoughnessFactor;
        }
    }

    float NormalScale;
    if (pMaterial->Get(AI_MATKEY_GLTF_TEXTURE_SCALE(aiTextureType_NORMALS, 0), NormalScale) == AI_SUCCESS) {
        material.MetallicRoughnessNormalOcclusion.z = NormalScale;
    }

    float OcclusionStrength;
    if (pMaterial->Get(AI_MATKEY_GLTF_TEXTURE_SCALE(aiTextureType_LIGHTMAP, 0), OcclusionStrength) == AI_SUCCESS) {
        material.MetallicRoughnessNormalOcclusion.w = OcclusionStrength;
    }

    float AlphaCutoff = 0.0f;

    if (pMaterial->Get(AI_MATKEY_GLTF_ALPHACUTOFF, AlphaCutoff) == AI_SUCCESS) {
        material.EmissiveColor.a = AlphaCutoff;
    } else {
        material.EmissiveColor.a = 0.5f;    // Based on the "3D Rendering Cookbook"
    }

    float OpaquenessThreshold = 0.05f;
    float Opacity = 1.0f;
    if (pMaterial->Get(AI_MATKEY_OPACITY, Opacity) == AI_SUCCESS) {
        material.m_transparencyFactor = std::clamp(1.0f - Opacity, 0.0f, 1.0f);
        if (material.m_transparencyFactor >= 1.0f - OpaquenessThreshold) {
            material.m_transparencyFactor = 0.0f;
        }
    }

    aiColor4D TransparentColor;
    if (pMaterial->Get(AI_MATKEY_COLOR_TRANSPARENT, TransparentColor) == AI_SUCCESS) {
        float Opacity = std::max(std::max(TransparentColor.r, TransparentColor.g), TransparentColor.b);
        material.m_transparencyFactor = std::clamp(Opacity, 0.0f, 1.0f);
        if (material.m_transparencyFactor >= 1.0f - OpaquenessThreshold) {
            material.m_transparencyFactor = 0.0f;
        }

        material.m_alphaTest = 0.5f;
    }

    ProcessClearCoat(index, pMaterial, material);

    if ((material.AmbientColor.r == 0.0f) && (material.AmbientColor.g == 0.0f) && (material.AmbientColor.b == 0.0f)) {
        // THE MIXAMO FALLBACK: If the DAE file provides a zero ambient vector,
        // match the diffuse color property so the ambient light works.
        material.AmbientColor = material.DiffuseColor;
    }
}


void CoreModel::ProcessClearCoat(int index, const aiMaterial* pMaterial, CoreMaterial& material)
{
    bool UseClearCoat = m_Materials[index].pTextures[TEX_TYPE_CLEARCOAT] ||
                        m_Materials[index].pTextures[TEX_TYPE_CLEARCOAT_NORMAL] ||
                        m_Materials[index].pTextures[TEX_TYPE_CLEARCOAT_ROUGHNESS];

    float ClearCoatFactor = 0.0f;
    if (pMaterial->Get(AI_MATKEY_CLEARCOAT_FACTOR, ClearCoatFactor) == AI_SUCCESS) {
        material.ClearCoatTransmissionThickness.x = ClearCoatFactor;
        UseClearCoat = true;
    }

    float ClearCoatRoughnessFactor = 0.0f;
    if (pMaterial->Get(AI_MATKEY_CLEARCOAT_ROUGHNESS_FACTOR, ClearCoatRoughnessFactor) == AI_SUCCESS) {
        material.ClearCoatTransmissionThickness.y = ClearCoatRoughnessFactor;
        UseClearCoat = true;
    }

    if (UseClearCoat) {
        material.m_materialType |= MaterialType_ClearCoat;
    }
}


void CoreModel::LoadColor(const aiMaterial* pMaterial, Vector4f& Color, 
                          const char* pAiMatKey, int AiMatType, int AiMatIdx, const char* pName)
{
    aiColor4D AiColor(0.0f, 0.0f, 0.0f, 0.0f);

    if (pMaterial->Get(pAiMatKey, AiMatType, AiMatIdx, AiColor) == AI_SUCCESS) {
#ifdef DEBUG_COLORS
        printf("Loaded %s color [%f %f %f]\n", pName, AiColor.r, AiColor.g, AiColor.b);
#endif
        Color.r = AiColor.r;
        Color.g = AiColor.g;
        Color.b = AiColor.b;
        Color.a = std::min(AiColor.a, 1.0f);
    } else {
        Color = Vector4f(1.0f);
    }
}


static void traverse(int depth, aiNode* pNode)
{
#ifdef DEBUG_SCENE_HIERARCHY
    for (int i = 0; i < depth; i++) {
        printf(" ");
    }

    printf("%s\n", pNode->mName.C_Str());
#endif

#ifdef DEBUG_SCENE_HIERARCHY
    Matrix4f NodeTransformation(pNode->mTransformation);
    NodeTransformation.Print(); 
#endif

    for (uint i = 0; i < pNode->mNumChildren; i++) {
        traverse(depth + 1, pNode->mChildren[i]);
    }
}


void CoreModel::InitCameras(const aiScene* pScene)
{
    printf("\n*** Initializing cameras ***\n");
    printf("Loading %d cameras\n", pScene->mNumCameras);

    m_cameras.resize(pScene->mNumCameras);

    for (unsigned int i = 0; i < pScene->mNumCameras; i++) {
        InitSingleCamera(i, pScene);
    }
}


void CoreModel::InitSingleCamera(int Index, const aiScene* pScene)
{
    const aiCamera* pCamera = pScene->mCameras[Index];
    printf("Loading Camera: '%s'\n", pCamera->mName.C_Str());

    // 1. Get the hierarchy matrix (Order: Parent * Child)
    Matrix4f Transformation;
    GetFullTransformation(pScene->mRootNode, pCamera->mName.C_Str(), Transformation);

    aiMatrix4x4 aiCameraMatrix;
    pCamera->GetCameraMatrix(aiCameraMatrix);
    Matrix4f CameraMatrix(aiCameraMatrix);
    printf("Camera internal transformation:\n");
    CameraMatrix.Print();

    Matrix4f FinalWorld = Transformation;
    
    printf("Final camera transformation:\n");
    FinalWorld.Print();

    // 3. Extract World Position
    Vector4f Pos4D(0.0f, 0.0f, 0.0f, 1.0f);
    Pos4D = FinalWorld * Pos4D;
    Vector3f FinalPos(pCamera->mPosition.x, pCamera->mPosition.y, -pCamera->mPosition.z);

    // 4. Extract World Direction (Blender Camera Forward is -Z)
    Vector4f Target4D(0.0f, 0.0f, -1.0f, 0.0f);
    Target4D = FinalWorld * Target4D;
    //Vector3f FinalTargetDir = Vector3f(pCamera->mLookAt.x, pCamera->mLookAt.y, -pCamera->mLookAt.z).Normalize();
    Vector3f FinalTargetDir(Transformation.m[0][2], -Transformation.m[1][2], Transformation.m[2][2]);

    FinalTargetDir = FinalTargetDir.Normalize();

    // 5. Extract World Up (Blender Camera Up is +Y)
    Vector4f Up4D(0.0f, 1.0f, 0.0f, 0.0f);
    Up4D = FinalWorld * Up4D;
    Vector3f FinalUp = Vector3f(Up4D.x, Up4D.y, Up4D.z).Normalize();

    // 6. Setup Perspective Projection
    PersProjInfo persProjInfo;
    persProjInfo.zNear = pCamera->mClipPlaneNear;
    persProjInfo.zFar = pCamera->mClipPlaneFar;

    int WindowWidth, WindowHeight;
#ifdef OGLDEV_VULKAN
    GetWindowSize(WindowWidth, WindowHeight);
#else
    m_pCoreRenderingSystem->GetWindowSize(WindowWidth, WindowHeight);
#endif
    persProjInfo.Width = (float)WindowWidth;
    persProjInfo.Height = (float)WindowHeight;

    float FullHorizontalDeg = ToDegree(pCamera->mHorizontalFOV);

    float AspectRatio = persProjInfo.Width / persProjInfo.Height;

    // 3. Mathematical conversion: Full Horizontal FOV -> Full Vertical FOV
    float FullVerticalRad = 2.0f * atan(tan(ToRadian(FullHorizontalDeg) / 2.0f) / AspectRatio);
    float FullVerticalDeg = ToDegree(FullVerticalRad);

    // 4. Assign the FULL vertical degree angle straight to your projection settings
    // (Do NOT divide by 2! glm::perspective handles halving internally via its cotangent math)
    persProjInfo.FOV = FullVerticalDeg;

    m_cameras[Index].Init(FinalPos.ToGLM(), FinalTargetDir.ToGLM(), FinalUp.ToGLM(), persProjInfo);

    printf("Final Pos: "); FinalPos.Print();
    printf("Final Dir: "); FinalTargetDir.Print();
    printf("Final Up: "); FinalUp.Print();
}


void CoreModel::InitLights(const aiScene* pScene)
{   
    printf("\n*** Initializing lights ***\n");

    for (int i = 0; i < (int)pScene->mNumLights; i++) {
        InitSingleLight(pScene, *pScene->mLights[i]);
    }
}


void CoreModel::InitSingleLight(const aiScene* pScene, const aiLight& light)
{
    printf("Init light '%s'\n", light.mName.C_Str());

    switch (light.mType) {

    case aiLightSource_DIRECTIONAL:
        InitDirectionalLight(pScene, light);
        break;

    case aiLightSource_POINT:
        InitPointLight(pScene, light);
        break;

    case aiLightSource_SPOT:
        InitSpotLight(pScene, light);
        break;

    case aiLightSource_AMBIENT:
        printf("Ambient light is not implemented\n");
        exit(0);

    case aiLightSource_AREA:
        printf("Area light is not implemented\n");
        exit(0);

    case aiLightSource_UNDEFINED:
    default:
        printf("Light type is undefined\n");
        exit(0);
    }
}


static bool GetFullTransformation(const aiNode* pRootNode, const char* pName, Matrix4f& Transformation)
{
    Transformation.InitIdentity();

    const aiNode* pNode = pRootNode->FindNode(pName);
    
    if (!pNode) {
        printf("Warning! Cannot find a node for '%s'\n", pName);
        return false;
    }    

    while (pNode) {
        Matrix4f CurTransformation(pNode->mTransformation);
        Transformation = CurTransformation * Transformation;
        pNode = pNode->mParent;
    }

    return true;
}


void CoreModel::InitDirectionalLight(const aiScene* pScene, const aiLight& light) 
{
    DirectionalLight l;
    //l.Color = Vector3f(light.mColorDiffuse.r, light.mColorDiffuse.g, light.mColorDiffuse.b);
    l.Color = Vector3f(1.0f);
    l.DiffuseIntensity = 1.0f; // TODO

    Matrix4f Transformation;
    GetFullTransformation(pScene->mRootNode, light.mName.C_Str(), Transformation);

   /* Vector3f Direction = VectorFromAssimpVector(light.mDirection);
    printf("Original direction: "); Direction.Print();
    Vector4f Dir4D(Direction, 0.0f);
    Dir4D = Transformation * Dir4D;
    l.WorldDirection = Dir4D;
    printf("Transformed direction: "); l.WorldDirection.Print();*/

    // Extract the components directly from your row-major matrix layout
    // to bypass Blender's embedded glTF offset rotation
    Vector3f TrueDirection;
    TrueDirection.x = Transformation.m[0][2];
    TrueDirection.y = Transformation.m[1][2];
    TrueDirection.z = Transformation.m[2][2];

    // Normalize to preserve clean calculations for your lighting equations
    l.WorldDirection = TrueDirection.Normalize();

    printf("Adjusted World Direction: "); l.WorldDirection.Print();

    Vector3f Up = VectorFromAssimpVector(light.mUp);
    printf("Original up: "); Up.Print();
    Vector4f Up4D(Up, 0.0f);
    Up4D = Transformation * Up4D;

    l.Up = Vector3f(Up4D);
    printf("Final up: "); l.Up.Print();

    m_dirLights.push_back(l);
}


void CoreModel::InitPointLight(const aiScene* pScene, const aiLight& light) 
{
    PointLight l;
    l.Color = Vector3f(light.mColorDiffuse.r, light.mColorDiffuse.g, light.mColorDiffuse.b);
    printf("Point light color [%f %f %f]\n", l.Color.x, l.Color.y, l.Color.z);
    //l.Color = Vector3f(1.0f);
    l.DiffuseIntensity = 1.0f; // TODO

    Vector3f Position = VectorFromAssimpVector(light.mPosition);
    printf("Original Position: "); Position.Print();

    Matrix4f Transformation;

    GetFullTransformation(pScene->mRootNode, light.mName.C_Str(), Transformation);

    Vector4f Pos4D(Position, 1.0f);
    Pos4D = Transformation * Pos4D;

    l.WorldPosition = Vector3f(Pos4D);
    printf("Final Position: "); l.WorldPosition.Print();

    l.Attenuation.Constant = light.mAttenuationConstant;
    l.Attenuation.Linear = light.mAttenuationLinear;
    l.Attenuation.Exp = light.mAttenuationQuadratic;

    printf("Attenuation: constant %f linear %f exp %f\n", l.Attenuation.Constant, l.Attenuation.Linear, l.Attenuation.Exp);

    m_pointLights.push_back(l);
}


void CoreModel::InitSpotLight(const aiScene* pScene, const aiLight& light) 
{
    SpotLight l;
    //l.Color = Vector3f(light.mColorDiffuse.r, light.mColorDiffuse.g, light.mColorDiffuse.b);
    l.Color = Vector3f(1.0f);
    l.DiffuseIntensity = 1.0f; // TODO

    Matrix4f Transformation;
    GetFullTransformation(pScene->mRootNode, light.mName.C_Str(), Transformation);

/*    Vector3f Direction = VectorFromAssimpVector(light.mDirection);
    printf("Original direction: "); Direction.Print();
    Vector4f Dir4D(Direction, 0.0f);
    Dir4D = Transformation * Dir4D;
    l.WorldDirection = Dir4D;
    printf("Final direction: "); l.WorldDirection.Print();*/

    Vector3f TrueDirection;
    TrueDirection.x = Transformation.m[0][2];
    TrueDirection.y = Transformation.m[1][2];
    TrueDirection.z = Transformation.m[2][2];

    // Normalize to preserve clean calculations for your lighting equations
    l.WorldDirection = TrueDirection.Normalize();

    Vector3f Up = VectorFromAssimpVector(light.mUp);
    printf("Original up: "); Up.Print();
    if (Up.Length() == 0) {
        // Use clean 3D vector literal dimensions for structural comparisons
        if ((l.WorldDirection == Vector3f(0.0f, 1.0f, 0.0f)) || (l.WorldDirection == Vector3f(0.0f, -1.0f, 0.0f))) {
            l.Up = Vector3f(1.0f, 0.0f, 0.0f);
        } else {
            l.Up = Vector3f(0.0f, 1.0f, 0.0f);
        }
    } else {
        Vector4f Up4D(Up, 0.0f);
        Up4D = Transformation * Up4D;
        l.Up = Vector3f(Up4D);
    }

    printf("Final up: "); l.Up.Print();

    Vector3f Position = VectorFromAssimpVector(light.mPosition);
    printf("Original Position: "); Position.Print();
    Vector4f Pos4D(Position, 1.0f);
    Pos4D = Transformation * Pos4D;

    l.WorldPosition = Vector3f(Pos4D);
    printf("Final Position: "); l.WorldPosition.Print();

    l.Attenuation.Constant = light.mAttenuationConstant;
    l.Attenuation.Linear = light.mAttenuationLinear;
    l.Attenuation.Exp = light.mAttenuationQuadratic / 100.0f;

    printf("Attenuation: constant %f linear %f exp (adjusted!) %f\n", l.Attenuation.Constant, l.Attenuation.Linear, l.Attenuation.Exp);

    if (light.mAngleInnerCone != light.mAngleOuterCone) {
        printf("Warning!!! Different values for spot light inner/outer cone angles is not supported\n");
    }

    l.Cutoff = ToDegree(light.mAngleOuterCone);

    printf("Cutoff angle %f\n", l.Cutoff);

    m_spotLights.push_back(l);
}


Matrix4f& CoreModel::GetNodeTransformation(const char* pNodeName)
{
    std::map<std::string, HierarchyNodeInfo>::iterator it = m_nodeMap.find(pNodeName);

    if (it == m_nodeMap.end()) {
        OGLDEV_ERROR("Cannot find node %s in the hierarchy\n", pNodeName);
        assert(0);
    }

    return *(it->second.GlobalTransform);
}


void CoreModel::LoadMeshBones(std::vector<SkinnedVertex>& SkinnedVertices, uint MeshIndex, const aiMesh* pMesh)
{
    if (pMesh->mNumBones > MAX_BONES) {
        printf("The number of bones in the model (%d) is larger than the maximum supported (%d)\n", pMesh->mNumBones, MAX_BONES);
        printf("Make sure to increase the macro MAX_BONES in the C++ header as well as in the shader to the same value\n");
        assert(0);
    }

    // printf("Loading mesh bones %d\n", MeshIndex);
    for (uint i = 0 ; i < pMesh->mNumBones ; i++) {
        // printf("Bone %d %s\n", i, pMesh->mBones[i]->mName.C_Str());
        LoadSingleBone(SkinnedVertices, MeshIndex, pMesh->mBones[i]);
    }
}


void CoreModel::LoadSingleBone(std::vector<SkinnedVertex>& SkinnedVertices, uint MeshIndex, const aiBone* pBone) 
{
    int BoneId = GetBoneId(pBone);

    if (BoneId == m_BoneInfo.size()) {
        BoneInfo bi(pBone->mOffsetMatrix);
        // bi.OffsetMatrix.Print();
        m_BoneInfo.push_back(bi);
    }

    for (uint i = 0; i < pBone->mNumWeights; i++) {
        const aiVertexWeight& vw = pBone->mWeights[i];

        // Fix: Use the isolated mesh layout offset safely.
        // If optimizing with meshopt, ensure this logic runs BEFORE meshopt_remapVertexBuffer maps the array!
        uint GlobalVertexID = m_Meshes[MeshIndex].BaseVertex + vw.mVertexId;

        if (GlobalVertexID < SkinnedVertices.size()) {
            SkinnedVertices[GlobalVertexID].Bones.AddBoneData(BoneId, vw.mWeight);
        }
    }
    MarkRequiredNodesForBone(pBone);
}


void CoreModel::MarkRequiredNodesForBone(const aiBone* pBone) 
{
    std::string NodeName(pBone->mName.C_Str());

    const aiNode* pParent = NULL;

    do {
        auto it = m_requiredNodeMap.find(NodeName);
        if (it == m_requiredNodeMap.end()) {
            printf("Cannot find bone %s in the hierarchy\n", NodeName.c_str());
            assert(0);
        }

        it->second.isRequired = true;

        pParent = it->second.pNode->mParent;

        if (pParent) {
            NodeName = std::string(pParent->mName.C_Str());
        }

    } while (pParent);
}


void CoreModel::InitializeRequiredNodeMap(const aiNode* pNode)
{
    std::string NodeName(pNode->mName.C_Str());

    NodeInfo info(pNode);

    m_requiredNodeMap[NodeName] = info;

    for (unsigned int i = 0 ; i < pNode->mNumChildren ; i++) {
        InitializeRequiredNodeMap(pNode->mChildren[i]);
    }
}


int CoreModel::GetBoneId(const aiBone* pBone)
{
    int BoneIndex = 0;
    std::string BoneName(pBone->mName.C_Str());

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


uint CoreModel::FindPosition(float AnimationTimeTicks, const aiNodeAnim* pNodeAnim)
{
    for (uint i = 0 ; i < pNodeAnim->mNumPositionKeys - 1 ; i++) {
        float t = (float)pNodeAnim->mPositionKeys[i + 1].mTime;
        if (AnimationTimeTicks < t) {
            return i;
        }
    }

    return 0;
}


void CoreModel::CalcInterpolatedPosition(aiVector3D& Out, float AnimationTimeTicks, const aiNodeAnim* pNodeAnim) 
{
    // we need at least two values to interpolate...
    if (pNodeAnim->mNumPositionKeys == 1) {
        Out = pNodeAnim->mPositionKeys[0].mValue;
        return;
    }

    float MaxTime = (float)pNodeAnim->mPositionKeys[pNodeAnim->mNumPositionKeys - 1].mTime;
    if (AnimationTimeTicks >= MaxTime) {
        Out = pNodeAnim->mPositionKeys[pNodeAnim->mNumPositionKeys - 1].mValue;
        return;
    }

    uint PositionIndex = FindPosition(AnimationTimeTicks, pNodeAnim);
    uint NextPositionIndex = PositionIndex + 1;
    assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);

    float t1 = (float)pNodeAnim->mPositionKeys[PositionIndex].mTime;
    float t2 = (float)pNodeAnim->mPositionKeys[NextPositionIndex].mTime;

    // Fix: Stripped redundant unreachable 'if (t1 > AnimationTimeTicks)' check and safe-guarded division
    float DeltaTime = t2 - t1;
    float Factor = 0.0f;
    if (DeltaTime > 0.0f) {
        Factor = (AnimationTimeTicks - t1) / DeltaTime;
    }

    Factor = std::clamp(Factor, 0.0f, 1.0f);

    const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
    const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
    Out = Start + Factor * (End - Start);
}


uint CoreModel::FindRotation(float AnimationTimeTicks, const aiNodeAnim* pNodeAnim)
{
    assert(pNodeAnim->mNumRotationKeys > 0);

    for (uint i = 0 ; i < pNodeAnim->mNumRotationKeys - 1 ; i++) {
        float t = (float)pNodeAnim->mRotationKeys[i + 1].mTime;
        if (AnimationTimeTicks < t) {
            return i;
        }
    }

    return 0;
}


void CoreModel::CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTimeTicks, const aiNodeAnim* pNodeAnim) 
{
    // we need at least two values to interpolate...
    if (pNodeAnim->mNumRotationKeys == 1) {
        Out = pNodeAnim->mRotationKeys[0].mValue;
        return;
    }

    float MaxTime = (float)pNodeAnim->mRotationKeys[pNodeAnim->mNumRotationKeys - 1].mTime;
    if (AnimationTimeTicks >= MaxTime) {
        Out = pNodeAnim->mRotationKeys[pNodeAnim->mNumRotationKeys - 1].mValue;
        return;
    }

    uint RotationIndex = FindRotation(AnimationTimeTicks, pNodeAnim);
    uint NextRotationIndex = RotationIndex + 1;
    assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);

    float t1 = (float)pNodeAnim->mRotationKeys[RotationIndex].mTime;
    float t2 = (float)pNodeAnim->mRotationKeys[NextRotationIndex].mTime;

    float DeltaTime = t2 - t1;
    float Factor = 0.0f;
    if (DeltaTime > 0.0f) {
        Factor = (AnimationTimeTicks - t1) / DeltaTime;
    }
    
    Factor = std::clamp(Factor, 0.0f, 1.0f);

    const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
    const aiQuaternion& EndRotationQ = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
    aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
    Out.Normalize();
}


uint CoreModel::FindScaling(float AnimationTimeTicks, const aiNodeAnim* pNodeAnim)
{
    assert(pNodeAnim->mNumScalingKeys > 0);

    for (uint i = 0 ; i < pNodeAnim->mNumScalingKeys - 1 ; i++) {
        float t = (float)pNodeAnim->mScalingKeys[i + 1].mTime;
        if (AnimationTimeTicks < t) {
            return i;
        }
    }

    return 0;
}


void CoreModel::CalcInterpolatedScaling(aiVector3D& Out, float AnimationTimeTicks, const aiNodeAnim* pNodeAnim)
{
    // we need at least two values to interpolate...
    if (pNodeAnim->mNumScalingKeys == 1) {
        Out = pNodeAnim->mScalingKeys[0].mValue;
        return;
    }

    uint ScalingIndex = FindScaling(AnimationTimeTicks, pNodeAnim);
    uint NextScalingIndex = ScalingIndex + 1;
    assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
    float t1 = (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime;
    if (t1 > AnimationTimeTicks) {
        Out = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
    } else {
        float t2 = (float)pNodeAnim->mScalingKeys[NextScalingIndex].mTime;
        float DeltaTime = t2 - t1;
        float Factor = (AnimationTimeTicks - (float)t1) / DeltaTime;
        assert(Factor >= 0.0f && Factor <= 1.0f);
        const aiVector3D& Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
        const aiVector3D& End   = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
        aiVector3D Delta = End - Start;
        Out = Start + Factor * Delta;
    }
}


void CoreModel::ReadNodeHierarchy(float AnimationTimeTicks, const aiNode* pNode, const Matrix4f& ParentTransform, const aiAnimation& Animation)
{
    std::string NodeName(pNode->mName.data);

    Matrix4f NodeTransformation(pNode->mTransformation);

    const aiNodeAnim* pNodeAnim = FindNodeAnim(Animation, NodeName);

    if (pNodeAnim) {
        LocalTransform Transform;
        CalcLocalTransform(Transform, AnimationTimeTicks, pNodeAnim);

        Matrix4f ScalingM;
        ScalingM.InitScaleTransform(Transform.Scaling.x, Transform.Scaling.y, Transform.Scaling.z);
        //        printf("Scaling %f %f %f\n", Transoform.Scaling.x, Transform.Scaling.y, Transform.Scaling.z);

        Matrix4f RotationM = Matrix4f(Transform.Rotation.GetMatrix());

        Matrix4f TranslationM;
        TranslationM.InitTranslationTransform(Transform.Translation.x, Transform.Translation.y, Transform.Translation.z);
        //        printf("Translation %f %f %f\n", Transform.Translation.x, Transform.Translation.y, Transform.Translation.z);

        // Combine the above transformations
        NodeTransformation = TranslationM * RotationM * ScalingM;
    }

    Matrix4f GlobalTransformation = ParentTransform * NodeTransformation;

    if (m_BoneNameToIndexMap.find(NodeName) != m_BoneNameToIndexMap.end()) {
        uint BoneIndex = m_BoneNameToIndexMap[NodeName];
        m_BoneInfo[BoneIndex].FinalTransformation = m_GlobalInverseTransform * GlobalTransformation * m_BoneInfo[BoneIndex].OffsetMatrix;
    }

    for (uint i = 0 ; i < pNode->mNumChildren ; i++) {
        std::string ChildName(pNode->mChildren[i]->mName.data);

        std::map<std::string,NodeInfo>::iterator it = m_requiredNodeMap.find(ChildName);

        if (it == m_requiredNodeMap.end()) {
            printf("Child %s cannot be found in the required node map\n", ChildName.c_str());
            assert(0);
        }

        if (it->second.isRequired) {
            ReadNodeHierarchy(AnimationTimeTicks, pNode->mChildren[i], GlobalTransformation, Animation);
        }
    }
}


void CoreModel::ReadNodeHierarchyBlended(float StartAnimationTimeTicks, float EndAnimationTimeTicks, const aiNode* pNode, const Matrix4f& ParentTransform,
                                         const aiAnimation& StartAnimation, const aiAnimation& EndAnimation, float BlendFactor) 
{
    std::string NodeName(pNode->mName.data);

    Matrix4f NodeTransformation(pNode->mTransformation);

    const aiNodeAnim* pStartNodeAnim = FindNodeAnim(StartAnimation, NodeName);

    LocalTransform StartTransform;

    if (pStartNodeAnim) {
        CalcLocalTransform(StartTransform, StartAnimationTimeTicks, pStartNodeAnim);
    }

    const aiNodeAnim* pEndNodeAnim = FindNodeAnim(EndAnimation, NodeName);
    LocalTransform EndTransform;
    if (pEndNodeAnim) {
        CalcLocalTransform(EndTransform, EndAnimationTimeTicks, pEndNodeAnim);
    }

    if (pStartNodeAnim && !pEndNodeAnim) {
        // Fallback to start transform only
        Matrix4f ScalingM, RotationM, TranslationM;
        ScalingM.InitScaleTransform(StartTransform.Scaling.x, StartTransform.Scaling.y, StartTransform.Scaling.z);
        RotationM = Matrix4f(StartTransform.Rotation.GetMatrix());
        TranslationM.InitTranslationTransform(StartTransform.Translation.x, StartTransform.Translation.y, StartTransform.Translation.z);
        NodeTransformation = TranslationM * RotationM * ScalingM;
    } else if (!pStartNodeAnim && pEndNodeAnim) {
        // Fallback to end transform only
        Matrix4f ScalingM, RotationM, TranslationM;
        ScalingM.InitScaleTransform(EndTransform.Scaling.x, EndTransform.Scaling.y, EndTransform.Scaling.z);
        RotationM = Matrix4f(EndTransform.Rotation.GetMatrix());
        TranslationM.InitTranslationTransform(EndTransform.Translation.x, EndTransform.Translation.y, EndTransform.Translation.z);
        NodeTransformation = TranslationM * RotationM * ScalingM;
    } else if (pStartNodeAnim && pEndNodeAnim) {
        // Interpolate scaling
        const aiVector3D& Scale0 = StartTransform.Scaling;
        const aiVector3D& Scale1 = EndTransform.Scaling;
        aiVector3D BlendedScaling = (1.0f - BlendFactor) * Scale0 + Scale1 * BlendFactor;
        Matrix4f ScalingM;
        ScalingM.InitScaleTransform(BlendedScaling.x, BlendedScaling.y, BlendedScaling.z);

        // Interpolate rotation
        const aiQuaternion& Rot0 = StartTransform.Rotation;
        const aiQuaternion& Rot1 = EndTransform.Rotation;
        aiQuaternion BlendedRot;
        aiQuaternion::Interpolate(BlendedRot, Rot0, Rot1, BlendFactor);
        Matrix4f RotationM = Matrix4f(BlendedRot.GetMatrix());

        // Interpolate translation
        const aiVector3D& Pos0 = StartTransform.Translation;
        const aiVector3D& Pos1 = EndTransform.Translation;
        aiVector3D BlendedTranslation = (1.0f - BlendFactor) * Pos0 + Pos1 * BlendFactor;
        Matrix4f TranslationM;
        TranslationM.InitTranslationTransform(BlendedTranslation.x, BlendedTranslation.y, BlendedTranslation.z);

        // Combine it all
        NodeTransformation = TranslationM * RotationM * ScalingM;
    }

    Matrix4f GlobalTransformation = ParentTransform * NodeTransformation;

    if (m_BoneNameToIndexMap.find(NodeName) != m_BoneNameToIndexMap.end()) {
        uint BoneIndex = m_BoneNameToIndexMap[NodeName];
        m_BoneInfo[BoneIndex].FinalTransformation = m_GlobalInverseTransform * GlobalTransformation * m_BoneInfo[BoneIndex].OffsetMatrix;
    }

    for (uint i = 0; i < pNode->mNumChildren; i++) {
        std::string ChildName(pNode->mChildren[i]->mName.data);
        auto it = m_requiredNodeMap.find(ChildName);
        if (it == m_requiredNodeMap.end()) {
            printf("Child %s cannot be found in the required node map\n", ChildName.c_str());
            assert(0);
        }

        if (it->second.isRequired) {
            ReadNodeHierarchyBlended(StartAnimationTimeTicks, EndAnimationTimeTicks,
                                     pNode->mChildren[i], GlobalTransformation, StartAnimation, EndAnimation, BlendFactor);
        }
    }
}


void CoreModel::CalcLocalTransform(LocalTransform& Transform, float AnimationTimeTicks, const aiNodeAnim* pNodeAnim)
{
    CalcInterpolatedScaling(Transform.Scaling, AnimationTimeTicks, pNodeAnim);
    CalcInterpolatedRotation(Transform.Rotation, AnimationTimeTicks, pNodeAnim);
    CalcInterpolatedPosition(Transform.Translation, AnimationTimeTicks, pNodeAnim);
}


void CoreModel::GetBoneTransforms(float TimeInSeconds, vector<Matrix4f>& Transforms, unsigned int AnimationIndex)
{
    if (AnimationIndex >= m_pScene->mNumAnimations) {
        printf("Invalid animation index %d, max is %d\n", AnimationIndex, m_pScene->mNumAnimations);
        assert(0);
    }

    Matrix4f Identity;
    Identity.InitIdentity();

    float AnimationTimeTicks = CalcAnimationTimeTicks(TimeInSeconds, AnimationIndex);
    const aiAnimation& Animation = *m_pScene->mAnimations[AnimationIndex];

    ReadNodeHierarchy(AnimationTimeTicks, m_pScene->mRootNode, Identity, Animation);
    Transforms.resize(m_BoneInfo.size());

    for (uint i = 0 ; i < m_BoneInfo.size() ; i++) {
        Transforms[i] = m_BoneInfo[i].FinalTransformation;
    }
}


void CoreModel::GetBoneTransformsBlended(float TimeInSeconds,
                                           std::vector<Matrix4f>& BlendedTransforms,
                                           unsigned int StartAnimIndex,
                                           unsigned int EndAnimIndex,
                                           float BlendFactor)
{
    if (StartAnimIndex >= m_pScene->mNumAnimations) {
        printf("Invalid start animation index %d, max is %d\n", StartAnimIndex, m_pScene->mNumAnimations);
        assert(0);
    }

    if (EndAnimIndex >= m_pScene->mNumAnimations) {
        printf("Invalid end animation index %d, max is %d\n", EndAnimIndex, m_pScene->mNumAnimations);
        assert(0);
    }

    if ((BlendFactor < 0.0f) || (BlendFactor > 1.0f)) {
        printf("Invalid blend factor %f\n", BlendFactor);
        assert(0);
    }

    float StartAnimationTimeTicks = CalcAnimationTimeTicks(TimeInSeconds, StartAnimIndex);
    float EndAnimationTimeTicks = CalcAnimationTimeTicks(TimeInSeconds, EndAnimIndex);

    const aiAnimation& StartAnimation = *m_pScene->mAnimations[StartAnimIndex];
    const aiAnimation& EndAnimation = *m_pScene->mAnimations[EndAnimIndex];

    Matrix4f Identity;
    Identity.InitIdentity();

    ReadNodeHierarchyBlended(StartAnimationTimeTicks, EndAnimationTimeTicks, m_pScene->mRootNode, Identity, StartAnimation, EndAnimation, BlendFactor);

    BlendedTransforms.resize(m_BoneInfo.size());

    for (uint i = 0 ; i < m_BoneInfo.size() ; i++) {
        BlendedTransforms[i] = m_BoneInfo[i].FinalTransformation;
    }
}


float CoreModel::CalcAnimationTimeTicks(float TimeInSeconds, unsigned int AnimationIndex) 
{
    float TicksPerSecond = (float)(m_pScene->mAnimations[AnimationIndex]->mTicksPerSecond != 0 ? m_pScene->mAnimations[AnimationIndex]->mTicksPerSecond : 25.0f);
    float TimeInTicks = TimeInSeconds * TicksPerSecond;
    // we need to use the integral part of mDuration for the total length of the animation
    float Duration = 0.0f;
    modf((float)m_pScene->mAnimations[AnimationIndex]->mDuration, &Duration);

    if (Duration <= 0.0f) {
        return 0.0f;
    }

    return std::fmod(TimeInTicks, Duration);
}


// TODO: Optimize this function by using a hash map for faster lookups if performance becomes an issue.
const aiNodeAnim* CoreModel::FindNodeAnim(const aiAnimation& Animation, const std::string& NodeName)
{
    for (uint i = 0 ; i < Animation.mNumChannels ; i++) {
        const aiNodeAnim* pNodeAnim = Animation.mChannels[i];

        if (string(pNodeAnim->mNodeName.data) == NodeName) {
            return pNodeAnim;
        }
    }

    return NULL;
}


bool CoreModel::IsAnimated() const
{
    bool ret = m_numAnimations > 0;

    if (ret && (NumBones() == 0)) {
        printf("Animations without bones? need to check this\n");
        assert(0);
    }

    return ret;
}


void GetVertexSizesInBytes(size_t& VertexSize, size_t& SkinnedVertexSize)
{
    CoreModel::GetVertexSizesInBytes(VertexSize, SkinnedVertexSize);
}
