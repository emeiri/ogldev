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
#include <GL/glew.h>

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>       // Output data structure
#include <assimp/postprocess.h> // Post processing flags

#include "ogldev_util.h"
#include "ogldev_math_3d.h"
#include "ogldev_texture.h"
#include "ogldev_material.h"
#include "ogldev_basic_glfw_camera.h"
#include "demolition_lights.h"


#define INVALID_MATERIAL 0xFFFFFFFF

class DemolitionRenderCallbacks
{
public:
    virtual void DrawStart_CB(uint DrawIndex) = 0;

    virtual void ControlSpecularExponent_CB(bool IsEnabled) = 0;

    virtual void SetMaterial_CB(const Material& material) = 0;

    virtual void DisableDiffuseTexture_CB() = 0;

    virtual void SetWorldMatrix_CB(const Matrix4f& World) = 0;
};


class DemolitionModel 
{
public:
    DemolitionModel() {};

    ~DemolitionModel();

    bool LoadAssimpModel(const std::string& Filename, int WindowWidth, int WindowHeight);

    void Render(DemolitionRenderCallbacks* pRenderCallbacks = NULL);

    void Render(uint DrawIndex, uint PrimID);

    void Render(uint NumInstances, const Matrix4f* WVPMats, const Matrix4f* WorldMats);

    PBRMaterial& GetPBRMaterial() { return m_Materials[0].PBRmaterial; };

    void GetLeadingVertex(uint DrawIndex, uint PrimID, Vector3f& Vertex);

    const std::vector<BasicCamera>& GetCameras() const { return m_cameras; }

    uint NumBones() const
    {
        return (uint)m_BoneNameToIndexMap.size();
    }

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

private:

    void Clear();

    void RenderMesh(int MeshIndex, DemolitionRenderCallbacks* pRenderCallbacks = NULL);

    virtual void ReserveSpace(uint NumVertices, uint NumIndices);

    virtual void InitSingleMesh(const aiScene* pScene, uint MeshIndex, const aiMesh* paiMesh);

    virtual void PopulateBuffers();

    bool InitFromScene(const aiScene* pScene, const std::string& Filename, int WindowWidth, int WindowHeight);

    bool InitGeometry(const aiScene* pScene, const string& Filename);

    void InitLights(const aiScene* pScene);

    void InitSingleLight(const aiScene* pScene, const aiLight& light);

    void InitDirectionalLight(const aiScene* pScene, const aiLight& light);

    void InitPointLight(const aiScene* pScene, const aiLight& light);

    void InitSpotLight(const aiScene* pScene, const aiLight& light);

    void CountVerticesAndIndices(const aiScene* pScene, uint& NumVertices, uint& NumIndices);

    void InitAllMeshes(const aiScene* pScene);

    void CalculateMeshTransformations(const aiScene* pScene);
    void TraverseNodeHierarchy(Matrix4f ParentTransformation, aiNode* pNode);

    bool InitMaterials(const aiScene* pScene, const std::string& Filename);

    void LoadTextures(const string& Dir, const aiMaterial* pMaterial, int index);

    void LoadDiffuseTexture(const string& Dir, const aiMaterial* pMaterial, int index);
    void LoadDiffuseTextureEmbedded(const aiTexture* paiTexture, int MaterialIndex);
    void LoadDiffuseTextureFromFile(const string& dir, const aiString& Path, int MaterialIndex);

    void LoadSpecularTexture(const string& Dir, const aiMaterial* pMaterial, int index);
    void LoadSpecularTextureEmbedded(const aiTexture* paiTexture, int MaterialIndex);
    void LoadSpecularTextureFromFile(const string& dir, const aiString& Path, int MaterialIndex);

    void LoadColors(const aiMaterial* pMaterial, int index);

    void InitCameras(const aiScene* pScene, int WindowWidth, int WindowHeight);

    void InitSingleCamera(int Index, const aiScene* pScene, int WindowWidth, int WindowHeight);

    enum BUFFER_TYPE {
        INDEX_BUFFER = 0,
        POS_VB       = 1,
        TEXCOORD_VB  = 2,
        NORMAL_VB    = 3,
        WVP_MAT_VB   = 4,  // required only for instancing
        WORLD_MAT_VB = 5,  // required only for instancing
        NUM_BUFFERS  = 6
    };

    GLuint m_VAO = 0;
    GLuint m_Buffers[NUM_BUFFERS] = { 0 };

    struct BasicMeshEntry {
        BasicMeshEntry()
        {
            NumIndices = 0;
            BaseVertex = 0;
            BaseIndex = 0;
            MaterialIndex = INVALID_MATERIAL;
        }

        uint NumIndices;
        uint BaseVertex;
        uint BaseIndex;
        uint MaterialIndex;
        Matrix4f Transformation;
    };

    std::vector<BasicMeshEntry> m_Meshes;

    const aiScene* m_pScene;

    Matrix4f m_GlobalInverseTransform;

    std::vector<Material> m_Materials;

    // Temporary space for vertex stuff before we load them into the GPU
    vector<Vector3f> m_Positions;
    vector<Vector3f> m_Normals;
    vector<Vector2f> m_TexCoords;
    vector<uint> m_Indices;

    Assimp::Importer m_Importer;

    std::vector<BasicCamera> m_cameras;
    std::vector<DirectionalLight> m_dirLights;
    std::vector<PointLight> m_pointLights;
    std::vector<SpotLight> m_spotLights;
	
	// Skeletal animation stuff
    #define MAX_NUM_BONES_PER_VERTEX 4
	
    struct VertexBoneData
    {
        uint BoneIDs[MAX_NUM_BONES_PER_VERTEX] = { 0 };
        float Weights[MAX_NUM_BONES_PER_VERTEX] = { 0.0f };
        int index = 0;  // slot for the next update

        VertexBoneData()
        {
        }

        void AddBoneData(uint BoneID, float Weight)
        {
            for (int i = 0 ; i < index ; i++) {
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

    void LoadMeshBones(uint MeshIndex, const aiMesh* paiMesh);
    void LoadSingleBone(uint MeshIndex, const aiBone* pBone);
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

    GLuint m_boneBuffer = 0;

    // Temporary space for vertex stuff before we load them into the GPU
    vector<VertexBoneData> m_Bones;

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

