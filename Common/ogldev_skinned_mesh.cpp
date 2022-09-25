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
#include "ogldev_skinned_mesh.h"

using namespace std;

#define BONE_ID_LOCATION     3
#define BONE_WEIGHT_LOCATION 4


SkinnedMesh::~SkinnedMesh()
{
    Clear();
}


void SkinnedMesh::ReserveSpace(unsigned int NumVertices, unsigned int NumIndices)
{
    BasicMesh::ReserveSpace(NumVertices, NumIndices);
    m_Bones.resize(NumVertices);
    InitializeRequiredNodeMap(m_pScene->mRootNode);
}


void SkinnedMesh::InitSingleMesh(uint MeshIndex, const aiMesh* paiMesh)
{
    BasicMesh::InitSingleMesh(MeshIndex, paiMesh);
    LoadMeshBones(MeshIndex, paiMesh);
}


void SkinnedMesh::LoadMeshBones(uint MeshIndex, const aiMesh* pMesh)
{
    if (pMesh->mNumBones > MAX_BONES) {
        printf("The number of bones in the model (%d) is larger than the maximum supported (%d)\n", pMesh->mNumBones, MAX_BONES);
        printf("Make sure to increase the macro MAX_BONES in the C++ header as well as in the shader to the same value\n");
        assert(0);
    }

    // printf("Loading mesh bones %d\n", MeshIndex);
    for (uint i = 0 ; i < pMesh->mNumBones ; i++) {
        // printf("Bone %d %s\n", i, pMesh->mBones[i]->mName.C_Str());
        LoadSingleBone(MeshIndex, pMesh->mBones[i]);
    }
}


void SkinnedMesh::LoadSingleBone(uint MeshIndex, const aiBone* pBone)
{
    int BoneId = GetBoneId(pBone);

    if (BoneId == m_BoneInfo.size()) {
        BoneInfo bi(pBone->mOffsetMatrix);
        // bi.OffsetMatrix.Print();
        m_BoneInfo.push_back(bi);
    }

    for (uint i = 0 ; i < pBone->mNumWeights ; i++) {
        const aiVertexWeight& vw = pBone->mWeights[i];
        uint GlobalVertexID = m_Meshes[MeshIndex].BaseVertex + pBone->mWeights[i].mVertexId;
        // printf("%d: %d %f\n",i, pBone->mWeights[i].mVertexId, vw.mWeight);
        m_Bones[GlobalVertexID].AddBoneData(BoneId, vw.mWeight);
    }

    MarkRequiredNodesForBone(pBone);
}


void SkinnedMesh::MarkRequiredNodesForBone(const aiBone* pBone)
{
    string NodeName(pBone->mName.C_Str());

    const aiNode* pParent = NULL;

    do {
        map<string,NodeInfo>::iterator it = m_requiredNodeMap.find(NodeName);

        if (it == m_requiredNodeMap.end()) {
            printf("Cannot find bone %s in the hierarchy\n", NodeName.c_str());
            assert(0);
        }

        it->second.isRequired = true;

        pParent = it->second.pNode->mParent;

        if (pParent) {
            NodeName = string(pParent->mName.C_Str());
        }

    } while (pParent);
}


