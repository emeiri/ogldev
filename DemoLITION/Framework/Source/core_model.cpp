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

#include "Int/core_rendering_system.h"
#include "Int/core_model.h"

//#define DEBUG_TEXTURES

#define DEBUG_SCENE_HIERARCHY

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


void CoreModel::LoadAssimpModel(const std::string& Filename, const ModelLoadFlags& Flags)
{
    AssimpModelLoader Loader;
    Loader.Load(Filename, Flags, m_modelData);

#ifndef OGLDEV_VULKAN
    std::string FileExt = Filename.substr(Filename.find_last_of(".") + 1);
    m_flipTexCoordV = (FileExt == "glb" || FileExt == "gltf");
#endif

    InitModel();
}


void CoreModel::InitModel()
{
    printf("\n*** Initializing model ***\n");
    
    m_Materials.resize(m_modelData.m_materials.size());

    unsigned int NumVertices = (unsigned int)m_modelData.NumVertices();
    unsigned int NumIndices = (unsigned int)m_modelData.m_indices.size();

    InitMaterials();

    InitGPUBuffers(NumVertices, NumIndices);

    InitCameras();

    CalculateMeshTransformations();

    InitGeometryPost();

#ifndef OGLDEV_VULKAN
    assert(GLCheckError());
#endif
}


void CoreModel::InitGPUBuffers(unsigned int NumVertices, unsigned int NumIndices)
{
    printf("Num skinned vertices %d\n", (int)m_modelData.m_skinnedVertices.size());

    if (m_modelData.IsSkinned()) {
        PopulateBuffersSkinned(m_modelData.m_skinnedVertices);
    }
    else {
        PopulateBuffers(m_modelData.m_vertices);
    }
}


void CoreModel::InitCameras()
{
    if (m_modelData.m_cameras.empty()) {
        printf("No cameras found in the model\n");
        return;
    }

    m_cameras.resize(m_modelData.m_cameras.size());

    int WindowWidth, WindowHeight;
#ifdef OGLDEV_VULKAN
    GetWindowSize(WindowWidth, WindowHeight);
#else
    m_pCoreRenderingSystem->GetWindowSize(WindowWidth, WindowHeight);
#endif

    for (int CameraIndex = 0; CameraIndex < m_modelData.m_cameras.size(); CameraIndex++) {
        const CameraData& Camera = m_modelData.m_cameras[CameraIndex];
        PersProjInfo persProjInfo;
        persProjInfo.Width = (float)WindowWidth;
        persProjInfo.Height = (float)WindowHeight;

        float AspectRatio = (float)WindowWidth / (float)WindowHeight;

        // 3. Mathematical conversion: Full Horizontal FOV -> Full Vertical FOV
        float FullVerticalRad = 2.0f * atan(tan(ToRadian(Camera.m_hFov) / 2.0f) / AspectRatio);
        float FullVerticalDeg = ToDegree(FullVerticalRad);

        persProjInfo.FOV = FullVerticalDeg;
        persProjInfo.zNear = Camera.m_zNear;
        persProjInfo.zFar = Camera.m_zFar;

        // GLMCameraFirstPerson is a yaw/pitch camera (no roll) and its Up parameter
        // is the world up axis, not the camera's own up vector (Camera.m_up)
        glm::vec3 WorldUp(0.0f, 1.0f, 0.0f);
        m_cameras[CameraIndex].Init(Camera.m_pos, Camera.m_dir, WorldUp, persProjInfo);
    }
}


void CoreModel::CalculateMeshTransformations()
{
    printf("----------------------------------------\n");
    printf("Calculating mesh transformations\n");

    for (uint NodeIndex = 0; NodeIndex < m_modelData.m_nodes.size(); NodeIndex++) {
        SkelNode& Node = m_modelData.m_nodes[NodeIndex];

        glm::mat4 MeshTransform = glm::make_mat4(Node.m_bindPoseGlobalTransform.data());
        // The matrix is stored as row major in the file but glm expects column major by default.
        glm::mat4 MeshTransformGLM = glm::transpose(MeshTransform);

        glm::mat4 ZFlip = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, -1.0f));
        glm::mat4 LeftHandedMatrixGLM = ZFlip * MeshTransformGLM * ZFlip;

        for (int j = 0; j < Node.m_meshes.size(); j++) {
            int MeshIndex = Node.m_meshes[j];
            m_transformations.push_back(Node.m_bindPoseGlobalTransform);
            m_transformationsGLM.push_back(LeftHandedMatrixGLM);
        }
    }
}


