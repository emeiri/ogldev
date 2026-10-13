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

#pragma once

#include <map>
#include <vector>
#include <unordered_map>

#include "ogldev_math_3d.h"
#include "GL/gl_basic_mesh_entry.h"
#include "demolition_lights.h"
#include "demolition_scene.h"
#include "ogldev_glm_camera.h"

struct aiNode;
struct aiBone;
struct aiMesh;
struct aiLight;
struct aiScene;
namespace Assimp {
    class Importer;
}

struct Vertex {
    Vector3f Position;
    Vector2f TexCoords0;
    Vector2f TexCoords1;
    Vector3f Normal;
    Vector3f Tangent;
    Vector3f Bitangent;
    Vector4f Color;

    void Print()
    {
        Position.Print();
        TexCoords0.Print();
        TexCoords1.Print();
        Normal.Print();
        Tangent.Print();
        Bitangent.Print();
        Color.Print();
    }
};

struct NodeInfo {

    NodeInfo() {}

    NodeInfo(const aiNode* n) { pNode = n; }

    const aiNode* pNode = NULL;
    bool isRequired = false;
};

struct BoneInfo
{
    Matrix4f OffsetMatrix;
    Matrix4f FinalTransformation;

    BoneInfo(const Matrix4f& Offset)
    {
        OffsetMatrix = Offset;
        FinalTransformation.SetZero();
    }
};



#define MAX_NUM_BONES_PER_VERTEX 4

struct VertexBoneData
{
    uint BoneIDs[MAX_NUM_BONES_PER_VERTEX] = { 0 };
    float Weights[MAX_NUM_BONES_PER_VERTEX] = { 0.0f };
    int index = 0;  // slot for the next update

    VertexBoneData() {}

    void AddBoneData(uint BoneID, float Weight)
    {
        for (int i = 0; i < index; i++) {
            if (BoneIDs[i] == BoneID) {
                //  printf("bone %d already found at index %d old weight %f new weight %f\n", BoneID, i, Weights[i], Weight);
                return;
            }
        }

        // The iClone 7 Raptoid Mascot (https://sketchfab.com/3d-models/iclone-7-raptoid-mascot-free-download-56a3e10a73924843949ae7a9800c97c7)
        // has a problem of zero weights causing an overflow and the assertion below. This fixes it.
        if (Weight == 0.0f) {
            return;
        }

        // printf("Adding bone %d weight %f at index %i\n", BoneID, Weight, index);

        if (index == MAX_NUM_BONES_PER_VERTEX) {
            return;
            assert(0);
        }

        BoneIDs[index] = BoneID;
        Weights[index] = Weight;

        index++;
    }
};


struct SkinnedVertex {
    Vector3f Position;
    Vector2f TexCoords0;
    Vector2f TexCoords1;
    Vector3f Normal;
    Vector3f Tangent;
    Vector3f Bitangent;
    Vector4f Color;
    VertexBoneData Bones;
};

struct TextureSource {
    std::string Path;                  // resolved full path, or empty
    std::vector<u8> EmbeddedBytes;     // non-empty for embedded textures
    bool IsSRGB = false;
    bool IsValid() const { return !Path.empty() || !EmbeddedBytes.empty(); }
};


struct MaterialData {
    std::string m_name;
    bool m_isPBR = false;
    int m_materialType = MaterialType_Invalid;
    Vector4f AmbientColor = Vector4f(0.0f, 0.0f, 0.0f, 0.0f);
    Vector4f DiffuseColor = Vector4f(0.0f, 0.0f, 0.0f, 0.0f);
    Vector4f SpecularColor = Vector4f(0.0f, 0.0f, 0.0f, 0.0f);
    Vector4f BaseColor = Vector4f(0.0f, 0.0f, 0.0f, 0.0f);
    Vector4f EmissiveColor = Vector4f(0.0f, 0.0f, 0.0f, 0.0f);
    Vector4f MetallicRoughnessNormalOcclusion = Vector4f(1.0f);
    Vector4f ClearCoatTransmissionThickness = Vector4f(1.0f);
    float m_transparencyFactor = 1.0f;
    float m_alphaTest = 0.0f;
    float m_ior = 1.5f;
    TextureSource m_textures[TEX_TYPE_NUM];
};


struct SkelNode {
    std::string m_name;
    int m_parentIndex = -1;
    Matrix4f m_localTransform;
    Matrix4f m_globalTransform;
    std::vector<int> m_meshes;
    int m_firstInstance = -1;
    int m_numInstances = 0;
    bool m_isRequired = false;
};


struct MeshInstance {
    u32      m_meshIndex;   // geometry + material
    int      m_nodeIndex;   // flat node it came from
    Matrix4f m_worldTransform;
};


struct ModelData {
    std::vector<BasicMeshEntry>     m_meshes;
    std::vector<Vertex>        m_vertices;          // used when !IsSkinned()
    std::vector<SkinnedVertex> m_skinnedVertices;   // used when IsSkinned()
    std::vector<u32>           m_indices;
    std::vector<MeshInstance> m_instances;
    std::vector<MaterialData>  m_materials;
    std::vector<DirectionalLight> m_dirLights;
    std::vector<PointLight>       m_pointLights;
    std::vector<SpotLight>        m_spotLights;
    std::vector<GLMCameraFirstPerson> m_cameras;
    MeshDims m_dims;
    Matrix4f m_globalInverseTransform;
    u32 m_numAnimations = 0;
    vector<BoneInfo> m_boneInfo;
    std::vector<SkelNode> m_nodes;
    std::unordered_map<std::string, int> m_nodeMap;
    std::map<std::string, uint> m_boneNameToIndexMap;
    std::map<std::string, NodeInfo> m_requiredNodeMap;
    

