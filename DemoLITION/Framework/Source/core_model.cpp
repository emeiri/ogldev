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
        ret = m_modelData.m_materials[0].m_isPBR;
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
    m_tempGlobalTransforms.resize(m_modelData.m_nodes.size());

    InitMaterials();

    InitGPUBuffers();

    InitCameras();

    CalculateMeshTransformations();

    InitGeometryPost();
}


void CoreModel::InitGPUBuffers()
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

    int WindowWidth = 0, WindowHeight = 0;
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
            
            m_Materials[MaterialIndex].pTextures[TexType] = NULL;

            TextureSource& TexSource = m_modelData.m_materials[MaterialIndex].m_textures[TexType];

            if (!TexSource.IsValid()) {
                continue;
            }

            bool IsSRGB = TexSource.IsSRGB;
            
            if (TexSource.EmbeddedBytes.size() > 0) {            
                LoadTextureEmbedded(MaterialIndex, (TEXTURE_TYPE)TexType, IsSRGB);
            } else {               
                const std::string& Path = TexSource.Path;
                LoadTextureFromFile(Path, MaterialIndex, (TEXTURE_TYPE)TexType, IsSRGB);
            }
        }

        m_Materials[MaterialIndex].m_pMaterialData = &m_modelData.m_materials[MaterialIndex];
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

    if (Node.m_numInstances == 0) {
        OGLDEV_ERROR("Node %s has no meshes\n", pNodeName);
        assert(0);
    }

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

static glm::vec3 Interpolate(const glm::vec3& Start, const glm::vec3& End, float Factor)
{
    return Start + Factor * (End - Start);
}


static glm::quat Interpolate(const glm::quat& Start, const glm::quat& End, float Factor)
{
    return glm::normalize(glm::slerp(Start, End, Factor));
}


// Samples a key track at the given time. An empty track returns the fallback
// (the bind-pose value), times outside the track are clamped to the end keys.
template<typename KeyT, typename T>
static T Sample(const std::vector<KeyT>& Keys, float Time, const T& Fallback)
{
    if (Keys.empty()) {
        return Fallback;
    }

    if ((Keys.size() == 1) || (Time <= Keys.front().m_time)) {
        return Keys.front().m_value;
    }

    if (Time >= Keys.back().m_time) {
        return Keys.back().m_value;
    }

    // Binary search for the last key with m_time <= Time. The checks above
    // guarantee Keys[Lo].m_time <= Time < Keys[Hi].m_time throughout.
    size_t Lo = 0;
    size_t Hi = Keys.size() - 1;

    while (Hi - Lo > 1) {
        size_t Mid = (Lo + Hi) / 2;

        if (Keys[Mid].m_time <= Time) {
            Lo = Mid;
        } else {
            Hi = Mid;
        }
    }

    float DeltaTime = Keys[Hi].m_time - Keys[Lo].m_time;
    float Factor = (DeltaTime > 0.0f) ? (Time - Keys[Lo].m_time) / DeltaTime : 0.0f;

    return Interpolate(Keys[Lo].m_value, Keys[Hi].m_value, Factor);
}


// Splits a TRS matrix (no shear) into scaling, rotation and translation
static void DecomposeTransform(const Matrix4f& m, glm::vec3& Scaling, glm::quat& Rotation, glm::vec3& Translation)
{
    // Matrix4f is row major, the translation is in the last column
    Translation = glm::vec3(m.m[0][3], m.m[1][3], m.m[2][3]);

    glm::mat3 R;
    for (int c = 0; c < 3; c++) {
        glm::vec3 Axis(m.m[0][c], m.m[1][c], m.m[2][c]);
        Scaling[c] = glm::length(Axis);
        R[c] = (Scaling[c] > 0.0f) ? Axis / Scaling[c] : Axis;
    }

    Rotation = glm::normalize(glm::quat_cast(R));
}