void CoreModel::InitMaterials()
{
    for (int MaterialIndex = 0; MaterialIndex < m_modelData.m_materials.size(); MaterialIndex++) {

        for (int TexType = 0; TexType < TEX_TYPE_NUM; TexType++) {        
            TextureSource& TextureSource = m_modelData.m_materials[MaterialIndex].m_textures[TexType];

            if (!TextureSource.IsValid()) {
                continue;
            }

            bool IsSRGB = TextureSource.IsSRGB;
            
            if (TextureSource.EmbeddedBytes.size() > 0) {            
                LoadTextureEmbedded(MaterialIndex, (TEXTURE_TYPE)TexType, IsSRGB);
            } else {               
                const std::string& Path = TextureSource.Path;
                LoadTextureFromFile(Path, MaterialIndex, (TEXTURE_TYPE)TexType, IsSRGB);
            }
        }

        m_Materials[MaterialIndex].m_isPBR = m_modelData.m_materials[MaterialIndex].m_isPBR;
        m_Materials[MaterialIndex].m_name = m_modelData.m_materials[MaterialIndex].m_name;
        m_Materials[MaterialIndex].m_materialType = m_modelData.m_materials[MaterialIndex].m_materialType;
        m_Materials[MaterialIndex].AmbientColor = m_modelData.m_materials[MaterialIndex].m_ambientColor;
        m_Materials[MaterialIndex].DiffuseColor = m_modelData.m_materials[MaterialIndex].m_diffuseColor;
        m_Materials[MaterialIndex].SpecularColor = m_modelData.m_materials[MaterialIndex].m_specularColor;
        m_Materials[MaterialIndex].BaseColor = m_modelData.m_materials[MaterialIndex].m_baseColor;
        m_Materials[MaterialIndex].EmissiveColor = m_modelData.m_materials[MaterialIndex].m_emissiveColor;
        m_Materials[MaterialIndex].MetallicRoughnessNormalOcclusion = m_modelData.m_materials[MaterialIndex].m_metallicRoughnessNormalOcclusion;
        m_Materials[MaterialIndex].ClearCoatTransmissionThickness = m_modelData.m_materials[MaterialIndex].m_clearCoatTransmissionThickness;
        m_Materials[MaterialIndex].m_transparencyFactor = m_modelData.m_materials[MaterialIndex].m_transparencyFactor;
        m_Materials[MaterialIndex].m_alphaTest = m_modelData.m_materials[MaterialIndex].m_alphaTest;
        m_Materials[MaterialIndex].m_ior = m_modelData.m_materials[MaterialIndex].m_ior;        
    }
}


void CoreModel::LoadTextureEmbedded(int MaterialIndex, TEXTURE_TYPE TexType, bool IsSRGB) 
{
#ifdef DEBUG_MATERIALS
    printf("Loaded embedded texture type '%s'\n", paiTexture->achFormatHint);
#endif
    // Raw assignment compiles successfully again
    m_Materials[MaterialIndex].pTextures[TexType] = AllocTexture2D();
    size_t BufferSize = m_modelData.m_materials[MaterialIndex].m_textures[TexType].EmbeddedBytes.size();
    void* pData = m_modelData.m_materials[MaterialIndex].m_textures[TexType].EmbeddedBytes.data();
    m_Materials[MaterialIndex].pTextures[TexType]->Load((u32)BufferSize, pData, IsSRGB);
}


void CoreModel::LoadTextureFromFile(const std::string& Path, int MaterialIndex, TEXTURE_TYPE TexType, bool IsSRGB) 
{
    m_Materials[MaterialIndex].pTextures[TexType] = AllocTexture2D();
    m_Materials[MaterialIndex].pTextures[TexType]->Load(Path.c_str(), IsSRGB);

#ifdef DEBUG_MATERIALS
    printf("Loaded texture type %d from '%s'\n", TexType, Path.c_str());
#endif
}