    // bool IsSkinned() const { return Skel != nullptr; }   // keyed on bones, not animation count
    bool IsPBR() const;

    size_t NumVertices() const {
        size_t NumVertices = 0;

        if (m_numAnimations > 0) {
            NumVertices = m_skinnedVertices.size();
        } else {
            NumVertices = m_vertices.size();
        }

        return NumVertices;
    }
};


class AssimpModelLoader {
public:
    // Returns null on failure; GetError() says why. All Assimp types stay inside the .cpp.
    AssimpModelLoader() = default;
    void Load(const std::string& Filename, const ModelLoadFlags& Flags, ModelData& OutModelData, int WindowWidth, int WindowHeight);
    const std::string& GetError() const { return m_error; }

private:
    void InitFromScene();
    void InitMaterials();
    void InitGeometry();
    void CountVerticesAndIndices(uint& NumVertices, uint& NumIndices);
    void InitMeshes(unsigned int NumVertices, unsigned int NumIndices);
    template<typename VertexType>
    void InitAllMeshes(std::vector<VertexType>& Vertices);
    template<typename VertexType>
    void InitSingleMesh(std::vector<VertexType>& Vertices, uint MeshIndex, const aiMesh* paiMesh);
    template<typename VertexType>
    void InitSingleMeshOpt(std::vector<VertexType>& AllVertices, uint MeshIndex, const aiMesh* paiMesh);
    template<typename VertexType>
    void OptimizeMesh(int MeshIndex, std::vector<uint>& Indices,
        std::vector<VertexType>& Vertices, std::vector<VertexType>& AllVertices);
    
    void InitCameras();
    void InitSingleCamera(int Index);
    
    void InitializeRequiredNodeMap(const aiNode* pNode);
    void LoadMeshBones(std::vector<SkinnedVertex>& SkinnedVertices, uint MeshIndex, const aiMesh* pMesh);
    void LoadSingleBone(std::vector<SkinnedVertex>& SkinnedVertices, uint MeshIndex, const aiBone* pBone);
    int GetBoneId(const aiBone* pBone);
    void MarkRequiredNodesForBone(const aiBone* pBone);
    void TraverseNodeHierarchy(const Matrix4f& ParentTransformation, aiNode* pNode);

    // Lights
    void InitLights();
    void InitSingleLight(const aiLight& light);
    void InitDirectionalLight(const aiLight& light);
    void InitPointLight(const aiLight& light);
    void InitSpotLight(const aiLight& light);

    // Materials
    bool InitMaterials(const aiScene* pScene, const std::string& Filename);
    void LoadTextures(const string& Dir, const aiMaterial* pMaterial, int index);
    void LoadDiffuseTexture(const string& Dir, const aiMaterial* pMaterial, int index);
    void LoadSpecularTexture(const string& Dir, const aiMaterial* pMaterial, int index);
    void LoadNormalTexture(const string& Dir, const aiMaterial* pMaterial, int MaterialIndex);
    void LoadMetalnessTexture(const string& Dir, const aiMaterial* pMaterial, int MaterialIndex);
    void LoadEmissiveTexture(const string& Dir, const aiMaterial* pMaterial, int MaterialIndex);
    void LoadEmissionColorTexture(const string& Dir, const aiMaterial* pMaterial, int MaterialIndex);
    void LoadNormalCameraTexture(const string& Dir, const aiMaterial* pMaterial, int MaterialIndex);
    void LoadRoughnessTexture(const string& Dir, const aiMaterial* pMaterial, int MaterialIndex);
    void LoadAmbientOcclusionTexture(const string& Dir, const aiMaterial* pMaterial, int MaterialIndex);
    void LoadClearCoatTexture(const string& Dir, const aiMaterial* pMaterial, int MaterialIndex);
    void LoadClearCoatRoughnessTexture(const string& Dir, const aiMaterial* pMaterial, int MaterialIndex);
    void LoadClearCoatNormalTexture(const string& Dir, const aiMaterial* pMaterial, int MaterialIndex);
    void LoadTexture(const string& Dir, const aiMaterial* pMaterial, int MaterialIndex, aiTextureType AssimpType, int AssimpTexIndex, TEXTURE_TYPE MyType);

    // Colors
    void LoadColors(const aiMaterial* pMaterial, MaterialData& MyMaterial);
    void ProcessClearCoat(const aiMaterial* pMaterial, MaterialData& MyMaterial);
    void LoadColor(const aiMaterial* pMaterial, Vector4f& Color, const char* pAiMatKey, int AiMatType, int AiMatIdx, const char* pName);

    // Skeleton
    void InitSkeletonHierarchy();

    // Each of these is a private free function or small helper class in the .cpp:
    //   MeshBuilder      - ExtractVertex<VertexType>(), indices, bounds, optional meshopt
    //   MaterialBuilder  - table-driven texture slots + LoadColors + ProcessClearCoat
    //   SceneExtractor   - node transforms, lights, cameras
    //   SkeletonBuilder  - bones, node tree, animation channels -> Skeleton
    std::string m_error;
    std::string m_filename;
    const aiScene* m_pScene = NULL;
    ModelData* m_pModelData = NULL;
    Assimp::Importer* m_pImporter = NULL;
    u32 m_numAnimations = 0;
    bool m_flipTexCoordV = false;
    int m_windowWidth = 0;
    int m_windowHeight = 0;
};

