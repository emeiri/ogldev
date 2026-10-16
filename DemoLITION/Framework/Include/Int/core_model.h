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

#include <vector>
#include <string>

#include "Int/core_material.h"
#include "ogldev_glm_camera.h"
#include "demolition_lights.h"
#include "demolition_scene.h"
#include "demolition_model.h"
#include "Int/core_mesh.h"
#include "Int/assimp_model_loader.h"


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

    bool IsPBR() const;

    void DestroyModel();

    void LoadAssimpModel(const std::string& Filename)
    {
        ModelLoadFlags DefaultFlags;
        LoadAssimpModel(Filename, DefaultFlags);
    }

    void LoadAssimpModel(const std::string& Filename, const ModelLoadFlags& Flags);

    const std::vector<GLMCameraFirstPerson>& GetCameras() const { return m_cameras; }

    uint NumBones() const { return (uint)m_modelData.m_boneNameToIndexMap.size(); }

    // This is the main function to drive the animation. It receives the animation time
    // in seconds and a reference to a vector of transformation matrices (one matrix per bone).
    // It calculates the current transformation for each bone according to the current time
    // and updates the corresponding matrix in the vector. This must then be updated in the VS
    // to be accumulated for the final local position (see skinning.vs). The animation index
    // is an optional param which selects one of the animations.
    void GetBoneTransforms(float AnimationTimeSec, std::vector<Matrix4f>& Transforms, unsigned int AnimationIndex = 0);

    // Same as above but this one blends two animations together based on a blending factor
    void GetBoneTransformsBlended(float AnimationTimeSec,
                                  std::vector<Matrix4f>& Transforms,
                                  unsigned int StartAnimIndex,
                                  unsigned int EndAnimIndex,
                                  float BlendFactor);

    const std::vector<DirectionalLight>& GetDirLights() const { return m_modelData.m_dirLights; }
    const std::vector<SpotLight>& GetSpotLights() const { return m_modelData.m_spotLights; }
    const std::vector<PointLight>& GetPointLights() const { return m_modelData.m_pointLights; }

    void SetPointLight(int i, const PointLight& Light) 
    { 
        if (i < m_modelData.m_pointLights.size()) { 
            m_modelData.m_pointLights[i] = Light; 
        } 
    }

    void SetDirLight(int i, const DirectionalLight& Light)
    {
        if (i < m_modelData.m_dirLights.size()) {
            m_modelData.m_dirLights[i] = Light;
        }
    }

    void SetTextureScale(float Scale) { m_textureScale = Scale; }

    bool IsAnimated() const;

    const CoreMaterial* GetMaterialForMesh(int MeshIndex) const;

    virtual void SetColorTexture(int TextureHandle) { assert(0); }

    virtual void SetNormalMap(int TextureHandle) { assert(0); }

    virtual void SetHeightMap(int TextureHandle) { assert(0); }

    virtual void SetAmbientOcclusionMap(int TextureHandle) { assert(0); }

    virtual size_t GetNumMeshes() const { return m_modelData.m_meshes.size(); }

    glm::mat4& GetNodeTransformationGLM(const char* pNodeName);

    void SetNodePositionXZ(const char* pNodeName, float x, float z);

    const std::vector<glm::mat4>& GetTransformations() const { return m_transformationsGLM; }

    std::vector<glm::mat4>& GetTransformationsMutable() { return m_transformationsGLM; }

    const MeshDims& GetNodeDims(const char* pNodeName) const 
    { 
        auto it = m_modelData.m_nodeMap.find(pNodeName);
        if (it == m_modelData.m_nodeMap.end()) {
            printf("Invalid node name '%s'\n", pNodeName);
            assert(0);
        }

        int NodeIndex = it->second;

        const SkelNode& Node = m_modelData.m_nodes[NodeIndex];

        if (Node.m_meshes.size() > 1) {
            printf("Node '%s' has multiple meshes\n", pNodeName);
            assert(0);
        }

        int FirstMeshIndex = Node.m_meshes[0];

        return m_modelData.m_meshes[FirstMeshIndex].Dims; 
    }

    // Returns the current position of the node, including any changes the application
    // made to its transformation (e.g. SetNodePositionXZ). Left handed, like m_transformationsGLM.
    const glm::vec3 GetNodePos(const char* pNodeName) const
    {
        auto it = m_modelData.m_nodeMap.find(pNodeName);
        if (it == m_modelData.m_nodeMap.end()) {
            printf("Invalid node name '%s'\n", pNodeName);
            assert(0);
        }

        int NodeIndex = it->second;

        const SkelNode& Node = m_modelData.m_nodes[NodeIndex];

        if (Node.m_numInstances == 0) {
            printf("Node '%s' has no meshes\n", pNodeName);
            assert(0);
        }

        const glm::mat4& Transformation = m_transformationsGLM[Node.m_firstInstance];

        return glm::vec3(Transformation[3]);
    }