void SkinnedMesh::InitializeRequiredNodeMap(const aiNode* pNode)
{
    string NodeName(pNode->mName.C_Str());

    NodeInfo info(pNode);

    m_requiredNodeMap[NodeName] = info;

    for (unsigned int i = 0 ; i < pNode->mNumChildren ; i++) {
        InitializeRequiredNodeMap(pNode->mChildren[i]);
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



void SkinnedMesh::PopulateBuffers()
{
    BasicMesh::PopulateBuffers();

    glGenBuffers(1, &m_boneBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_boneBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_Bones[0]) * m_Bones.size(), &m_Bones[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(BONE_ID_LOCATION);
    glVertexAttribIPointer(BONE_ID_LOCATION, MAX_NUM_BONES_PER_VERTEX, GL_INT, sizeof(VertexBoneData), (const GLvoid*)0);
    glEnableVertexAttribArray(BONE_WEIGHT_LOCATION);
    glVertexAttribPointer(BONE_WEIGHT_LOCATION, MAX_NUM_BONES_PER_VERTEX, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData),
                          (const GLvoid*)(MAX_NUM_BONES_PER_VERTEX * sizeof(int32_t)));
}



uint SkinnedMesh::FindPosition(float AnimationTimeTicks, const aiNodeAnim* pNodeAnim)
{
    for (uint i = 0 ; i < pNodeAnim->mNumPositionKeys - 1 ; i++) {
        float t = (float)pNodeAnim->mPositionKeys[i + 1].mTime;
        if (AnimationTimeTicks < t) {
            return i;
        }
    }

    return 0;
}


void SkinnedMesh::CalcInterpolatedPosition(aiVector3D& Out, float AnimationTimeTicks, const aiNodeAnim* pNodeAnim)
{
    // we need at least two values to interpolate...
    if (pNodeAnim->mNumPositionKeys == 1) {
        Out = pNodeAnim->mPositionKeys[0].mValue;
        return;
    }

    uint PositionIndex = FindPosition(AnimationTimeTicks, pNodeAnim);
    uint NextPositionIndex = PositionIndex + 1;
    assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
    float t1 = (float)pNodeAnim->mPositionKeys[PositionIndex].mTime;
    if (t1 > AnimationTimeTicks) {
        Out = pNodeAnim->mPositionKeys[PositionIndex].mValue;
    } else {
        float t2 = (float)pNodeAnim->mPositionKeys[NextPositionIndex].mTime;
        float DeltaTime = t2 - t1;
        float Factor = (AnimationTimeTicks - t1) / DeltaTime;
        assert(Factor >= 0.0f && Factor <= 1.0f);
        const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
        const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
        aiVector3D Delta = End - Start;
        Out = Start + Factor * Delta;
    }
}


uint SkinnedMesh::FindRotation(float AnimationTimeTicks, const aiNodeAnim* pNodeAnim)
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


void SkinnedMesh::CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTimeTicks, const aiNodeAnim* pNodeAnim)
{
    // we need at least two values to interpolate...
    if (pNodeAnim->mNumRotationKeys == 1) {
        Out = pNodeAnim->mRotationKeys[0].mValue;
        return;
    }

    uint RotationIndex = FindRotation(AnimationTimeTicks, pNodeAnim);
    uint NextRotationIndex = RotationIndex + 1;
    assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
    float t1 = (float)pNodeAnim->mRotationKeys[RotationIndex].mTime;
    if (t1 > AnimationTimeTicks) {
        Out = pNodeAnim->mRotationKeys[RotationIndex].mValue;
    } else {
        float t2 = (float)pNodeAnim->mRotationKeys[NextRotationIndex].mTime;
        float DeltaTime = t2 - t1;
        float Factor = (AnimationTimeTicks - t1) / DeltaTime;
        assert(Factor >= 0.0f && Factor <= 1.0f);
        const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
        const aiQuaternion& EndRotationQ   = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
        aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
    }

    Out.Normalize();
}


uint SkinnedMesh::FindScaling(float AnimationTimeTicks, const aiNodeAnim* pNodeAnim)
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


void SkinnedMesh::CalcInterpolatedScaling(aiVector3D& Out, float AnimationTimeTicks, const aiNodeAnim* pNodeAnim)
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


void SkinnedMesh::ReadNodeHierarchy(float AnimationTimeTicks, const aiNode* pNode, const Matrix4f& ParentTransform, const aiAnimation& Animation)
{
    string NodeName(pNode->mName.data);

    Matrix4f NodeTransformation(pNode->mTransformation);

    const aiNodeAnim* pNodeAnim = FindNodeAnim(Animation, NodeName);
    aiVector3D Scaling;
    aiQuaternion RotationQ;
    aiVector3D Translation;

    if (pNodeAnim) {
        // Interpolate scaling and generate scaling transformation matrix
        CalcInterpolatedScaling(Scaling, AnimationTimeTicks, pNodeAnim);
        Matrix4f ScalingM;
        ScalingM.InitScaleTransform(Scaling.x, Scaling.y, Scaling.z);
        //        printf("Scaling %f %f %f\n", Scaling.x, Scaling.y, Scaling.z);
        // Interpolate rotation and generate rotation transformation matrix
        CalcInterpolatedRotation(RotationQ, AnimationTimeTicks, pNodeAnim);
        Matrix4f RotationM = Matrix4f(RotationQ.GetMatrix());

        // Interpolate translation and generate translation transformation matrix
        CalcInterpolatedPosition(Translation, AnimationTimeTicks, pNodeAnim);
        Matrix4f TranslationM;
        TranslationM.InitTranslationTransform(Translation.x, Translation.y, Translation.z);
        //        printf("Translation %f %f %f\n", Translation.x, Translation.y, Translation.z);
        // Combine the above transformations
        NodeTransformation = TranslationM * RotationM * ScalingM;
    } else {
        //        printf("%s\n", NodeName.c_str());
        //        NodeTransformation.Print();
    }

    Matrix4f GlobalTransformation = ParentTransform * NodeTransformation;

    if (m_BoneNameToIndexMap.find(NodeName) != m_BoneNameToIndexMap.end()) {
        uint BoneIndex = m_BoneNameToIndexMap[NodeName];
        m_BoneInfo[BoneIndex].FinalTransformation = m_GlobalInverseTransform * GlobalTransformation * m_BoneInfo[BoneIndex].OffsetMatrix;
    }

    for (uint i = 0 ; i < pNode->mNumChildren ; i++) {
        string ChildName(pNode->mChildren[i]->mName.data);

        map<string,NodeInfo>::iterator it = m_requiredNodeMap.find(ChildName);

        if (it == m_requiredNodeMap.end()) {
            printf("Child %s cannot be found in the required node map\n", ChildName.c_str());
            assert(0);
        }

        if (it->second.isRequired) {
            ReadNodeHierarchy(AnimationTimeTicks, pNode->mChildren[i], GlobalTransformation, Animation);
        }
    }
}


void SkinnedMesh::ReadNodeHierarchyBlended(float StartAnimationTimeTicks, float EndAnimationTimeTicks, const aiNode* pNode, const Matrix4f& ParentTransform,
                                           const aiAnimation& StartAnimation, const aiAnimation& EndAnimation, float BlendFactor)
{
    string NodeName(pNode->mName.data);

    Matrix4f NodeTransformation(pNode->mTransformation);

    const aiNodeAnim* pStartNodeAnim = FindNodeAnim(StartAnimation, NodeName);

    LocalTransform StartTransform;

    if (pStartNodeAnim) {
        CalcLocalTransform(StartTransform, StartAnimationTimeTicks, pStartNodeAnim);
    } else {
        //        printf("%s\n", NodeName.c_str());
        //        NodeTransformation.Print();
    }

    LocalTransform EndTransform;

    const aiNodeAnim* pEndNodeAnim = FindNodeAnim(EndAnimation, NodeName);

    if ((pStartNodeAnim && !pEndNodeAnim) || (!pStartNodeAnim && pEndNodeAnim)) {
        printf("On the node %s there is an animation node for only one of the start/end animations.\n", NodeName.c_str());
        printf("This case is not supported\n");
        exit(0);
    }

    if (pEndNodeAnim) {
        CalcLocalTransform(EndTransform, EndAnimationTimeTicks, pEndNodeAnim);
    }

    if (pStartNodeAnim && pEndNodeAnim) {
        const aiQuaternion& Rot0 = StartTransform.Rotation;
        const aiQuaternion& Rot1 = EndTransform.Rotation;
        aiQuaternion BlendedRot;
        aiQuaternion::Interpolate(BlendedRot, Rot0, Rot1, BlendFactor);
        Matrix4f RotationM = Matrix4f(BlendedRot.GetMatrix());

        const aiVector3D& Pos0 = StartTransform.Translation;
        const aiVector3D& Pos1 = EndTransform.Translation;
        aiVector3D BlendedTranslation = (1.0f - BlendFactor) * Pos0 + Pos1 * BlendFactor;
        Matrix4f TranslationM;
        TranslationM.InitTranslationTransform(BlendedTranslation.x, BlendedTranslation.y, BlendedTranslation.z);
        NodeTransformation = TranslationM * RotationM;
    }

    Matrix4f GlobalTransformation = ParentTransform * NodeTransformation;

    if (m_BoneNameToIndexMap.find(NodeName) != m_BoneNameToIndexMap.end()) {
        uint BoneIndex = m_BoneNameToIndexMap[NodeName];
        m_BoneInfo[BoneIndex].FinalTransformation = m_GlobalInverseTransform * GlobalTransformation * m_BoneInfo[BoneIndex].OffsetMatrix;
    }

    for (uint i = 0 ; i < pNode->mNumChildren ; i++) {
        string ChildName(pNode->mChildren[i]->mName.data);

        map<string,NodeInfo>::iterator it = m_requiredNodeMap.find(ChildName);

        if (it == m_requiredNodeMap.end()) {
            printf("Child %s cannot be found in the required node map\n", ChildName.c_str());
            assert(0);
        }

        if (it->second.isRequired) {
            ReadNodeHierarchyBlended(StartAnimationTimeTicks, EndAnimationTimeTicks, pNode->mChildren[i], GlobalTransformation, StartAnimation, EndAnimation, BlendFactor);
        }
    }
}


void SkinnedMesh::CalcLocalTransform(LocalTransform& Transform, float AnimationTimeTicks, const aiNodeAnim* pNodeAnim)
{
    CalcInterpolatedScaling(Transform.Scaling, AnimationTimeTicks, pNodeAnim);
    CalcInterpolatedRotation(Transform.Rotation, AnimationTimeTicks, pNodeAnim);
    CalcInterpolatedPosition(Transform.Translation, AnimationTimeTicks, pNodeAnim);
}


void SkinnedMesh::GetBoneTransforms(float TimeInSeconds, vector<Matrix4f>& Transforms, unsigned int AnimationIndex)
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
        //  m_BoneInfo[i].FinalTransformation.Print();
    }

    //exit(0);
}


