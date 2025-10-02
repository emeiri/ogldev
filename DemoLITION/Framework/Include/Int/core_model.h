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

#pragma once

#include <map>
#include <vector>

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>       // Output data structure
#include <assimp/postprocess.h> // Post processing flags

#include "Int/core_material.h"
#include "ogldev_glm_camera.h"
#include "demolition_lights.h"
#include "demolition_model.h"
#include "Int/core_mesh.h"
#include "GL/gl_basic_mesh_entry.h"

void GetVertexSizesInBytes(size_t& VertexSize, size_t& SkinnedVertexSize);


class DemolitionRenderCallbacks
{
public:
    virtual void DrawStart_CB(uint DrawIndex) = 0;

    virtual void ControlSpecularExponent_CB(bool IsEnabled) = 0;

    virtual void SetMaterial_CB(const CoreMaterial& material) = 0;

    virtual void SetWorldMatrix_CB(const Matrix4f& World) = 0;
};


class CoreRenderingSystem;

class CoreModel : public Model
{
public:
    CoreModel() {}

    CoreModel(CoreRenderingSystem* pCoreRenderingSystem) { m_pCoreRenderingSystem = pCoreRenderingSystem; }

    bool IsPBR() const { return m_shadingModel == aiShadingMode_PBR_BRDF; }

    void DestroyModel();

    bool LoadAssimpModel(const std::string& Filename);

    const std::vector<GLMCameraFirstPerson>& GetCameras() const { return m_cameras; }

    uint NumBones() const { return (uint)m_BoneNameToIndexMap.size(); }

    // This is the main function to drive the animation. It receives the animation time
    // in seconds and a reference to a vector of transformation matrices (one matrix per bone).
    // It calculates the current transformation for each bone according to the current time
    // and updates the corresponding matrix in the vector. This must then be updated in the VS
    // to be accumulated for the final local position (see skinning.vs). The animation index
    // is an optional param which selects one of the animations.
    void GetBoneTransforms(float AnimationTimeSec, vector<Matrix4f>& Transforms, unsigned int AnimationIndex = 0);

    // Same as above but this one blends two animations together based on a blending factor
    void GetBoneTransformsBlended(float AnimationTimeSec,
                                  vector<Matrix4f>& Transforms,
                                  unsigned int StartAnimIndex,
                                  unsigned int EndAnimIndex,
                                  float BlendFactor);

    const std::vector<DirectionalLight>& GetDirLights() const { return m_dirLights; }
    const std::vector<SpotLight>& GetSpotLights() const { return m_spotLights; }
    const std::vector<PointLight>& GetPointLights() const { return m_pointLights; }

    void SetTextureScale(float Scale) { m_textureScale = Scale; }

    bool IsAnimated() const;

    const CoreMaterial* GetMaterialForMesh(int MeshIndex) const;

    virtual void SetColorTexture(int TextureHandle) { assert(0); }

    virtual void SetNormalMap(int TextureHandle) { assert(0); }

    virtual void SetHeightMap(int TextureHandle) { assert(0); }

    virtual void SetAmbientOcclusionMap(int TextureHandle) { assert(0); }

    virtual int GetMeshIndex(const std::string& Name);

    virtual size_t GetNumMeshes() const { return m_Meshes.size(); }

    static void GetVertexSizesInBytes(size_t& VertexSize, size_t& SkinnedVertexSize);

protected:

    virtual void AllocBuffers() = 0;

    virtual Texture* AllocTexture2D() = 0;

    virtual void DestroyTexture(Texture* pTexture) = 0;

    enum BUFFER_TYPE {
        INDEX_BUFFER = 0,
        VERTEX_BUFFER = 1,
        WVP_MAT_BUFFER = 2,  // required only for instancing
        WORLD_MAT_BUFFER = 3,  // required only for instancing
        NUM_BUFFERS = 4
    };


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

#define MAX_NUM_BONES_PER_VERTEX 4

    struct VertexBoneData
    {
        uint BoneIDs[MAX_NUM_BONES_PER_VERTEX] = { 0 };
        float Weights[MAX_NUM_BONES_PER_VERTEX] = { 0.0f };
        int index = 0;  // slot for the next update

        VertexBoneData() { }

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

    virtual void InitGeometryPost() = 0;

    std::vector<BasicMeshEntry> m_Meshes;
    std::vector<CoreMaterial> m_Materials;

    // Temporary space for vertex stuff before we load them into the GPU
    vector<uint> m_Indices;

    CoreRenderingSystem* m_pCoreRenderingSystem = NULL;

private:

    template<typename VertexType>
    void ReserveSpace(std::vector<VertexType>& Vertices, uint NumVertices, uint NumIndices);

    template<typename VertexType>
    void InitSingleMesh(vector<VertexType>& Vertices, uint MeshIndex, const aiMesh* paiMesh);

    template<typename VertexType>
    void InitSingleMeshOpt(vector<VertexType>& Vertices, uint MeshIndex, const aiMesh* paiMesh);