protected:

    // This is a hack for Vulkan that doesn't have a CoreWindowSystem pointer right now.
    virtual void GetWindowSize(int& Width, int& Height) const
    {
        assert(0);
    }

    virtual Texture* AllocTexture2D() = 0;

    virtual void DestroyTexture(Texture* pTexture) = 0;

    virtual void InitGeometryPost() {}

    ModelData m_modelData;
    std::vector<CoreMaterial> m_Materials;
    std::vector<GLMCameraFirstPerson> m_cameras;

    CoreRenderingSystem* m_pCoreRenderingSystem = NULL;

private:

    virtual void PopulateBuffersSkinned(std::vector<SkinnedVertex>& Vertices) = 0;

    virtual void PopulateBuffers(std::vector<Vertex>& Vertices) = 0;    

    void InitModel();

    void InitCameras();
    void InitMaterials();
    void LoadTextureEmbedded(int MaterialIndex, TEXTURE_TYPE TexType, bool IsSRGB);
    void LoadTextureFromFile(const std::string& Path, int MaterialIndex, TEXTURE_TYPE TexType, bool IsSRGB);

    void InitGPUBuffers(unsigned int NumVertices, unsigned int NumIndices);

    template<typename VertexType>
    void InitAllMeshes(const std::string& Filename, std::vector<VertexType>& Vertices);

    void CalculateMeshTransformations();

    std::vector<Matrix4f> m_transformations;
    std::vector<glm::mat4> m_transformationsGLM;
    float m_textureScale = 1.0f;
    bool m_flipTexCoordV = false;

    /////////////////////////////////////
	// Skeletal animation stuff
    /////////////////////////////////////

    void CalcInterpolatedScaling(glm::vec3& Out, float AnimationTime, const AnimChannel& NodeAnim);
    void CalcInterpolatedRotation(glm::quat& Out, float AnimationTime, const AnimChannel& NodeAnim);
    void CalcInterpolatedPosition(glm::vec3& Out, float AnimationTime, const AnimChannel& NodeAnim);
    uint FindScaling(float AnimationTime, const AnimChannel& NodeAnim);
    uint FindRotation(float AnimationTime, const AnimChannel& NodeAnim);
    uint FindPosition(float AnimationTime, const AnimChannel& NodeAnim);
    void ReadNodeHierarchy(float AnimationTime, const AnimationClip& Animation);
    void ReadNodeHierarchyBlended(float StartAnimationTimeTicks, float EndAnimationTimeTicks, 
                                  const AnimationClip &StartAnimation, const AnimationClip& EndAnimation, float BlendFactor);
    float CalcAnimationTimeTicks(float TimeInSeconds, unsigned int AnimationIndex);

    struct LocalTransform {
        glm::vec3 Scaling;
        glm::quat Rotation;
        glm::vec3 Translation;
    };

    void CalcLocalTransform(LocalTransform& Transform, float AnimationTimeTicks, const AnimChannel& NodeAnim);
};

