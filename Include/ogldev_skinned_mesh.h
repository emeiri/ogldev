/*

    Copyright 2021 Etay Meiri

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

#ifndef OGLDEV_SKINNED_MESH_H
#define OGLDEV_SKINNED_MESH_H

#include <map>
#include <vector>
#include <GL/glew.h>

#include "ogldev_util.h"
#include "ogldev_math_3d.h"
#include "ogldev_texture.h"
#include "ogldev_world_transform.h"
#include "ogldev_material.h"
#include "ogldev_basic_mesh.h"

class SkinnedMesh : public BasicMesh
{
public:
    SkinnedMesh() {};

    ~SkinnedMesh();

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
private:
    #define MAX_NUM_BONES_PER_VERTEX 4

    virtual void ReserveSpace(unsigned int NumVertices, unsigned int NumIndices);

    virtual void InitSingleMesh(uint MeshIndex, const aiMesh* paiMesh);

    virtual void PopulateBuffers();

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


#endif  /* OGLDEV_SKINNED_MESH_H */
