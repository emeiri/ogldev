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

#include <cstddef>
#include <map>
#include <vector>
#include <unordered_map>

#include "ogldev_math_3d.h"
#include "demolition_lights.h"

struct MeshDims {
    glm::vec3 Size = glm::vec3(0.0f);
    glm::vec3 BboxMin = glm::vec3(0.0f);
    glm::vec3 BboxMax = glm::vec3(0.0f);
};


struct BasicMeshEntry {
    uint NumIndices = 0;
    uint NumVertices = 0;
    uint BaseVertex = 0;
    uint BaseIndex = 0;
    uint ValidFaces = 0;
    int MaterialIndex = -1;
    std::string Name;
    MeshDims Dims;
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

struct BoneInfo
{
    Matrix4f OffsetMatrix;

    BoneInfo(const Matrix4f& Offset)
    {
        OffsetMatrix = Offset;
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
            assert(0);
            return;
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


// The vertex attribute setup in GLModel computes offsets by counting floats, so these
// structs must stay tightly packed. VertexBoneData::index is loader bookkeeping that
// still goes to the GPU (4 bytes per vertex) until the vertex pulling layout is settled.
static_assert(sizeof(Vertex) == 80, "Vertex must be 80 bytes");
static_assert(offsetof(Vertex, Color) == 64, "unexpected Vertex::Color offset");
static_assert(sizeof(VertexBoneData) == 36, "VertexBoneData must be 36 bytes");
static_assert(offsetof(VertexBoneData, Weights) == 16, "unexpected VertexBoneData::Weights offset");
static_assert(offsetof(VertexBoneData, index) == 32, "unexpected VertexBoneData::index offset");
static_assert(sizeof(SkinnedVertex) == 116, "SkinnedVertex must be 116 bytes");
static_assert(offsetof(SkinnedVertex, Color) == offsetof(Vertex, Color), "SkinnedVertex must start with the Vertex layout");
static_assert(offsetof(SkinnedVertex, Bones) == sizeof(Vertex), "SkinnedVertex::Bones must follow the Vertex fields");


struct TextureSource {
    std::string Path;                  // resolved full path, or empty
    std::vector<u8> EmbeddedBytes;     // non-empty for embedded textures
    bool IsSRGB = false;
    bool IsValid() const { return !Path.empty() || !EmbeddedBytes.empty(); }
};


enum MaterialTypeFlags : u32 {
    MaterialType_Invalid = 0,
    MaterialType_MetallicRoughness = 0x1,
    MaterialType_SpecularGlossiness = 0x2,
    MaterialType_Sheen = 0x4,
    MaterialType_ClearCoat = 0x8,
    MaterialType_Specular = 0x10,
    MaterialType_Transmission = 0x20,
    MaterialType_Volume = 0x40,
    MaterialType_Unlit = 0x80,
};


enum TEXTURE_TYPE {
    TEX_TYPE_BASE = 0,      // Base color / diffuse / albedo
    TEX_TYPE_SPECULAR = 1,
    TEX_TYPE_NORMAL = 2,
    TEX_TYPE_METALNESS = 3,
    TEX_TYPE_EMISSIVE = 4,
    TEX_TYPE_NORMAL_CAMERA = 5,
    TEX_TYPE_EMISSION_COLOR = 6,
    TEX_TYPE_ROUGHNESS = 7,
    TEX_TYPE_AMBIENT_OCCLUSION = 8,
    TEX_TYPE_CLEARCOAT = 9,
    TEX_TYPE_CLEARCOAT_ROUGHNESS = 10,
    TEX_TYPE_CLEARCOAT_NORMAL = 11,
    TEX_TYPE_NUM = 12
};


struct MaterialData {
    std::string m_name;
    bool m_isPBR = false;
    int m_materialType = MaterialType_Invalid;
    Vector4f m_ambientColor = Vector4f(0.0f);
    Vector4f m_diffuseColor = Vector4f(0.0f);
    Vector4f m_specularColor = Vector4f(0.0f);
    Vector4f m_baseColor = Vector4f(1.0f);
    Vector4f m_emissiveColor = Vector4f(0.0f);
    Vector4f m_metallicRoughnessNormalOcclusion = Vector4f(1.0f);
    Vector4f m_clearCoatTransmissionThickness = Vector4f(1.0f);
    float m_transparencyFactor = 1.0f;
    float m_alphaTest = 0.0f;
    float m_ior = 1.5f;
    TextureSource m_textures[TEX_TYPE_NUM];
};


struct SkelNode {
    std::string m_name;
    bool m_isRequired = false;
    int m_parentIndex = -1;
    Matrix4f m_localTransform;
    Matrix4f m_bindPoseGlobalTransform; // calculated once
    int m_boneIndex = -1;  // -1 = not a bone
    std::vector<int> m_meshes;
    int m_firstInstance = -1;
    int m_numInstances = 0;
};


struct MeshInstance {
    u32      m_meshIndex;   // geometry + material
    int      m_nodeIndex;   // flat node it came from
    Matrix4f m_worldTransform;
};


struct VectorKey {
    float m_time;
    glm::vec3 m_value;
};


struct QuatKey {
    float m_time;
    glm::quat m_value;
};


struct AnimChannel {
    std::string m_nodeName;
    std::vector<VectorKey> m_positions;
    std::vector<VectorKey> m_scalings;
    std::vector<QuatKey>   m_rotations;
};


struct AnimationClip {
    std::string m_name;
    float m_durationTicks = 0.0f;
    float m_ticksPerSecond = 25.0f;
    std::vector<AnimChannel> m_channels;
    std::vector<int> m_nodeToChannel;   // per flat node, -1 = not animated

    const AnimChannel* FindNodeAnim(int NodeIndex) const
    {
        assert(NodeIndex >= 0 && NodeIndex < (int)m_nodeToChannel.size());
        int ChannelIndex = m_nodeToChannel[NodeIndex];
        const AnimChannel* pChannel = NULL;
        if (ChannelIndex != -1) {
            assert(ChannelIndex >= 0 && ChannelIndex < (int)m_channels.size());
            pChannel = &m_channels[ChannelIndex];
        }
        return pChannel;
    }
};


struct CameraData {
    std::string m_name;
    glm::vec3 m_pos = glm::vec3(0.0f);
    glm::vec3 m_dir = glm::vec3(0.0f);
    glm::vec3 m_up = glm::vec3(0.0f, 1.0f, 0.0f);
    float m_zNear = 0.1f;
    float m_zFar = 1000.0f;
    float m_hFov = 45.0f;
};


struct ModelData {
    std::vector<BasicMeshEntry>   m_meshes;
    std::vector<Vertex>           m_vertices;          // used when !IsSkinned()
    std::vector<SkinnedVertex>    m_skinnedVertices;   // used when IsSkinned()
    std::vector<u32>              m_indices;
    std::vector<MeshInstance>     m_instances;
    std::vector<MaterialData>     m_materials;
    std::vector<DirectionalLight> m_dirLights;
    std::vector<PointLight>       m_pointLights;
    std::vector<SpotLight>        m_spotLights;
    std::vector<CameraData>       m_cameras;
    std::vector<AnimationClip>    m_animations;
    Matrix4f m_globalInverseTransform;
    std::vector<BoneInfo> m_boneInfo;
    std::vector<SkelNode> m_nodes;
    std::unordered_map<std::string, int> m_nodeMap;
    std::map<std::string, uint> m_boneNameToIndexMap;

    bool IsSkinned() const { return !m_skinnedVertices.empty(); }

    size_t NumVertices() const {
        size_t ret = 0;

        if (m_skinnedVertices.size() > 0) {
            ret = m_skinnedVertices.size();
        } else {
            ret = m_vertices.size();
        }

        return ret;
    }

    bool GetNodeWorldTransform(const char* Name, Matrix4f& Out) const
    {
        auto it = m_nodeMap.find(Name);
        if (it == m_nodeMap.end()) {
            printf("Warning! Cannot find a node for '%s'\n", Name);
            Out.InitIdentity();
            return false;
        }
        Out = m_nodes[it->second].m_bindPoseGlobalTransform;
        return true;
    }
};