void SkinnedMesh::GetBoneTransformsBlended(float TimeInSeconds,
                                           vector<Matrix4f>& BlendedTransforms,
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

    Matrix4f Identity;
    Identity.InitIdentity();

    float StartAnimationTimeTicks = CalcAnimationTimeTicks(TimeInSeconds, StartAnimIndex);
    float EndAnimationTimeTicks = CalcAnimationTimeTicks(TimeInSeconds, EndAnimIndex);

    const aiAnimation& StartAnimation = *m_pScene->mAnimations[StartAnimIndex];
    const aiAnimation& EndAnimation = *m_pScene->mAnimations[EndAnimIndex];

    ReadNodeHierarchyBlended(StartAnimationTimeTicks, EndAnimationTimeTicks, m_pScene->mRootNode, Identity, StartAnimation, EndAnimation, BlendFactor);
    BlendedTransforms.resize(m_BoneInfo.size());

    for (uint i = 0 ; i < m_BoneInfo.size() ; i++) {
        BlendedTransforms[i] = m_BoneInfo[i].FinalTransformation;
        //  m_BoneInfo[i].FinalTransformation.Print();
    }

}


/*void SkinnedMesh::GetBoneTransformsBlended(float TimeInSeconds,
                                           vector<Matrix4f>& BlendedTransforms,
                                           unsigned int StartAnimIndex,
                                           unsigned int EndAnimIndex,
                                           float BlendFactor)
{
    vector<Matrix4f> StartTransforms;
    GetBoneTransforms(TimeInSeconds, StartTransforms, StartAnimIndex);

    vector<LocalTransform> StartTransformsLocal;
    vector<Matrix4f> Foo;
    Foo.resize(StartTransforms.size());
    StartTransformsLocal.resize(StartTransforms.size());

    for (int i = 0 ; i < StartTransforms.size() ; i++) {
        StartTransformsLocal[i].Scaling     = m_BoneInfo[i].Scaling;
        StartTransformsLocal[i].Rotation    = m_BoneInfo[i].Rotation;
        StartTransformsLocal[i].Translation = m_BoneInfo[i].Translation;
        Foo[i] = m_BoneInfo[i].LocalTransformation;
    }

    vector<Matrix4f> EndTransforms;
    GetBoneTransforms(TimeInSeconds, EndTransforms, EndAnimIndex);

    assert(StartTransforms.size() == EndTransforms.size());

    vector<LocalTransform> EndTransformsLocal;
    EndTransformsLocal.resize(StartTransforms.size());

    for (int i = 0 ; i < StartTransforms.size() ; i++) {
        EndTransformsLocal[i].Scaling     = m_BoneInfo[i].Scaling;
        EndTransformsLocal[i].Rotation    = m_BoneInfo[i].Rotation;
        EndTransformsLocal[i].Translation = m_BoneInfo[i].Translation;
    }

    vector<Matrix4f> BlendedLocalTransforms;
    BlendedLocalTransforms.resize(StartTransforms.size());

    for (int i = 0 ; i < StartTransformsLocal.size() ; i++) {
        if (StartTransformsLocal[i].Scaling == aiVector3D(0.0f, 0.0f, 0.0f)) {
            BlendedLocalTransforms[i].InitIdentity();
        } else {
            const aiQuaternion& Rot0 = StartTransformsLocal[i].Rotation;
            const aiQuaternion& Rot1 = EndTransformsLocal[i].Rotation;
            aiQuaternion BlendedRot;
            aiQuaternion::Interpolate(BlendedRot, Rot0, Rot1, BlendFactor);
            Matrix4f RotationM = Matrix4f(BlendedRot.GetMatrix());

            const aiVector3D& Pos0 = StartTransformsLocal[i].Translation;
            const aiVector3D& Pos1 = EndTransformsLocal[i].Translation;
            aiVector3D BlendedTranslation = (1.0f - BlendFactor) * Pos0 + Pos1 * BlendFactor;
            Matrix4f TranslationM;
            TranslationM.InitTranslationTransform(BlendedTranslation.x, BlendedTranslation.y, BlendedTranslation.z);
            BlendedLocalTransforms[i] = TranslationM * RotationM;
            //BlendedLocalTransforms[i] = Foo[i];
        }
        //        BlendedLocalTransforms[i].Print();
    }

    Matrix4f ParentTransform;
    ParentTransform.InitIdentity();

    ReadNodeHierarchyBlended(m_pScene->mRootNode, ParentTransform, BlendedLocalTransforms);

    BlendedTransforms.resize(m_BoneInfo.size());

    for (uint i = 0 ; i < m_BoneInfo.size() ; i++) {
        BlendedTransforms[i] = m_BoneInfo[i].FinalTransformation;
        //        BlendedTransforms[i].Print();
    }

    //    exit(0);
    }*/

