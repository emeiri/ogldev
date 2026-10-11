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

//#define DEBUG_COLORS
//#define DEBUG_MATERIALS
//#define DEBUG_TEXTURES
//#define DEBUG_BONES
#define DEBUG_SCENE_HIERARCHY

// config flags
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

static uint CountValidFaces(const aiMesh& Mesh);
static bool GetFullTransformation(const aiNode* pRootNode, const char* pName, Matrix4f& Transformation);

int traverse(int depth, aiNode* pNode);
    
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
    int WindowWidth, WindowHeight;
#ifdef OGLDEV_VULKAN
    GetWindowSize(WindowWidth, WindowHeight);
#else
    m_pCoreRenderingSystem->GetWindowSize(WindowWidth, WindowHeight);
#endif

    AssimpModelLoader Loader;
    Loader.Load(Filename, Flags, m_modelData, WindowWidth, WindowHeight);

#ifndef OGLDEV_VULKAN
    std::string FileExt = Filename.substr(Filename.find_last_of(".") + 1);
    m_flipTexCoordV = (FileExt == "glb" || FileExt == "gltf");
#endif

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
        Ret = InitFromScene(m_pScene, Filename);
    }
    else {
        printf("Error parsing '%s': '%s'\n", Filename.c_str(), m_Importer.GetErrorString());
    }

    return Ret;
}


bool CoreModel::InitFromScene(const aiScene* pScene, const std::string& Filename)
{
    if (!InitGeometry(pScene, Filename)) {
        return false;
    }    

    return true;
}


bool CoreModel::InitGeometry(const aiScene* pScene, const std::string& Filename)
{
    printf("\n*** Initializing geometry ***\n");
    
    m_Meshes = m_modelData.m_meshes;
    m_Materials.resize(pScene->mNumMaterials);

    unsigned int NumVertices = (unsigned int)m_modelData.NumVertices();
    unsigned int NumIndices = (unsigned int)m_modelData.m_indices.size();

    if (!InitMaterials(pScene, Filename)) {
        return false;
    }

    InitGPUBuffers(pScene, NumVertices, NumIndices);

    CalculateMeshTransformations(pScene);

    InitGeometryPost();

#ifdef OGLDEV_VULKAN
    return true;
#else
    return GLCheckError();
#endif
}


void CoreModel::InitGPUBuffers(const aiScene* pScene, unsigned int NumVertices, unsigned int NumIndices)
{
    printf("Num animations %d\n", pScene->mNumAnimations);

    if (pScene->mNumAnimations > 0) {
        PopulateBuffersSkinned(m_modelData.m_skinnedVertices);
    }
    else {
        PopulateBuffers(m_modelData.m_vertices);
    }
}


void CoreModel::CalculateMeshTransformations(const aiScene* pScene)
{
    printf("----------------------------------------\n");
    printf("Calculating mesh transformations\n");
    Matrix4f Transformation;
    Transformation.InitIdentity();

    TraverseNodeHierarchy(Transformation, pScene->mRootNode);
    
    for (u32 SubmeshIndex = 0; SubmeshIndex < m_Meshes.size(); SubmeshIndex++) {
        // The matrix is stored as row major in the file but glm expects column major by default.
        const glm::mat4& MeshTransformGLM = m_transformationsGLM[SubmeshIndex];
        glm::vec3 MeshPos = glm::vec3(MeshTransformGLM[3]);
        m_Meshes[SubmeshIndex].Dims.Pos = MeshPos;
    }
}


