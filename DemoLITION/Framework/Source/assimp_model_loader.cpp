/*

        Copyright 2026 Etay Meiri

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

#include <string>
#include <filesystem>
#include <type_traits>
#include <cassert>
#include <cstdio>
#include <algorithm>
#include <cstring>

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>       // Output data structure
#include <assimp/postprocess.h> // Post processing flags
#include <assimp/GltfMaterial.h>

#include "Int/assimp_model_loader.h"
#include "3rdparty/meshoptimizer/src/meshoptimizer.h"

//#define DEBUG_COLORS
//#define DEBUG_MATERIALS
//#define DEBUG_TEXTURES
//#define DEBUG_BONES


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
aiProcess_CalcTangentSpace |    \
aiProcess_FindDegenerates | \
aiProcess_FindInvalidData | \
aiProcess_GenBoundingBoxes | \
aiProcess_GenSmoothNormals |    \
aiProcess_GenUVCoords | \
aiProcess_ImproveCacheLocality | \
aiProcess_JoinIdenticalVertices |   \
aiProcess_LimitBoneWeights | \
aiProcess_RemoveRedundantMaterials | \
aiProcess_SortByPType | \
aiProcess_Triangulate | \
aiProcess_ValidateDataStructure)
#else

#define DEMOLITION_ASSIMP_LOAD_FLAGS (aiProcess_JoinIdenticalVertices | \
                                      aiProcess_Triangulate | \
                                      aiProcess_GenBoundingBoxes | \
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


static bool SceneHasBones(const aiScene* pScene)
{
    for (unsigned i = 0; i < pScene->mNumMeshes; i++) {
        if (pScene->mMeshes[i]->mNumBones > 0) {
            return true;
        }
    }

    return false;
}

static int traverse(int depth, const aiNode* pNode)
{
    if (!pNode) {
        return 0;
    }

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

    int Count = 1;

    for (uint i = 0; i < pNode->mNumChildren; i++) {
        Count += traverse(depth + 1, pNode->mChildren[i]);
    }

    return Count;
}


static Vector3f VectorFromAssimpVector(const aiVector3D& v)
{
    Vector3f ret;

    ret.x = v.x;
    ret.y = v.y;
    ret.z = v.z;

    return ret;
}


static uint CountValidFaces(const aiMesh& Mesh)
{
    uint NumValidFaces = 0;

    for (uint i = 0; i < Mesh.mNumFaces; i++) {
        if (Mesh.mFaces[i].mNumIndices == 3) {
            NumValidFaces++;
        }
    }

    return NumValidFaces;
}


static void SetMaterialType(const aiMaterial* pMaterial, MaterialData& MyMaterial)
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


static std::string GetFullPath(const std::string& Dir, const aiString& Path)
{
    std::filesystem::path p(Path.C_Str());
    std::string ret = (std::filesystem::path(Dir) / p).generic_string();
    // .generic_string() automatically converts windows \ to /
    return ret;
}


void AssimpModelLoader::Load(const std::string& Filename, const ModelLoadFlags& Flags, ModelData& OutModelData)
{
    m_filename = Filename;
    m_pModelData = &OutModelData;

    assert(OutModelData.m_meshes.empty());

#ifndef OGLDEV_VULKAN
    std::string FileExt = Filename.substr(Filename.find_last_of(".") + 1);
    FileExt = ToLower(FileExt);
    m_flipTexCoordV = (FileExt == "glb" || FileExt == "gltf");
#endif

    uint LoadFlags = DEMOLITION_ASSIMP_LOAD_FLAGS;

    m_convertToLeftHanded = Flags.ConvertToLeftHanded;

    if (Flags.ConvertToLeftHanded) {
        LoadFlags |= aiProcess_ConvertToLeftHanded;
    }

    Assimp::Importer Importer;
    m_pScene = Importer.ReadFile(Filename.c_str(), LoadFlags);

    if (m_pScene) {
        printf("--- START Node Hierarchy ---\n");
        int NumNodes = traverse(0, m_pScene->mRootNode);
        printf("Num nodes: %d\n", NumNodes);
        printf("--- END Node Hierarchy ---\n");
        m_pModelData->m_globalInverseTransform = m_pScene->mRootNode->mTransformation;
        m_pModelData->m_globalInverseTransform = m_pModelData->m_globalInverseTransform.Inverse();
        InitFromScene();
        m_pScene = NULL;
    } else {
        OGLDEV_ERROR("Error parsing '%s': '%s'\n", Filename.c_str(), Importer.GetErrorString());
    }

    m_pModelData = NULL;
}


void AssimpModelLoader::InitFromScene()
{
    InitMaterials();

    InitSkeletonHierarchy();

    InitGeometry();

    InitCameras();

    InitLights();

    InitAnimations();
}


void AssimpModelLoader::InitGeometry()
{
    printf("\n*** Initializing geometry ***\n");

    m_pModelData->m_meshes.resize(m_pScene->mNumMeshes);

    uint NumVertices = 0;
    uint NumIndices = 0;

    CountVerticesAndIndices(NumVertices, NumIndices);    

    InitMeshes(NumVertices, NumIndices);
}


void AssimpModelLoader::CountVerticesAndIndices(uint& NumVertices, uint& NumIndices)
{
    for (unsigned int i = 0; i < m_pModelData->m_meshes.size(); i++) {
        m_pModelData->m_meshes[i].MaterialIndex = m_pScene->mMeshes[i]->mMaterialIndex;
        m_pModelData->m_meshes[i].ValidFaces = CountValidFaces(*m_pScene->mMeshes[i]);

        NumVertices += m_pScene->mMeshes[i]->mNumVertices;
        NumIndices += m_pModelData->m_meshes[i].ValidFaces * 3;
    }
}


void AssimpModelLoader::InitMeshes(unsigned int NumVertices, unsigned int NumIndices)
{
    m_pModelData->m_indices.reserve(NumIndices);

    if (SceneHasBones(m_pScene)) {
        m_pModelData->m_skinnedVertices.reserve(NumVertices);
        InitAllMeshes(m_pModelData->m_skinnedVertices);
    } else {
        m_pModelData->m_vertices.reserve(NumVertices);
        InitAllMeshes(m_pModelData->m_vertices);
    }
}


template<typename VertexType>
void AssimpModelLoader::InitAllMeshes(std::vector<VertexType>& Vertices)
{
    for (unsigned int i = 0; i < m_pModelData->m_meshes.size(); i++) {
        const aiMesh* paiMesh = m_pScene->mMeshes[i];

        aiAABB BBox = paiMesh->mAABB;
        m_pModelData->m_meshes[i].Dims.BboxMin = glm::vec3(BBox.mMin.x, BBox.mMin.y, BBox.mMin.z);
        m_pModelData->m_meshes[i].Dims.BboxMax = glm::vec3(BBox.mMax.x, BBox.mMax.y, BBox.mMax.z);

        float Width = BBox.mMax.x - BBox.mMin.x;
        float Height = BBox.mMax.y - BBox.mMin.y;
        float Depth = BBox.mMax.z - BBox.mMin.z;
        m_pModelData->m_meshes[i].Dims.Size = glm::vec3(Width, Height, Depth);

        if (UseMeshOptimizer) {
            InitSingleMeshOpt<VertexType>(Vertices, i, paiMesh);
        } else {
            InitSingleMesh<VertexType>(Vertices, i, paiMesh);
        }
    }
}


template<typename VertexType>
void AssimpModelLoader::InitSingleMesh(std::vector<VertexType>& Vertices, uint MeshIndex, const aiMesh* paiMesh)
{
   // printf("Mesh %d: %s\n", MeshIndex, paiMesh->mName.C_Str());
   // printf("Size: %f, %f, %f\n", m_pModelData->m_meshes[MeshIndex].Dims.Size.x, m_pModelData->m_meshes[MeshIndex].Dims.Size.y, m_pModelData->m_meshes[MeshIndex].Dims.Size.z);

    // Update exact data tracking directly from current dynamic array size to prevent offset corruption
    m_pModelData->m_meshes[MeshIndex].BaseVertex = (uint)(Vertices.size());
    m_pModelData->m_meshes[MeshIndex].BaseIndex = (uint)(m_pModelData->m_indices.size());

    unsigned int AddedVertices = 0;
    unsigned int AddedIndices = 0;

    for (unsigned int i = 0; i < paiMesh->mNumVertices; i++) {
        VertexType v;
        const aiVector3D& Pos = paiMesh->mVertices[i];
        v.Position = Vector3f(Pos.x, Pos.y, Pos.z);

        if (paiMesh->mNormals) {
            v.Normal = Vector3f(paiMesh->mNormals[i].x, paiMesh->mNormals[i].y, paiMesh->mNormals[i].z);
        } else {
            v.Normal = Vector3f(0.0f, 1.0f, 0.0f);
        }

        v.TexCoords0 = paiMesh->HasTextureCoords(0) ? Vector2f(paiMesh->mTextureCoords[0][i].x, paiMesh->mTextureCoords[0][i].y) :
            Vector2f(0.0f);
        v.TexCoords1 = paiMesh->HasTextureCoords(1) ? Vector2f(paiMesh->mTextureCoords[1][i].x, paiMesh->mTextureCoords[1][i].y) :
            Vector2f(0.0f);
        if (m_flipTexCoordV) {
            v.TexCoords0.y = 1.0f - v.TexCoords0.y;
            v.TexCoords1.y = 1.0f - v.TexCoords1.y;
        }

        v.Tangent = paiMesh->mTangents ? Vector3f(paiMesh->mTangents[i].x, paiMesh->mTangents[i].y, paiMesh->mTangents[i].z) :
            Vector3f(0.0f);
        v.Bitangent = paiMesh->mBitangents ? Vector3f(paiMesh->mBitangents[i].x, paiMesh->mBitangents[i].y, paiMesh->mBitangents[i].z) :
            Vector3f(0.0f);
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
        m_pModelData->m_indices.push_back(Face.mIndices[0]);
        m_pModelData->m_indices.push_back(Face.mIndices[1]);
        m_pModelData->m_indices.push_back(Face.mIndices[2]);
        AddedIndices += 3;
    }

    // Re-assign accurate post-parsing count figures
    m_pModelData->m_meshes[MeshIndex].NumVertices = AddedVertices;
    m_pModelData->m_meshes[MeshIndex].NumIndices = AddedIndices;

    if constexpr (std::is_same_v<VertexType, SkinnedVertex>) {
        // Vertices holds the vertices of all the meshes so far
        LoadMeshBones(Vertices, MeshIndex, paiMesh, m_pModelData->m_meshes[MeshIndex].BaseVertex);
    }
}


template<typename VertexType>
void AssimpModelLoader::InitSingleMeshOpt(std::vector<VertexType>& AllVertices, uint MeshIndex, const aiMesh* paiMesh)
{
    const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

    std::vector<VertexType> Vertices(paiMesh->mNumVertices);
    for (unsigned int i = 0; i < paiMesh->mNumVertices; i++) {
        VertexType v;
        const aiVector3D& Pos = paiMesh->mVertices[i];

        v.Position = Vector3f(Pos.x, Pos.y, Pos.z);

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

    m_pModelData->m_meshes[MeshIndex].BaseVertex = (uint)AllVertices.size();
    m_pModelData->m_meshes[MeshIndex].BaseIndex = (uint)m_pModelData->m_indices.size();

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
        // Vertices holds only the vertices of this mesh (before they are optimized
        // and appended to AllVertices) so the weights are indexed from zero
        LoadMeshBones(Vertices, MeshIndex, paiMesh, 0);
    }

    OptimizeMesh(MeshIndex, Indices, Vertices, AllVertices);
}


template<typename VertexType>
void AssimpModelLoader::OptimizeMesh(int MeshIndex, std::vector<uint>& Indices, std::vector<VertexType>& Vertices,
    std::vector<VertexType>& AllVertices)
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

    m_pModelData->m_meshes[MeshIndex].NumVertices = (uint)OptVertexCount;

    float Threshold = 1.0f;
    if (Threshold < 1.0f) { // Only run simplification code if reduction threshold is requested
        size_t TargetIndexCount = (size_t)(NumIndices * Threshold);
        float TargetError = 0.01f;
        std::vector<unsigned int> SimplifiedIndices(OptIndices.size());
        size_t OptIndexCount = meshopt_simplify(SimplifiedIndices.data(), OptIndices.data(), NumIndices, &OptVertices[0].Position.x, OptVertexCount, sizeof(VertexType), TargetIndexCount, TargetError);
        SimplifiedIndices.resize(OptIndexCount);

        m_pModelData->m_indices.insert(m_pModelData->m_indices.end(), SimplifiedIndices.begin(), SimplifiedIndices.end());
        m_pModelData->m_meshes[MeshIndex].NumIndices = (uint)OptIndexCount;
    } else {
        m_pModelData->m_indices.insert(m_pModelData->m_indices.end(), OptIndices.begin(), OptIndices.end());
        m_pModelData->m_meshes[MeshIndex].NumIndices = (uint)NumIndices;
    }

    AllVertices.insert(AllVertices.end(), OptVertices.begin(), OptVertices.end());
}


void AssimpModelLoader::InitCameras()
{
    printf("\n*** Initializing cameras ***\n");
    printf("Loading %d cameras\n", m_pScene->mNumCameras);

    m_pModelData->m_cameras.resize(m_pScene->mNumCameras);

    for (unsigned int i = 0; i < m_pScene->mNumCameras; i++) {
        InitSingleCamera(i);
    }
}


// Transforms a point (w = 1) or a direction (w = 0) that is local to a camera/light node
// into the left handed world space of the engine (the right handed scene mirrored in Z).
// aiProcess_ConvertToLeftHanded mirrors the node transformations but not the local
// vectors of cameras and lights, so in that case the local vector is mirrored instead.
Vector3f AssimpModelLoader::LocalToLeftHandedWorld(const Matrix4f& NodeWorld, const aiVector3D& v, float w) const
{
    if (m_convertToLeftHanded) {
        Vector4f World = NodeWorld * Vector4f(v.x, v.y, -v.z, w);
        return Vector3f(World.x, World.y, World.z);
    } else {
        Vector4f World = NodeWorld * Vector4f(v.x, v.y, v.z, w);
        return Vector3f(World.x, World.y, -World.z);
    }
}


void AssimpModelLoader::InitSingleCamera(int Index)
{
    const aiCamera* pCamera = m_pScene->mCameras[Index];
    m_pModelData->m_cameras[Index].m_name = pCamera->mName.C_Str();
    printf("Loading Camera: '%s'\n", pCamera->mName.C_Str());

    Matrix4f Transformation;
    m_pModelData->GetNodeWorldTransform(pCamera->mName.C_Str(), Transformation);
    printf("Camera transformation:\n");
    Transformation.Print();

    printf("Camera local position: (%f, %f, %f)\n", pCamera->mPosition.x, pCamera->mPosition.y, pCamera->mPosition.z);
    printf("Camera local look at: (%f, %f, %f)\n", pCamera->mLookAt.x, pCamera->mLookAt.y, pCamera->mLookAt.z);

    // mPosition is relative to the camera node, but the glTF importer copies
    // the node's own translation into it. In that case it is already part of
    // the node transformation and must not be applied again.
    // (with aiProcess_ConvertToLeftHanded the node translation is mirrored but mPosition isn't)
    aiVector3D LocalPos = pCamera->mPosition;
    const aiNode* pNode = m_pScene->mRootNode->FindNode(pCamera->mName);

    if (pNode) {
        const aiMatrix4x4& m = pNode->mTransformation;
        aiVector3D NodeTranslation(m.a4, m.b4, m_convertToLeftHanded ? -m.c4 : m.c4);

        if ((LocalPos - NodeTranslation).Length() < 0.0001f) {
            LocalPos = aiVector3D(0.0f, 0.0f, 0.0f);
        }
    }

    // Extract World Position
    Vector3f FinalPos = LocalToLeftHandedWorld(Transformation, LocalPos, 1.0f);

    // Extract World Direction (mLookAt is relative to the camera node)
    Vector3f FinalTargetDir = LocalToLeftHandedWorld(Transformation, pCamera->mLookAt, 0.0f).Normalize();

    // Extract World Up (mUp is relative to the camera node)
    Vector3f FinalUp = LocalToLeftHandedWorld(Transformation, pCamera->mUp, 0.0f).Normalize();

    // Setup Perspective Projection
    m_pModelData->m_cameras[Index].m_zNear = pCamera->mClipPlaneNear;
    m_pModelData->m_cameras[Index].m_zFar = pCamera->mClipPlaneFar;

    float FullHorizontalDeg = ToDegree(pCamera->mHorizontalFOV);
    m_pModelData->m_cameras[Index].m_hFov = FullHorizontalDeg;

    // Assign the FULL vertical degree angle straight to your projection settings
    // (Do NOT divide by 2! glm::perspective handles halving internally via its cotangent math)
    m_pModelData->m_cameras[Index].m_pos = glm::vec3(FinalPos.x, FinalPos.y, FinalPos.z);
    m_pModelData->m_cameras[Index].m_dir = glm::vec3(FinalTargetDir.x, FinalTargetDir.y, FinalTargetDir.z);
    m_pModelData->m_cameras[Index].m_up = glm::vec3(FinalUp.x, FinalUp.y, FinalUp.z);

    printf("Final Pos: "); FinalPos.Print();
    printf("Final Dir: "); FinalTargetDir.Print();
    printf("Final Up: "); FinalUp.Print();
}


void AssimpModelLoader::InitLights()
{
    printf("\n*** Initializing lights ***\n");

    for (int i = 0; i < (int)m_pScene->mNumLights; i++) {
        InitSingleLight(*m_pScene->mLights[i]);
    }
}


void AssimpModelLoader::InitSingleLight(const aiLight& light)
{
    printf("Init light '%s'\n", light.mName.C_Str());

    switch (light.mType) {

    case aiLightSource_DIRECTIONAL:
        InitDirectionalLight(light);
        break;

    case aiLightSource_POINT:
        InitPointLight(light);
        break;

    case aiLightSource_SPOT:
        InitSpotLight(light);
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


static glm::vec4 ConvertAssimpColor(const aiColor3D& c)
{
    float Intensity = std::max({ c.r, c.g, c.b });          // the multiplier Assimp folded in
    glm::vec3 Color = (Intensity > 0.0f) ? glm::vec3(c.r, c.g, c.b) / Intensity : glm::vec3(1.0f);
    float m_lightUnitScale = 0.0005f;
    float DiffuseIntensity = Intensity * m_lightUnitScale;   // one scale factor, exposed in the GUI
    glm::vec4 ret = glm::vec4(Color, DiffuseIntensity);
    return ret;
}

void AssimpModelLoader::InitDirectionalLight(const aiLight& light)
{
    glm::vec4 ColorAndIntensity = ConvertAssimpColor(light.mColorDiffuse);

    DirectionalLight l;
    l.Color = Vector3f(ColorAndIntensity.x, ColorAndIntensity.y, ColorAndIntensity.z);
    l.DiffuseIntensity = ColorAndIntensity.w;

    Matrix4f Transformation;
    m_pModelData->GetNodeWorldTransform(light.mName.C_Str(), Transformation);

    // mDirection is relative to the light node (glTF lights point down the local -Z)
    printf("Original direction: (%f, %f, %f)\n", light.mDirection.x, light.mDirection.y, light.mDirection.z);

    // Normalize to preserve clean calculations for your lighting equations
    l.WorldDirection = LocalToLeftHandedWorld(Transformation, light.mDirection, 0.0f).Normalize();

    printf("Adjusted World Direction: "); l.WorldDirection.Print();

    Vector3f Up = VectorFromAssimpVector(light.mUp);
    printf("Original up: "); Up.Print();

    l.Up = LocalToLeftHandedWorld(Transformation, light.mUp, 0.0f);
    printf("Final up: "); l.Up.Print();

    m_pModelData->m_dirLights.push_back(l);
}


void AssimpModelLoader::InitPointLight(const aiLight& light)
{
    glm::vec4 ColorAndIntensity = ConvertAssimpColor(light.mColorDiffuse);

    PointLight l;
    l.Color = Vector3f(ColorAndIntensity.x, ColorAndIntensity.y, ColorAndIntensity.z);
    printf("Point light color [%f %f %f]\n", l.Color.x, l.Color.y, l.Color.z);
    l.DiffuseIntensity = ColorAndIntensity.w;

    Vector3f Position = VectorFromAssimpVector(light.mPosition);
    printf("Original Position: "); Position.Print();

    Matrix4f Transformation;

    m_pModelData->GetNodeWorldTransform(light.mName.C_Str(), Transformation);

    Vector4f Pos4D(Position, 1.0f);
    Pos4D = Transformation * Pos4D;

    l.WorldPosition = Vector3f(Pos4D);
    printf("Final Position: "); l.WorldPosition.Print();

    l.Attenuation.Constant = light.mAttenuationConstant;
    l.Attenuation.Linear = light.mAttenuationLinear;
    l.Attenuation.Exp = light.mAttenuationQuadratic;

    printf("Attenuation: constant %f linear %f exp %f\n", l.Attenuation.Constant, l.Attenuation.Linear, l.Attenuation.Exp);

    m_pModelData->m_pointLights.push_back(l);
}


void AssimpModelLoader::InitSpotLight(const aiLight& light)
{
    glm::vec4 ColorAndIntensity = ConvertAssimpColor(light.mColorDiffuse);

    SpotLight l;    
    l.Color = Vector3f(ColorAndIntensity.x, ColorAndIntensity.y, ColorAndIntensity.z);
    l.DiffuseIntensity = ColorAndIntensity.w;

    Matrix4f Transformation;
    m_pModelData->GetNodeWorldTransform(light.mName.C_Str(), Transformation);

    // mDirection is relative to the light node (glTF lights point down the local -Z)
    printf("Original direction: (%f, %f, %f)\n", light.mDirection.x, light.mDirection.y, light.mDirection.z);

    // Normalize to preserve clean calculations for your lighting equations
    l.WorldDirection = LocalToLeftHandedWorld(Transformation, light.mDirection, 0.0f).Normalize();
    printf("Final direction: "); l.WorldDirection.Print();

    Vector3f Up = VectorFromAssimpVector(light.mUp);
    printf("Original up: "); Up.Print();
    if (Up.Length() == 0) {
        // Use clean 3D vector literal dimensions for structural comparisons
        if (fabsf(l.WorldDirection.Dot(Vector3f(0.0f, 1.0f, 0.0f))) > 0.999f) {
            l.Up = Vector3f(1.0f, 0.0f, 0.0f);
        } else {
            l.Up = Vector3f(0.0f, 1.0f, 0.0f);
        }
    } else {
        l.Up = LocalToLeftHandedWorld(Transformation, light.mUp, 0.0f);
    }

    printf("Final up: "); l.Up.Print();

    Vector3f Position = VectorFromAssimpVector(light.mPosition);
    printf("Original Position: "); Position.Print();

    l.WorldPosition = LocalToLeftHandedWorld(Transformation, light.mPosition, 1.0f);
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

    m_pModelData->m_spotLights.push_back(l);
}


void AssimpModelLoader::InitMaterials()
{
    std::string Dir = GetDirFromFilename(m_filename);

    uint NumMaterials = m_pScene->mNumMaterials;
    printf("Num materials: %d\n", NumMaterials);

    m_pModelData->m_materials.resize(NumMaterials);

    // Initialize the materials
    for (unsigned int i = 0; i < NumMaterials; i++) {
        const aiMaterial* pMaterial = m_pScene->mMaterials[i];

        MaterialData& MyMaterial = m_pModelData->m_materials[i];

        MyMaterial.m_name = pMaterial->GetName().C_Str();
#ifdef DEBUG_MATERIALS
        printf("\nLoading material %d: '%s'\n", i, pMaterial->GetName().C_Str());
#endif
        SetMaterialType(pMaterial, MyMaterial);

        LoadTextures(Dir, pMaterial, i);

        LoadColors(pMaterial, MyMaterial);
    }
}


void AssimpModelLoader::LoadTextures(const std::string& Dir, const aiMaterial* pMaterial, int index)
{
    int TextureCount = GetTextureCount(pMaterial);
    printf("Number of textures %d\n", TextureCount);

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


void AssimpModelLoader::LoadDiffuseTexture(const std::string& Dir, const aiMaterial* pMaterial, int MaterialIndex)
{
    if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
        LoadTexture(Dir, pMaterial, MaterialIndex, aiTextureType_DIFFUSE, 0, TEX_TYPE_BASE);
    } else {
       // printf("Warning! no diffuse texture\n");

        aiString Path;

        if (MissingTextureDetection) {
            Path = ("no_texture.png");  // Big red texture with the text "no texture" to make it obvious when a texture is missing
        } else {
            Path = ("white.png");   // Single pixel texture to minimize the performance impact of missing textures
        }

        std::string DefaultTextureDir = GetOgldevPath("Content/textures/");
        std::string FullPath = GetFullPath(DefaultTextureDir, Path);
        m_pModelData->m_materials[MaterialIndex].m_textures[TEX_TYPE_BASE].Path = FullPath;
    }
}


void AssimpModelLoader::LoadSpecularTexture(const std::string& Dir, const aiMaterial* pMaterial, int MaterialIndex)
{
    LoadTexture(Dir, pMaterial, MaterialIndex, aiTextureType_SHININESS, 0, TEX_TYPE_SPECULAR);
}


void AssimpModelLoader::LoadNormalTexture(const std::string& Dir, const aiMaterial* pMaterial, int MaterialIndex)
{
    LoadTexture(Dir, pMaterial, MaterialIndex, aiTextureType_NORMALS, 0, TEX_TYPE_NORMAL);
}


void AssimpModelLoader::LoadMetalnessTexture(const std::string& Dir, const aiMaterial* pMaterial, int MaterialIndex)
{
    LoadTexture(Dir, pMaterial, MaterialIndex, aiTextureType_METALNESS, 0, TEX_TYPE_METALNESS);
}


void AssimpModelLoader::LoadEmissiveTexture(const std::string& Dir, const aiMaterial* pMaterial, int MaterialIndex)
{
    LoadTexture(Dir, pMaterial, MaterialIndex, aiTextureType_EMISSIVE, 0, TEX_TYPE_EMISSIVE);
}


void AssimpModelLoader::LoadEmissionColorTexture(const std::string& Dir, const aiMaterial* pMaterial, int MaterialIndex)
{
    LoadTexture(Dir, pMaterial, MaterialIndex, aiTextureType_EMISSION_COLOR, 0, TEX_TYPE_EMISSION_COLOR);
}


void AssimpModelLoader::LoadNormalCameraTexture(const std::string& Dir, const aiMaterial* pMaterial, int MaterialIndex)
{
    LoadTexture(Dir, pMaterial, MaterialIndex, aiTextureType_NORMAL_CAMERA, 0, TEX_TYPE_NORMAL_CAMERA);
}


void AssimpModelLoader::LoadRoughnessTexture(const std::string& Dir, const aiMaterial* pMaterial, int MaterialIndex)
{
    LoadTexture(Dir, pMaterial, MaterialIndex, aiTextureType_DIFFUSE_ROUGHNESS, 0, TEX_TYPE_ROUGHNESS);
}


void AssimpModelLoader::LoadAmbientOcclusionTexture(const std::string& Dir, const aiMaterial* pMaterial, int MaterialIndex)
{
    LoadTexture(Dir, pMaterial, MaterialIndex, aiTextureType_UNKNOWN, 0, TEX_TYPE_AMBIENT_OCCLUSION);
}


void AssimpModelLoader::LoadClearCoatTexture(const std::string& Dir, const aiMaterial* pMaterial, int MaterialIndex)
{
    LoadTexture(Dir, pMaterial, MaterialIndex, aiTextureType_CLEARCOAT, 0, TEX_TYPE_CLEARCOAT);
}


void AssimpModelLoader::LoadClearCoatRoughnessTexture(const std::string& Dir, const aiMaterial* pMaterial, int MaterialIndex)
{
    LoadTexture(Dir, pMaterial, MaterialIndex, aiTextureType_CLEARCOAT, 1, TEX_TYPE_CLEARCOAT_ROUGHNESS);
}


void AssimpModelLoader::LoadClearCoatNormalTexture(const std::string& Dir, const aiMaterial* pMaterial, int MaterialIndex)
{
    LoadTexture(Dir, pMaterial, MaterialIndex, aiTextureType_CLEARCOAT, 2, TEX_TYPE_CLEARCOAT_NORMAL);
}


void AssimpModelLoader::LoadTexture(const std::string& Dir, const aiMaterial* pMaterial, int MaterialIndex, 
                                    int AssimpTextureType, int AssimpTexIndex, TEXTURE_TYPE MyType)
{
    assert(MyType < TEX_TYPE_NUM);

    if (pMaterial->GetTextureCount((aiTextureType)AssimpTextureType) > 0) {
        aiString Path;

        if (pMaterial->GetTexture((aiTextureType)AssimpTextureType, AssimpTexIndex, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
            const aiTexture* paiTexture = m_pScene->GetEmbeddedTexture(Path.C_Str());

            bool IsSRGB = (MyType == TEX_TYPE_BASE) || (MyType == TEX_TYPE_EMISSIVE) || (MyType == TEX_TYPE_EMISSION_COLOR);

            m_pModelData->m_materials[MaterialIndex].m_textures[MyType].IsSRGB = IsSRGB;

            if (paiTexture) {
                assert(paiTexture->mHeight == 0);
                int BufferSize = paiTexture->mWidth;
                m_pModelData->m_materials[MaterialIndex].m_textures[MyType].EmbeddedBytes.resize(BufferSize);
                memcpy(m_pModelData->m_materials[MaterialIndex].m_textures[MyType].EmbeddedBytes.data(), 
                       paiTexture->pcData, BufferSize);
            } else {
                std::string FullPath = GetFullPath(Dir, Path);
                m_pModelData->m_materials[MaterialIndex].m_textures[MyType].Path = FullPath;                
            }
        }
    }
}


void AssimpModelLoader::LoadColors(const aiMaterial* pMaterial, MaterialData& MyMaterial)
{
    LoadColor(pMaterial, MyMaterial.m_ambientColor, AI_MATKEY_COLOR_AMBIENT, "ambient color");

    LoadColor(pMaterial, MyMaterial.m_diffuseColor, AI_MATKEY_COLOR_DIFFUSE, "diffuse color");

    LoadColor(pMaterial, MyMaterial.m_specularColor, AI_MATKEY_COLOR_SPECULAR, "specular color");

    LoadColor(pMaterial, MyMaterial.m_baseColor, AI_MATKEY_BASE_COLOR, "base color");

    // Emissive light is added once by the shaders, independent of the lights.
    // Unlike the other colors (see LoadColor) a missing emissive color means no emission.
    aiColor4D EmissiveColor(0.0f, 0.0f, 0.0f, 0.0f);

    if (pMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, EmissiveColor) == AI_SUCCESS) {
#ifdef DEBUG_COLORS
        printf("Loaded emissive color [%f %f %f]\n", EmissiveColor.r, EmissiveColor.g, EmissiveColor.b);
#endif
    }

    MyMaterial.m_emissiveColor = Vector4f(EmissiveColor.r, EmissiveColor.g, EmissiveColor.b, 0.0f);

    float EmissiveStrength = 1.0f;
    if (pMaterial->Get(AI_MATKEY_EMISSIVE_INTENSITY, EmissiveStrength) == AI_SUCCESS) {
        MyMaterial.m_emissiveColor = MyMaterial.m_emissiveColor * Vector4f(EmissiveStrength, EmissiveStrength, EmissiveStrength, 1.0f);
    }

    if (MyMaterial.m_materialType & MaterialType_MetallicRoughness) {
        float MetallicFactor;

        if (pMaterial->Get(AI_MATKEY_METALLIC_FACTOR, MetallicFactor) == AI_SUCCESS) {
            MyMaterial.m_metallicRoughnessNormalOcclusion.x = MetallicFactor;
        }

        float RoughnessFactor;

        if (pMaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, RoughnessFactor) == AI_SUCCESS) {
            MyMaterial.m_metallicRoughnessNormalOcclusion.y = RoughnessFactor;
        }
    }

    float NormalScale;
    if (pMaterial->Get(AI_MATKEY_GLTF_TEXTURE_SCALE(aiTextureType_NORMALS, 0), NormalScale) == AI_SUCCESS) {
        MyMaterial.m_metallicRoughnessNormalOcclusion.z = NormalScale;
    }

    float OcclusionStrength;
    if (pMaterial->Get(AI_MATKEY_GLTF_TEXTURE_SCALE(aiTextureType_LIGHTMAP, 0), OcclusionStrength) == AI_SUCCESS) {
        MyMaterial.m_metallicRoughnessNormalOcclusion.w = OcclusionStrength;
    }

    float AlphaCutoff = 0.0f;

    if (pMaterial->Get(AI_MATKEY_GLTF_ALPHACUTOFF, AlphaCutoff) == AI_SUCCESS) {
        MyMaterial.m_emissiveColor.a = AlphaCutoff;
    } else {
        MyMaterial.m_emissiveColor.a = 0.5f;    // Based on the "3D Rendering Cookbook"
    }

    float OpaquenessThreshold = 0.05f;
    float Opacity = 1.0f;
    if (pMaterial->Get(AI_MATKEY_OPACITY, Opacity) == AI_SUCCESS) {
        MyMaterial.m_transparencyFactor = std::clamp(1.0f - Opacity, 0.0f, 1.0f);
        if (MyMaterial.m_transparencyFactor >= 1.0f - OpaquenessThreshold) {
            MyMaterial.m_transparencyFactor = 0.0f;
        }
    }

    aiColor4D TransparentColor;
    if (pMaterial->Get(AI_MATKEY_COLOR_TRANSPARENT, TransparentColor) == AI_SUCCESS) {
        float Opacity = std::max(std::max(TransparentColor.r, TransparentColor.g), TransparentColor.b);
        MyMaterial.m_transparencyFactor = std::clamp(Opacity, 0.0f, 1.0f);
        if (MyMaterial.m_transparencyFactor >= 1.0f - OpaquenessThreshold) {
            MyMaterial.m_transparencyFactor = 0.0f;
        }

        MyMaterial.m_alphaTest = 0.5f;
    }

    ProcessClearCoat(pMaterial, MyMaterial);
        
    if ((MyMaterial.m_ambientColor.r == 0.0f) && (MyMaterial.m_ambientColor.g == 0.0f) && (MyMaterial.m_ambientColor.b == 0.0f)) {
        // THE MIXAMO FALLBACK: If the DAE file provides a zero ambient vector,
        // match the diffuse color property so the ambient light works.
        MyMaterial.m_ambientColor = MyMaterial.m_diffuseColor;
    }
}


void AssimpModelLoader::ProcessClearCoat(const aiMaterial* pMaterial, MaterialData& MyMaterial)
{
    bool UseClearCoat = MyMaterial.m_textures[TEX_TYPE_CLEARCOAT].IsValid() ||
                        MyMaterial.m_textures[TEX_TYPE_CLEARCOAT_NORMAL].IsValid() ||
                        MyMaterial.m_textures[TEX_TYPE_CLEARCOAT_ROUGHNESS].IsValid();

    float ClearCoatFactor = 0.0f;
    if (pMaterial->Get(AI_MATKEY_CLEARCOAT_FACTOR, ClearCoatFactor) == AI_SUCCESS) {
        MyMaterial.m_clearCoatTransmissionThickness.x = ClearCoatFactor;
        UseClearCoat = true;
    }

    float ClearCoatRoughnessFactor = 0.0f;
    if (pMaterial->Get(AI_MATKEY_CLEARCOAT_ROUGHNESS_FACTOR, ClearCoatRoughnessFactor) == AI_SUCCESS) {
        MyMaterial.m_clearCoatTransmissionThickness.y = ClearCoatRoughnessFactor;
        UseClearCoat = true;
    }

    if (UseClearCoat) {
        MyMaterial.m_materialType |= MaterialType_ClearCoat;
    }
}


void AssimpModelLoader::LoadColor(const aiMaterial* pMaterial, Vector4f& Color,
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


void AssimpModelLoader::LoadMeshBones(std::vector<SkinnedVertex>& SkinnedVertices, uint MeshIndex, const aiMesh* pMesh, uint VertexOffset)
{
    // printf("Loading mesh bones %d\n", MeshIndex);
    for (uint i = 0; i < pMesh->mNumBones; i++) {
        // printf("Bone %d %s\n", i, pMesh->mBones[i]->mName.C_Str());
        LoadSingleBone(SkinnedVertices, pMesh->mBones[i], VertexOffset);
    }

    int NumBones = (int)m_pModelData->m_boneInfo.size();
    if (NumBones > MAX_BONES) {
        printf("The number of bones in the model (%d) is larger than the maximum supported (%d)\n", NumBones, MAX_BONES);
        printf("Make sure to increase the macro MAX_BONES in the C++ header as well as in the shader to the same value\n");
        assert(0);
    }
}


static int GetNodeByBone(const aiBone* pBone, const ModelData* pModelData)
{
    std::string NodeName(pBone->mName.C_Str());

    auto NodeIter = pModelData->m_nodeMap.find(NodeName);

    if (NodeIter == pModelData->m_nodeMap.end()) {
        printf("Cannot find bone %s in the hierarchy\n", NodeName.c_str());
        assert(0);
    }

    int NodeIndex = NodeIter->second;

    return NodeIndex;
}



void AssimpModelLoader::LoadSingleBone(std::vector<SkinnedVertex>& SkinnedVertices, const aiBone* pBone, uint VertexOffset)
{
    int BoneId = GetBoneId(pBone);

    if (BoneId == m_pModelData->m_boneInfo.size()) {
        BoneInfo bi(pBone->mOffsetMatrix);
        // bi.OffsetMatrix.Print();
        m_pModelData->m_boneInfo.push_back(bi);
    }

    int NodeIndex = GetNodeByBone(pBone, m_pModelData);

    SkelNode& Node = m_pModelData->m_nodes[NodeIndex];
    Node.m_boneIndex = BoneId;

    uint NumOutOfRange = 0;

    for (uint i = 0; i < pBone->mNumWeights; i++) {
        const aiVertexWeight& vw = pBone->mWeights[i];

        // mVertexId is relative to the mesh. VertexOffset is where the mesh starts in SkinnedVertices.
        uint VertexID = VertexOffset + vw.mVertexId;

        if (VertexID < SkinnedVertices.size()) {
            SkinnedVertices[VertexID].Bones.AddBoneData(BoneId, vw.mWeight);
        } else {
            NumOutOfRange++;
        }
    }

    if (NumOutOfRange > 0) {
        printf("Warning! bone '%s': %d of %d vertex weights are out of range and were ignored\n",
               pBone->mName.C_Str(), NumOutOfRange, pBone->mNumWeights);
    }

    MarkRequiredNodesForBone(NodeIndex);
}


void AssimpModelLoader::MarkRequiredNodesForBone(int NodeIndex)
{
    do {
        SkelNode& Node = m_pModelData->m_nodes[NodeIndex];

        if (Node.m_isRequired) {
            break;  // early exit - no need to traverse up the hierarchy if this node is already marked as required
        }

        Node.m_isRequired = true;

        int ParentIndex = Node.m_parentIndex;

        if (ParentIndex == -1) {
            break;
        } else {
            NodeIndex = ParentIndex;
        }
    } while (true);
}


int AssimpModelLoader::GetBoneId(const aiBone* pBone)
{
    int BoneIndex = 0;
    std::string BoneName(pBone->mName.C_Str());

    if (m_pModelData->m_boneNameToIndexMap.find(BoneName) == m_pModelData->m_boneNameToIndexMap.end()) {
        // Allocate an index for a new bone
        BoneIndex = (int)m_pModelData->m_boneNameToIndexMap.size();
        m_pModelData->m_boneNameToIndexMap[BoneName] = BoneIndex;
    } else {
        BoneIndex = m_pModelData->m_boneNameToIndexMap[BoneName];
    }

    return BoneIndex;
}


static void FlattenNodeHierarchy(const aiNode* pNode, int ParentIndex, 
                                 const Matrix4f& ParentTransform,
                                 std::vector<SkelNode>& Out, 
                                 std::unordered_map<std::string, int>& NodeNameToIndexMap,
                                 std::vector<MeshInstance>& Instances)
{
    if (pNode->mName.length == 0) {
        printf("Warning! Node with empty name found, parent %d,skipping\n", ParentIndex);
        return;
    }

    int NodeIndex = (int)Out.size();

    SkelNode node;
    node.m_name = pNode->mName.C_Str();
    node.m_parentIndex = ParentIndex;
    node.m_localTransform = Matrix4f(pNode->mTransformation);
    node.m_bindPoseGlobalTransform = ParentTransform * node.m_localTransform;
    node.m_meshes.resize(pNode->mNumMeshes);
    if (pNode->mNumMeshes > 0) {
        node.m_firstInstance = (int)Instances.size();
    }
    for (unsigned int i = 0; i < pNode->mNumMeshes; i++) {
        int MeshIndex = pNode->mMeshes[i];
        node.m_meshes[i] = MeshIndex;
        MeshInstance mi;
        mi.m_meshIndex = MeshIndex;
        mi.m_nodeIndex = NodeIndex;
        mi.m_worldTransform = node.m_bindPoseGlobalTransform;
        Instances.push_back(mi);
    }
    node.m_numInstances = pNode->mNumMeshes;

    Out.push_back(node);

    auto it = NodeNameToIndexMap.find(node.m_name);

    if (it == NodeNameToIndexMap.end()) {
        NodeNameToIndexMap[node.m_name] = NodeIndex;
    } else {
        printf("Warning! Duplicate node name '%s' (nodes %d and %d), using the first\n",
               node.m_name.c_str(), it->second, NodeIndex);
    }

   // printf("Adding node '%s', index %d with parent index %d\n", node.m_name.c_str(), NodeIndex, ParentIndex);

    for (unsigned int i = 0; i < pNode->mNumChildren; i++) {
        FlattenNodeHierarchy(pNode->mChildren[i], NodeIndex, node.m_bindPoseGlobalTransform, Out, NodeNameToIndexMap, Instances);
    }
}


void AssimpModelLoader::InitSkeletonHierarchy()
{
    printf("\n*** Initializing skeleton ***\n");
    Matrix4f Identity;
    Identity.InitIdentity();
    FlattenNodeHierarchy(m_pScene->mRootNode, -1, Identity, 
                         m_pModelData->m_nodes, m_pModelData->m_nodeMap, m_pModelData->m_instances);
}   


void AssimpModelLoader::InitAnimations()
{
    printf("\n*** Initializing animations ***\n");
    printf("Loading %d animations\n", m_pScene->mNumAnimations);
    m_pModelData->m_animations.resize(m_pScene->mNumAnimations);

    for (unsigned int i = 0; i < m_pScene->mNumAnimations; i++) {
        InitSingleAnimation(i);
    }
}


void AssimpModelLoader::InitSingleAnimation(int Index)
{
    const aiAnimation* pAnim = m_pScene->mAnimations[Index];
    
    printf("Loading Animation: '%s'\n", pAnim->mName.C_Str());

    AnimationClip& MyAnim = m_pModelData->m_animations[Index];

    MyAnim.m_name = pAnim->mName.C_Str();
    MyAnim.m_durationTicks = (float)pAnim->mDuration;
    MyAnim.m_ticksPerSecond = (float)pAnim->mTicksPerSecond;

    printf("Animation duration: %f ticks, ticks per second: %f\n", MyAnim.m_durationTicks, MyAnim.m_ticksPerSecond);

    MyAnim.m_nodeToChannel.assign(m_pModelData->m_nodes.size(), -1);

    MyAnim.m_channels.resize(pAnim->mNumChannels);

    for (unsigned int i = 0; i < pAnim->mNumChannels; i++) {
        InitSingleNodeAnimation(MyAnim, i, MyAnim.m_channels[i], pAnim->mChannels[i]);
    }
}


void AssimpModelLoader::InitSingleNodeAnimation(AnimationClip& MyAnim, int ChannelIndex, AnimChannel& MyChannel, const aiNodeAnim* pNodeAnim)
{
    std::string NodeName(pNodeAnim->mNodeName.C_Str());

    auto it = m_pModelData->m_nodeMap.find(NodeName);

    if (it == m_pModelData->m_nodeMap.end()) {
        printf("Warning! Animation channel for node '%s' not found in the hierarchy\n", NodeName.c_str());
        exit(0);
    }

    MyChannel.m_nodeName = NodeName;

    int NodeIndex = it->second;
    MyAnim.m_nodeToChannel[NodeIndex] = ChannelIndex;

    MyChannel.m_positions.resize(pNodeAnim->mNumPositionKeys);
    for (unsigned int i = 0; i < pNodeAnim->mNumPositionKeys; i++) {
        const aiVectorKey& key = pNodeAnim->mPositionKeys[i];
        MyChannel.m_positions[i].m_time = (float)key.mTime;
        MyChannel.m_positions[i].m_value = glm::vec3(key.mValue.x, key.mValue.y, key.mValue.z);
    }

    MyChannel.m_rotations.resize(pNodeAnim->mNumRotationKeys);    
    for (unsigned int i = 0; i < pNodeAnim->mNumRotationKeys; i++) {
        const aiQuatKey& key = pNodeAnim->mRotationKeys[i];
        MyChannel.m_rotations[i].m_time = (float)key.mTime;
        MyChannel.m_rotations[i].m_value = glm::quat(key.mValue.w, key.mValue.x, key.mValue.y, key.mValue.z);
    }

    MyChannel.m_scalings.resize(pNodeAnim->mNumScalingKeys);
    for (unsigned int i = 0; i < pNodeAnim->mNumScalingKeys; i++) {
        const aiVectorKey& key = pNodeAnim->mScalingKeys[i];
        MyChannel.m_scalings[i].m_time = (float)key.mTime;
        MyChannel.m_scalings[i].m_value = glm::vec3(key.mValue.x, key.mValue.y, key.mValue.z);
    }
}