void CoreModel::ReadNodeHierarchy(float AnimationTimeTicks, const AnimationClip& Animation, std::vector<Matrix4f>& Transforms)
{
    Transforms.resize(m_modelData.m_boneInfo.size());

    // The nodes are stored parent-first so the parent's global transform is always ready
    for (uint NodeIndex = 0; NodeIndex < m_modelData.m_nodes.size(); NodeIndex++) {
        const SkelNode& Node = m_modelData.m_nodes[NodeIndex];
        std::string NodeName(Node.m_name);

        Matrix4f NodeTransformation(Node.m_localTransform);

        const AnimChannel* pNodeAnim = Animation.FindNodeAnim(NodeIndex);

        if (pNodeAnim) {
            LocalTransform Transform;
            CalcLocalTransform(Transform, AnimationTimeTicks, *pNodeAnim, Node.m_localTransform);

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
            m_tempGlobalTransforms[NodeIndex] = m_tempGlobalTransforms[Node.m_parentIndex] * NodeTransformation;
        } else {
            m_tempGlobalTransforms[NodeIndex] = NodeTransformation;
        }

        int BoneIndex = Node.m_boneIndex;

        if (BoneIndex >= 0) {
            Transforms[BoneIndex] = m_modelData.m_globalInverseTransform *
                                    m_tempGlobalTransforms[NodeIndex] *
                                    m_modelData.m_boneInfo[BoneIndex].OffsetMatrix;
        }
    }
}


void CoreModel::ReadNodeHierarchyBlended(float StartAnimationTimeTicks, 
                                         float EndAnimationTimeTicks,
                                         const AnimationClip& StartAnimation, 
                                         const AnimationClip& EndAnimation, 
                                         float BlendFactor,
                                         std::vector<Matrix4f>& BlendedTransforms)
                        
{
    BlendedTransforms.resize(m_modelData.m_boneInfo.size());

    for (uint NodeIndex = 0; NodeIndex < m_modelData.m_nodes.size(); NodeIndex++) {
        const SkelNode& Node = m_modelData.m_nodes[NodeIndex];
        std::string NodeName(Node.m_name);

        Matrix4f NodeTransformation(Node.m_localTransform);

        const AnimChannel* pStartNodeAnim = StartAnimation.FindNodeAnim(NodeIndex);
   
        LocalTransform StartTransform;

        if (pStartNodeAnim) {
            CalcLocalTransform(StartTransform, StartAnimationTimeTicks, *pStartNodeAnim, Node.m_localTransform);
        }

        const AnimChannel* pEndNodeAnim = EndAnimation.FindNodeAnim(NodeIndex);
        LocalTransform EndTransform;
        if (pEndNodeAnim) {
            CalcLocalTransform(EndTransform, EndAnimationTimeTicks, *pEndNodeAnim, Node.m_localTransform);
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
            m_tempGlobalTransforms[NodeIndex] = m_tempGlobalTransforms[Node.m_parentIndex] * NodeTransformation;
        } else {
            m_tempGlobalTransforms[NodeIndex] = NodeTransformation;
        }

        int BoneIndex = Node.m_boneIndex;

        if (BoneIndex >= 0) {
            BlendedTransforms[BoneIndex] = m_modelData.m_globalInverseTransform *
                                           m_tempGlobalTransforms[NodeIndex] *
                                           m_modelData.m_boneInfo[BoneIndex].OffsetMatrix;
        }
    }
}


void CoreModel::CalcLocalTransform(LocalTransform& Transform, float AnimationTimeTicks, const AnimChannel& NodeAnim,
                                   const Matrix4f& BindPoseTransform)
{
    // A channel may animate only some of the components (e.g. rotation only),
    // the missing ones keep their bind pose value
    LocalTransform BindPose;
    DecomposeTransform(BindPoseTransform, BindPose.Scaling, BindPose.Rotation, BindPose.Translation);

    Transform.Scaling     = Sample(NodeAnim.m_scalings,  AnimationTimeTicks, BindPose.Scaling);
    Transform.Rotation    = Sample(NodeAnim.m_rotations, AnimationTimeTicks, BindPose.Rotation);
    Transform.Translation = Sample(NodeAnim.m_positions, AnimationTimeTicks, BindPose.Translation);
}


void CoreModel::GetBoneTransforms(float TimeInSeconds, std::vector<Matrix4f>& Transforms, unsigned int AnimationIndex)
{
    if (AnimationIndex >= m_modelData.m_animations.size()) {
        printf("Invalid animation index %d, max is %d\n", AnimationIndex, (int)(m_modelData.m_animations.size() - 1));
        assert(0);
    }

    float AnimationTimeTicks = CalcAnimationTimeTicks(TimeInSeconds, AnimationIndex);
    const AnimationClip& Animation = m_modelData.m_animations[AnimationIndex];

    ReadNodeHierarchy(AnimationTimeTicks, Animation, Transforms);
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

    ReadNodeHierarchyBlended(StartAnimationTimeTicks, 
                             EndAnimationTimeTicks, 
                             StartAnimation, 
                             EndAnimation, 
                             BlendFactor,
                             BlendedTransforms);
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