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

#include <vector>

#include "ogldev_math_3d.h"
#include "demolition_scene.h"
#include "Int/model_data.h"

struct aiNode;
struct aiBone;
struct aiMesh;
struct aiLight;
struct aiScene;
struct aiMaterial;
struct aiNodeAnim;
namespace Assimp {
    class Importer;
}



class AssimpModelLoader {

public:

    AssimpModelLoader() = default;
    void Load(const std::string& Filename, const ModelLoadFlags& Flags, ModelData& OutModelData);
    const std::string& GetError() const { return m_error; }

private:
    void InitFromScene();
    void InitMaterials();
    void InitGeometry();
    void InitAnimations();
    void InitSingleAnimation(int Index);
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
    Vector3f LocalToLeftHandedWorld(const Matrix4f& NodeWorld, const aiVector3D& v, float w) const;

    void LoadMeshBones(std::vector<SkinnedVertex>& SkinnedVertices, uint MeshIndex, const aiMesh* pMesh, uint VertexOffset);
    void LoadSingleBone(std::vector<SkinnedVertex>& SkinnedVertices, const aiBone* pBone, uint VertexOffset);
    int GetBoneId(const aiBone* pBone);
    void MarkRequiredNodesForBone(int NodeIndex);

    // Lights
    void InitLights();
    void InitSingleLight(const aiLight& light);
    void InitDirectionalLight(const aiLight& light);
    void InitPointLight(const aiLight& light);
    void InitSpotLight(const aiLight& light);

    // Materials
    void LoadTextures(const std::string& Dir, const aiMaterial* pMaterial, int index);
    void LoadDiffuseTexture(const std::string& Dir, const aiMaterial* pMaterial, int index);
    void LoadSpecularTexture(const std::string& Dir, const aiMaterial* pMaterial, int index);
    void LoadNormalTexture(const std::string& Dir, const aiMaterial* pMaterial, int MaterialIndex);
    void LoadMetalnessTexture(const std::string& Dir, const aiMaterial* pMaterial, int MaterialIndex);
    void LoadEmissiveTexture(const std::string& Dir, const aiMaterial* pMaterial, int MaterialIndex);
    void LoadEmissionColorTexture(const std::string& Dir, const aiMaterial* pMaterial, int MaterialIndex);
    void LoadNormalCameraTexture(const std::string& Dir, const aiMaterial* pMaterial, int MaterialIndex);
    void LoadRoughnessTexture(const std::string& Dir, const aiMaterial* pMaterial, int MaterialIndex);
    void LoadAmbientOcclusionTexture(const std::string& Dir, const aiMaterial* pMaterial, int MaterialIndex);
    void LoadClearCoatTexture(const std::string& Dir, const aiMaterial* pMaterial, int MaterialIndex);
    void LoadClearCoatRoughnessTexture(const std::string& Dir, const aiMaterial* pMaterial, int MaterialIndex);
    void LoadClearCoatNormalTexture(const std::string& Dir, const aiMaterial* pMaterial, int MaterialIndex);
    void LoadTexture(const std::string& Dir, const aiMaterial* pMaterial, int MaterialIndex, int AssimpTextureType, int AssimpTexIndex, TEXTURE_TYPE MyType);

    // Colors
    void LoadColors(const aiMaterial* pMaterial, MaterialData& MyMaterial);
    void ProcessClearCoat(const aiMaterial* pMaterial, MaterialData& MyMaterial);
    void LoadColor(const aiMaterial* pMaterial, Vector4f& Color, const char* pAiMatKey, int AiMatType, int AiMatIdx, const char* pName);

    // Skeleton
    void InitSkeletonHierarchy();

    void InitSingleNodeAnimation(AnimationClip& MyAnim, int ChannelIndex, AnimChannel& MyChannel, const aiNodeAnim* pNodeAnim);

    std::string m_error;
    std::string m_filename;
    const aiScene* m_pScene = NULL;
    ModelData* m_pModelData = NULL;
    bool m_convertToLeftHanded = false;
};