    virtual void PopulateBuffersSkinned(vector<SkinnedVertex>& Vertices) = 0;

    virtual void PopulateBuffers(vector<Vertex>& Vertices) = 0;

    uint CountValidFaces(const aiMesh& Mesh);

    bool InitFromScene(const aiScene* pScene, const std::string& Filename);

    bool InitGeometry(const aiScene* pScene, const string& Filename);

    void InitBuffers(const aiScene* pScene, unsigned int NumVertices, unsigned int NumIndices);

    template<typename VertexType>
    void InitGeometryInternal(std::vector<VertexType>& Vertices, int NumVertices, int NumIndices);

    void InitLights(const aiScene* pScene);

    void InitSingleLight(const aiScene* pScene, const aiLight& light);

    void InitDirectionalLight(const aiScene* pScene, const aiLight& light);

    void InitPointLight(const aiScene* pScene, const aiLight& light);

    void InitSpotLight(const aiScene* pScene, const aiLight& light);

    void CountVerticesAndIndices(const aiScene* pScene, uint& NumVertices, uint& NumIndices);

    template<typename VertexType>
    void InitAllMeshes(const aiScene* pScene, std::vector<VertexType>& Vertices);

    template<typename VertexType>
    void OptimizeMesh(int MeshIndex, std::vector<uint>& Indices, std::vector<VertexType>& Vertices, std::vector<VertexType>& AllVertices);

    void CalculateMeshTransformations(const aiScene* pScene);
    void TraverseNodeHierarchy(Matrix4f ParentTransformation, aiNode* pNode);

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
    void LoadTextureEmbedded(const aiTexture* paiTexture, int MaterialIndex, TEXTURE_TYPE MyType, bool IsSRGB);
    void LoadTextureFromFile(const string& dir, const aiString& Path, int MaterialIndex, TEXTURE_TYPE MyType, bool IsSRGB);

    void LoadColors(const aiMaterial* pMaterial, int index);
    void ProcessClearCoat(int index, const aiMaterial* pMaterial, CoreMaterial& material);
    void LoadColor(const aiMaterial* pMaterial, Vector4f& Color, const char* pAiMatKey, int AiMatType, int AiMatIdx, const char* pName);
    void DetectShadingModel(const aiMaterial* pMaterial);

    void InitCameras(const aiScene* pScene);

    void InitSingleCamera(int Index, const aiScene* pScene);

    const aiScene* m_pScene = NULL;

    aiShadingMode m_shadingModel = aiShadingMode_NoShading;
    Matrix4f m_GlobalInverseTransform;

    Assimp::Importer m_Importer;

    std::vector<GLMCameraFirstPerson> m_cameras;
    std::vector<DirectionalLight> m_dirLights;
    std::vector<PointLight> m_pointLights;
    std::vector<SpotLight> m_spotLights;
    float m_textureScale = 1.0f;
    u32 m_numAnimations = 0;

    Vector3f m_minPos = Vector3f(FLT_MAX, FLT_MAX, FLT_MAX);
    Vector3f m_maxPos = Vector3f(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    std::map<std::string, int> m_meshNameToMeshIndex;

    /////////////////////////////////////
	// Skeletal animation stuff
    /////////////////////////////////////

    void LoadMeshBones(vector<SkinnedVertex>& SkinnedVertices, uint MeshIndex, const aiMesh* paiMesh);
    void LoadSingleBone(vector<SkinnedVertex>& SkinnedVertices, uint MeshIndex, const aiBone* pBone);
    int GetBoneId(const aiBone* pBone);
    void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
    void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
    void CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
    uint FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim);
    uint FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim);
    uint FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim);
    const aiNodeAnim* FindNodeAnim(const aiAnimation& Animation, const string& NodeName);
    void ReadNodeHierarchy(float AnimationTime, const aiNode* pNode, const Matrix4f& ParentTransform, const aiAnimation& Animation);
    void ReadNodeHierarchyBlended(float StartAnimationTimeTicksm, float EndAnimationTimeTicks, const aiNode* pNode, const Matrix4f& ParentTransform,
                                  const aiAnimation& StartAnimation, const aiAnimation& EndAnimation, float BlendFactor);
    void MarkRequiredNodesForBone(const aiBone* pBone);
    void InitializeRequiredNodeMap(const aiNode* pNode);
    float CalcAnimationTimeTicks(float TimeInSeconds, unsigned int AnimationIndex);

    struct LocalTransform {
        aiVector3D Scaling;
        aiQuaternion Rotation;
        aiVector3D Translation;
    };

    void CalcLocalTransform(LocalTransform& Transform, float AnimationTimeTicks, const aiNodeAnim* pNodeAnim);

    map<string,uint> m_BoneNameToIndexMap;

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

    vector<BoneInfo> m_BoneInfo;

    struct NodeInfo {

        NodeInfo() {}

        NodeInfo(const aiNode* n) { pNode = n;}

        const aiNode* pNode = NULL;
        bool isRequired = false;
    };

    map<string,NodeInfo> m_requiredNodeMap;
};