glm::mat4& CoreModel::GetNodeTransformationGLM(const char* pNodeName)
{
    auto it = m_modelData.m_nodeMap.find(pNodeName);

    if (it == m_modelData.m_nodeMap.end()) {
        OGLDEV_ERROR("Cannot find node %s in the hierarchy\n", pNodeName);
        assert(0);
    }

    int NodeIndex = it->second;

    const SkelNode& Node = m_modelData.m_nodes[NodeIndex];

    int FirstInstance = Node.m_firstInstance;

    return m_transformationsGLM[FirstInstance];
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



uint CoreModel::FindPosition(float AnimationTimeTicks, const AnimChannel& NodeAnim)
{
    for (uint i = 0 ; i < NodeAnim.m_positions.size() - 1 ; i++) {
        float t = (float)NodeAnim.m_positions[i + 1].m_time;
        if (AnimationTimeTicks < t) {
            return i;
        }
    }

    return 0;
}


void CoreModel::CalcInterpolatedPosition(glm::vec3& Out, float AnimationTimeTicks, const AnimChannel& NodeAnim)
{
    // we need at least two values to interpolate...
    if (NodeAnim.m_positions.size() == 1) {
        Out = NodeAnim.m_positions[0].m_value;
        return;
    }

    float MaxTime = (float)NodeAnim.m_positions[NodeAnim.m_positions.size() - 1].m_time;
    if (AnimationTimeTicks >= MaxTime) {
        Out = NodeAnim.m_positions[NodeAnim.m_positions.size() - 1].m_value;
        return;
    }

    uint PositionIndex = FindPosition(AnimationTimeTicks, NodeAnim);
    uint NextPositionIndex = PositionIndex + 1;
    assert(NextPositionIndex < NodeAnim.m_positions.size());

    float t1 = (float)NodeAnim.m_positions[PositionIndex].m_time;
    float t2 = (float)NodeAnim.m_positions[NextPositionIndex].m_time;

    // Fix: Stripped redundant unreachable 'if (t1 > AnimationTimeTicks)' check and safe-guarded division
    float DeltaTime = t2 - t1;
    float Factor = 0.0f;
    if (DeltaTime > 0.0f) {
        Factor = (AnimationTimeTicks - t1) / DeltaTime;
    }

    Factor = std::clamp(Factor, 0.0f, 1.0f);

    const glm::vec3& Start = NodeAnim.m_positions[PositionIndex].m_value;
    const glm::vec3& End = NodeAnim.m_positions[NextPositionIndex].m_value;
    Out = Start + Factor * (End - Start);
}


uint CoreModel::FindRotation(float AnimationTimeTicks, const AnimChannel& NodeAnim)
{
    assert(NodeAnim.m_rotations.size() > 0);

    for (uint i = 0 ; i < NodeAnim.m_rotations.size() - 1 ; i++) {
        float t = (float)NodeAnim.m_rotations[i + 1].m_time;
        if (AnimationTimeTicks < t) {
            return i;
        }
    }

    return 0;
}


void CoreModel::CalcInterpolatedRotation(glm::quat& Out, float AnimationTimeTicks, const AnimChannel& NodeAnim) 
{
    // we need at least two values to interpolate...
    if (NodeAnim.m_rotations.size() == 1) {
        Out = NodeAnim.m_rotations[0].m_value;
        return;
    }

    float MaxTime = (float)NodeAnim.m_rotations[NodeAnim.m_rotations.size() - 1].m_time;
    if (AnimationTimeTicks >= MaxTime) {
        Out = NodeAnim.m_rotations[NodeAnim.m_rotations.size() - 1].m_value;
        return;
    }

    uint RotationIndex = FindRotation(AnimationTimeTicks, NodeAnim);
    uint NextRotationIndex = RotationIndex + 1;
    assert(NextRotationIndex < NodeAnim.m_rotations.size());

    float t1 = (float)NodeAnim.m_rotations[RotationIndex].m_time;
    float t2 = (float)NodeAnim.m_rotations[NextRotationIndex].m_time;

    float DeltaTime = t2 - t1;
    float Factor = 0.0f;
    if (DeltaTime > 0.0f) {
        Factor = (AnimationTimeTicks - t1) / DeltaTime;
    }
    
    Factor = std::clamp(Factor, 0.0f, 1.0f);

    const glm::quat& StartRotationQ = NodeAnim.m_rotations[RotationIndex].m_value;
    const glm::quat& EndRotationQ = NodeAnim.m_rotations[NextRotationIndex].m_value;
    Out = glm::slerp(StartRotationQ, EndRotationQ, Factor);
    Out = glm::normalize(Out);
}


uint CoreModel::FindScaling(float AnimationTimeTicks, const AnimChannel& NodeAnim)
{
    assert(NodeAnim.m_scalings.size() > 0);

    for (uint i = 0 ; i < NodeAnim.m_scalings.size() - 1 ; i++) {
        float t = (float)NodeAnim.m_scalings[i + 1].m_time;
        if (AnimationTimeTicks < t) {
            return i;
        }
    }

    return 0;
}


void CoreModel::CalcInterpolatedScaling(glm::vec3& Out, float AnimationTimeTicks, const AnimChannel& NodeAnim)
{
    // we need at least two values to interpolate...
    if (NodeAnim.m_scalings.size() == 1) {
        Out = NodeAnim.m_scalings[0].m_value;
        return;
    }

    uint ScalingIndex = FindScaling(AnimationTimeTicks, NodeAnim);
    uint NextScalingIndex = ScalingIndex + 1;
    assert(NextScalingIndex < NodeAnim.m_scalings.size());
    float t1 = (float)NodeAnim.m_scalings[ScalingIndex].m_time;
    if (t1 > AnimationTimeTicks) {
        Out = NodeAnim.m_scalings[ScalingIndex].m_value;
    } else {
        float t2 = (float)NodeAnim.m_scalings[NextScalingIndex].m_time;
        float DeltaTime = t2 - t1;
        float Factor = (AnimationTimeTicks - (float)t1) / DeltaTime;
        assert(Factor >= 0.0f && Factor <= 1.0f);
        const glm::vec3& Start = NodeAnim.m_scalings[ScalingIndex].m_value;
        const glm::vec3& End   = NodeAnim.m_scalings[NextScalingIndex].m_value;
        glm::vec3 Delta = End - Start;
        Out = Start + Factor * Delta;
    }
}


void CoreModel::ReadNodeHierarchy(float AnimationTimeTicks, const AnimationClip& Animation)
{
    // The nodes are stored parent-first so the parent's global transform is always ready
    std::vector<Matrix4f> GlobalTransforms(m_modelData.m_nodes.size());

    for (uint NodeIndex = 0; NodeIndex < m_modelData.m_nodes.size(); NodeIndex++) {
        const SkelNode& Node = m_modelData.m_nodes[NodeIndex];
        std::string NodeName(Node.m_name);

        Matrix4f NodeTransformation(Node.m_localTransform);

        const AnimChannel* pNodeAnim = Animation.FindNodeAnim(NodeIndex);

        if (pNodeAnim) {
            LocalTransform Transform;
            CalcLocalTransform(Transform, AnimationTimeTicks, *pNodeAnim);

            Matrix4f ScalingM;
            ScalingM.InitScaleTransform(Transform.Scaling.x, Transform.Scaling.y, Transform.Scaling.z);
            // printf("Scaling %f %f %f\n", Transform.Scaling.x, Transform.Scaling.y, Transform.Scaling.z);

            glm::mat4 RotationGLM = glm::mat4_cast(Transform.Rotation);
            Matrix4f RotationM = Matrix4f(RotationGLM);

            Matrix4f TranslationM;
            TranslationM.InitTranslationTransform(Transform.Translation.x, Transform.Translation.y, Transform.Translation.z);
            //printf("Translation %f %f %f\n", Transform.Translation.x, Transform.Translation.y, Transform.Translation.z);

            // Combine the above transformations
            NodeTransformation = TranslationM * RotationM * ScalingM;
        }

        if (Node.m_parentIndex >= 0) {
            GlobalTransforms[NodeIndex] = GlobalTransforms[Node.m_parentIndex] * NodeTransformation;
        } else {
            GlobalTransforms[NodeIndex] = NodeTransformation;
        }

        if (m_modelData.m_boneNameToIndexMap.find(NodeName) != m_modelData.m_boneNameToIndexMap.end()) {
            uint BoneIndex = m_modelData.m_boneNameToIndexMap[NodeName];
            m_modelData.m_boneInfo[BoneIndex].FinalTransformation = m_modelData.m_globalInverseTransform *
                                                                    GlobalTransforms[NodeIndex] *
                                                                    m_modelData.m_boneInfo[BoneIndex].OffsetMatrix;
        }
    }
}


void CoreModel::ReadNodeHierarchyBlended(float StartAnimationTimeTicks, float EndAnimationTimeTicks,
                                         const AnimationClip& StartAnimation, const AnimationClip& EndAnimation, float BlendFactor) 
{
    std::vector<Matrix4f> GlobalTransforms(m_modelData.m_nodes.size());

    for (uint NodeIndex = 0; NodeIndex < m_modelData.m_nodes.size(); NodeIndex++) {
        const SkelNode& Node = m_modelData.m_nodes[NodeIndex];
        std::string NodeName(Node.m_name);

        Matrix4f NodeTransformation(Node.m_localTransform);

        const AnimChannel* pStartNodeAnim = StartAnimation.FindNodeAnim(NodeIndex);
   
        LocalTransform StartTransform;

        if (pStartNodeAnim) {
            CalcLocalTransform(StartTransform, StartAnimationTimeTicks, *pStartNodeAnim);
        }

        const AnimChannel* pEndNodeAnim = EndAnimation.FindNodeAnim(NodeIndex);
        LocalTransform EndTransform;
        if (pEndNodeAnim) {
            CalcLocalTransform(EndTransform, EndAnimationTimeTicks, *pEndNodeAnim);
        }

        if (pStartNodeAnim && !pEndNodeAnim) {
            // Fallback to start transform only
            Matrix4f ScalingM, RotationM, TranslationM;
            ScalingM.InitScaleTransform(StartTransform.Scaling.x, StartTransform.Scaling.y, StartTransform.Scaling.z);
            glm::mat4 RotationGLM = glm::mat4_cast(StartTransform.Rotation);
            RotationM = Matrix4f(RotationGLM);
            TranslationM.InitTranslationTransform(StartTransform.Translation.x, StartTransform.Translation.y, StartTransform.Translation.z);
            NodeTransformation = TranslationM * RotationM * ScalingM;
        } else if (!pStartNodeAnim && pEndNodeAnim) {
            // Fallback to end transform only
            Matrix4f ScalingM, RotationM, TranslationM;
            ScalingM.InitScaleTransform(EndTransform.Scaling.x, EndTransform.Scaling.y, EndTransform.Scaling.z);
            glm::mat4 RotationGLM = glm::mat4_cast(EndTransform.Rotation);
            RotationM = Matrix4f(RotationGLM);
            TranslationM.InitTranslationTransform(EndTransform.Translation.x, EndTransform.Translation.y, EndTransform.Translation.z);
            NodeTransformation = TranslationM * RotationM * ScalingM;
        } else if (pStartNodeAnim && pEndNodeAnim) {
            // Interpolate scaling
            const glm::vec3& Scale0 = StartTransform.Scaling;
            const glm::vec3& Scale1 = EndTransform.Scaling;
            glm::vec3 BlendedScaling = (1.0f - BlendFactor) * Scale0 + Scale1 * BlendFactor;
            Matrix4f ScalingM;
            ScalingM.InitScaleTransform(BlendedScaling.x, BlendedScaling.y, BlendedScaling.z);

            // Interpolate rotation
            const glm::quat& Rot0 = StartTransform.Rotation;
            const glm::quat& Rot1 = EndTransform.Rotation;
            glm::quat BlendedRot = glm::slerp(Rot0, Rot1, BlendFactor);
            Matrix4f RotationM = Matrix4f(glm::mat4_cast(BlendedRot));

            // Interpolate translation
            const glm::vec3& Pos0 = StartTransform.Translation;
            const glm::vec3& Pos1 = EndTransform.Translation;
            glm::vec3 BlendedTranslation = (1.0f - BlendFactor) * Pos0 + Pos1 * BlendFactor;
            Matrix4f TranslationM;
            TranslationM.InitTranslationTransform(BlendedTranslation.x, BlendedTranslation.y, BlendedTranslation.z);

            // Combine it all
            NodeTransformation = TranslationM * RotationM * ScalingM;
        }

        if (Node.m_parentIndex >= 0) {
            GlobalTransforms[NodeIndex] = GlobalTransforms[Node.m_parentIndex] * NodeTransformation;
        } else {
            GlobalTransforms[NodeIndex] = NodeTransformation;
        }

        if (m_modelData.m_boneNameToIndexMap.find(NodeName) != m_modelData.m_boneNameToIndexMap.end()) {
            uint BoneIndex = m_modelData.m_boneNameToIndexMap[NodeName];
            m_modelData.m_boneInfo[BoneIndex].FinalTransformation = m_modelData.m_globalInverseTransform *
                                                                    GlobalTransforms[NodeIndex] *
                                                                    m_modelData.m_boneInfo[BoneIndex].OffsetMatrix;
        }
    }
}


void CoreModel::CalcLocalTransform(LocalTransform& Transform, float AnimationTimeTicks, const AnimChannel& NodeAnim)
{
    CalcInterpolatedScaling(Transform.Scaling, AnimationTimeTicks, NodeAnim);
    CalcInterpolatedRotation(Transform.Rotation, AnimationTimeTicks, NodeAnim);
    CalcInterpolatedPosition(Transform.Translation, AnimationTimeTicks, NodeAnim);
}


void CoreModel::GetBoneTransforms(float TimeInSeconds, std::vector<Matrix4f>& Transforms, unsigned int AnimationIndex)
{
    if (AnimationIndex >= m_modelData.m_animations.size()) {
        printf("Invalid animation index %d, max is %d\n", AnimationIndex, (int)(m_modelData.m_animations.size() - 1));
        assert(0);
    }

    float AnimationTimeTicks = CalcAnimationTimeTicks(TimeInSeconds, AnimationIndex);
    const AnimationClip& Animation = m_modelData.m_animations[AnimationIndex];

    ReadNodeHierarchy(AnimationTimeTicks, Animation);
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
    if (StartAnimIndex >= m_modelData.m_animations.size()) {
        printf("Invalid start animation index %d, max is %d\n", StartAnimIndex, (int)(m_modelData.m_animations.size() - 1));
        assert(0);
    }

    if (EndAnimIndex >= m_modelData.m_animations.size()) {
        printf("Invalid end animation index %d, max is %d\n", EndAnimIndex, (int)(m_modelData.m_animations.size() - 1));
        assert(0);
    }

    if ((BlendFactor < 0.0f) || (BlendFactor > 1.0f)) {
        printf("Invalid blend factor %f\n", BlendFactor);
        assert(0);
    }

    float StartAnimationTimeTicks = CalcAnimationTimeTicks(TimeInSeconds, StartAnimIndex);
    float EndAnimationTimeTicks = CalcAnimationTimeTicks(TimeInSeconds, EndAnimIndex);

    const AnimationClip& StartAnimation = m_modelData.m_animations[StartAnimIndex];
    const AnimationClip& EndAnimation = m_modelData.m_animations[EndAnimIndex];

    ReadNodeHierarchyBlended(StartAnimationTimeTicks, EndAnimationTimeTicks, StartAnimation, EndAnimation, BlendFactor);

    BlendedTransforms.resize(m_modelData.m_boneInfo.size());

    for (uint i = 0 ; i < m_modelData.m_boneInfo.size() ; i++) {
        BlendedTransforms[i] = m_modelData.m_boneInfo[i].FinalTransformation;
    }
}


float CoreModel::CalcAnimationTimeTicks(float TimeInSeconds, unsigned int AnimationIndex) 
{
    float TicksPerSecond = (float)(m_modelData.m_animations[AnimationIndex].m_ticksPerSecond != 0 ? 
                                   m_modelData.m_animations[AnimationIndex].m_ticksPerSecond : 25.0f);

    float TimeInTicks = TimeInSeconds * TicksPerSecond;

    // we need to use the integral part of mDuration for the total length of the animation
    float Duration = 0.0f;
    modf((float)m_modelData.m_animations[AnimationIndex].m_durationTicks, &Duration);

    if (Duration <= 0.0f) {
        return 0.0f;
    }

    return std::fmod(TimeInTicks, Duration);
}


bool CoreModel::IsAnimated() const
{
    bool ret = m_modelData.IsSkinned();

    if (ret && (NumBones() == 0)) {
        printf("Animations without bones? need to check this\n");
        assert(0);
    }

    return ret;
}