void CoreModel::TraverseNodeHierarchy(const Matrix4f& ParentTransformation, aiNode* pNode)
{
#ifdef DEBUG_SCENE_HIERARCHY
    printf("Traversing node '%s'\n", pNode->mName.C_Str());
#endif
    Matrix4f LocalTransform(pNode->mTransformation);    

    Matrix4f GlobalTransform = ParentTransformation * LocalTransform;

#ifdef DEBUG_SCENE_HIERARCHY
    printf("Global transformation:\n");
    GlobalTransform.Print();
#endif

   // glm::mat4 CombinedTransformationGLM = glm::make_mat4(CombinedTransformation.data());
   // CombinedTransformationGLM = glm::transpose(CombinedTransformationGLM);
  
    if (pNode->mNumMeshes > 0) {
#ifdef DEBUG_SCENE_HIERARCHY
        printf("Num meshes: %d\n", pNode->mNumMeshes);
#endif
        for (int i = 0; i < (int)pNode->mNumMeshes; i++) {
            int MeshIndex = pNode->mMeshes[i];
            const BasicMeshEntry& MeshEntry = m_Meshes[MeshIndex];
            int CurInstanceID = (int)m_transformations.size();
            m_transformations.push_back(GlobalTransform);

            glm::mat4 MeshTransform = glm::make_mat4(GlobalTransform.data());
            // The matrix is stored as row major in the file but glm expects column major by default.
            glm::mat4 MeshTransformGLM = glm::transpose(MeshTransform);

            glm::mat4 ZFlip = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, -1.0f));
            glm::mat4 LeftHandedMatrixGLM = ZFlip * MeshTransformGLM * ZFlip;

            m_transformationsGLM.push_back(LeftHandedMatrixGLM);

#ifdef DEBUG_SCENE_HIERARCHY
            printf("%d ", MeshIndex);
#endif
            m_nodeMap[pNode->mName.C_Str()] = MeshIndex;
            m_Meshes[MeshIndex].TransformationDeprecated = GlobalTransform;
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
        TraverseNodeHierarchy(GlobalTransform, pNode->mChildren[i]);
    }
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
    LoadTexture(Dir, pMaterial, MaterialIndex, aiTextureType_CLEARCOAT, 1, TEX_TYPE_CLEARCOAT_ROUGHNESS);
}


void CoreModel::LoadClearCoatNormalTexture(const std::string& Dir, const aiMaterial* pMaterial, int MaterialIndex)
{
    LoadTexture(Dir, pMaterial, MaterialIndex, aiTextureType_CLEARCOAT, 2, TEX_TYPE_CLEARCOAT_NORMAL);
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


glm::mat4& CoreModel::GetNodeTransformationGLM(const char* pNodeName)
{
    std::map<std::string, int>::iterator it = m_nodeMap.find(pNodeName);

    if (it == m_nodeMap.end()) {
        OGLDEV_ERROR("Cannot find node %s in the hierarchy\n", pNodeName);
        assert(0);
    }

    return m_transformationsGLM[it->second];
}


void CoreModel::SetNodePositionXZ(const char* pNodeName, float x, float z)
{
    glm::mat4& Transformation = GetNodeTransformationGLM(pNodeName);
    Transformation[3][0] = x;
    Transformation[3][2] = z;
}

//
// Skeletal Animation Support
//



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

    if (m_modelData.m_boneNameToIndexMap.find(NodeName) != m_modelData.m_boneNameToIndexMap.end()) {
        uint BoneIndex = m_modelData.m_boneNameToIndexMap[NodeName];
        m_modelData.m_boneInfo[BoneIndex].FinalTransformation = m_modelData.m_globalInverseTransform * GlobalTransformation * m_modelData.m_boneInfo[BoneIndex].OffsetMatrix;
    }

    for (uint i = 0 ; i < pNode->mNumChildren ; i++) {
        std::string ChildName(pNode->mChildren[i]->mName.data);

        std::map<std::string,NodeInfo>::iterator it = m_modelData.m_requiredNodeMap.find(ChildName);

        if (it == m_modelData.m_requiredNodeMap.end()) {
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

    if (m_modelData.m_boneNameToIndexMap.find(NodeName) != m_modelData.m_boneNameToIndexMap.end()) {
        uint BoneIndex = m_modelData.m_boneNameToIndexMap[NodeName];
        m_modelData.m_boneInfo[BoneIndex].FinalTransformation = m_modelData.m_globalInverseTransform * GlobalTransformation * m_modelData.m_boneInfo[BoneIndex].OffsetMatrix;
    }

    for (uint i = 0; i < pNode->mNumChildren; i++) {
        std::string ChildName(pNode->mChildren[i]->mName.data);
        auto it = m_modelData.m_requiredNodeMap.find(ChildName);
        if (it == m_modelData.m_requiredNodeMap.end()) {
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
    Transforms.resize(m_modelData.m_boneInfo.size());

    for (uint i = 0 ; i < m_modelData.m_boneInfo.size() ; i++) {
        Transforms[i] = m_modelData.m_boneInfo[i].FinalTransformation;
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

    BlendedTransforms.resize(m_modelData.m_boneInfo.size());

    for (uint i = 0 ; i < m_modelData.m_boneInfo.size() ; i++) {
        BlendedTransforms[i] = m_modelData.m_boneInfo[i].FinalTransformation;
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
    bool ret = m_modelData.m_numAnimations > 0;

    if (ret && (NumBones() == 0)) {
        printf("Animations without bones? need to check this\n");
        assert(0);
    }

    return ret;
}