/*void SkinnedMesh::GetBoneTransformsBlended(float AnimationTimeSec,
                                           vector<Matrix4f>& Transforms,
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

    if ((BlendFactor > 1.0f) || (BlendFactor < 0.0f)) {
        printf("Invalid blend factor %f\n", BlendFactor);
        assert(0);
    }

    float a = 1.0f;
    float b = m_pScene->mAnimations[StartAnimIndex]->mDuration / m_pScene->mAnimations[EndAnimIndex]->mDuration;

    float AnimSpeedMultiplierUp = (1.0f - BlendFactor) * a + b * BlendFactor;

    a = m_pScene->mAnimations[EndAnimIndex]->mDuration / m_pScene->mAnimations[StartAnimIndex]->mDuration;
    b = 1.0f;

    float AnimSpeedMultiplierDown = (1.0f - BlendFactor) * a + b * BlendFactor;

    // Current time of each animation, "scaled" by the above speed multiplier variables
    static float currentTimeBase = 0.0f;
    currentTimeBase += m_pScene->mAnimations[StartAnimIndex]->mTicksPerSecond * AnimationTimeSec * AnimSpeedMultiplierUp;
    currentTimeBase = fmod(currentTimeBase, m_pScene->mAnimations[StartAnimIndex]->mDuration);

    static float currentTimeLayered = 0.0f;
    currentTimeLayered += m_pScene->mAnimations[EndAnimIndex]->mTicksPerSecond * AnimationTimeSec * AnimSpeedMultiplierDown;
    currentTimeLayered = fmod(currentTimeLayered, m_pScene->mAnimations[EndAnimIndex]->mDuration);

    aiAnimation* pBaseAnimation = m_pScene->mAnimations[StartAnimIndex];
    aiAnimation* pLayeredAnimation = m_pScene->mAnimations[EndAnimIndex];
    Matrix4f Identity;
    Identity.InitIdentity();
    //CalculateBlendedBoneTransform(pBaseAnimation, pLayeredAnimation, currentTimeBase, currentTimeLayered, Identity, BlendFactor);
}


/*void SkinnedMesh::CalculateBlendedBoneTransform(aiAnimation* pAnimationBase, aiAnimation* pAnimationLayer,
                                                float currentTimeBase, float currentTimeLayered,
                                                const Matrix4f& parentTransform, float BlendFactor)
{
    string NodeName(pNode->mName.data);

    glm::mat4 nodeTransform = node->transformation;
    Bone* pBone = pAnimationBase->FindBone(nodeName);
    if (pBone)
    {
        pBone->Update(currentTimeBase);
        nodeTransform = pBone->GetLocalTransform();
    }

    glm::mat4 layeredNodeTransform = nodeLayered->transformation;
    pBone = pAnimationLayer->FindBone(nodeName);
    if (pBone)
    {
        pBone->Update(currentTimeLayered);
        layeredNodeTransform = pBone->GetLocalTransform();
    }

    // Blend two matrices
    const glm::quat rot0 = glm::quat_cast(nodeTransform);
    const glm::quat rot1 = glm::quat_cast(layeredNodeTransform);
    const glm::quat finalRot = glm::slerp(rot0, rot1, blendFactor);
    glm::mat4 blendedMat = glm::mat4_cast(finalRot);
    blendedMat[3] = (1.0f - blendFactor) * nodeTransform[3] + layeredNodeTransform[3] * blendFactor;

    glm::mat4 globalTransformation = parentTransform * blendedMat;

    const auto& boneInfoMap = pAnimationBase->GetBoneInfoMap();
    if (boneInfoMap.find(nodeName) != boneInfoMap.end())
    {
        const int index = boneInfoMap.at(nodeName).id;
        const glm::mat4& offset = boneInfoMap.at(nodeName).offset;

        m_FinalBoneMatrices[index] = globalTransformation * offset;
    }

    for (size_t i = 0; i < node->children.size(); ++i)
        CalculateBlendedBoneTransform(pAnimationBase, &node->children[i], pAnimationLayer, &nodeLayered->children[i], currentTimeBase, currentTimeLayered, globalTransformation, blendFactor);
}
*/


float SkinnedMesh::CalcAnimationTimeTicks(float TimeInSeconds, unsigned int AnimationIndex)
{
    float TicksPerSecond = (float)(m_pScene->mAnimations[AnimationIndex]->mTicksPerSecond != 0 ? m_pScene->mAnimations[AnimationIndex]->mTicksPerSecond : 25.0f);
    float TimeInTicks = TimeInSeconds * TicksPerSecond;
    // we need to use the integral part of mDuration for the total length of the animation
    float Duration = 0.0f;
    float fraction = modf((float)m_pScene->mAnimations[AnimationIndex]->mDuration, &Duration);
    float AnimationTimeTicks = fmod(TimeInTicks, Duration);
    return AnimationTimeTicks;
}


const aiNodeAnim* SkinnedMesh::FindNodeAnim(const aiAnimation&
                                            Animation, const string& NodeName)
{
    for (uint i = 0 ; i < Animation.mNumChannels ; i++) {
        const aiNodeAnim* pNodeAnim = Animation.mChannels[i];

        if (string(pNodeAnim->mNodeName.data) == NodeName) {
            return pNodeAnim;
        }
    }

    return NULL;
}
