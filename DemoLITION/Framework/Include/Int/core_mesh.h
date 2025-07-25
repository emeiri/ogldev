#pragma once

#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "ogldev_types.h"


#define MAX_LOD_COUNT 8

struct Mesh {
    u32 m_lodCount = 1;
    u32 m_indexOffset = 0;
    u32 m_vertexOffset = 0;
    u32 m_vertexCount = 0;
    u32 m_lodOffsets[MAX_LOD_COUNT + 1] = { 0 };
    u32 m_materialID = 0;

    u32 GetLodIndicesCount(u32 Lod)
    {
        assert(Lod <= MAX_LOD_COUNT);

        u32 ret = 0;

        if (Lod < m_lodCount) {
            ret = m_lodOffsets[Lod + 1] - m_lodOffsets[Lod];
        }

        return ret;
    }
};


struct MeshFileHeader {
    u32 m_magicValue = 0x12345678;
    u32 m_meshCount = 0;
    u32 m_indexDataSizeBytes = 0;
    u32 m_vertexDataSizeBytes = 0;
};


struct BoundingBox {
    glm::vec3 m_min;
    glm::vec3 m_max;

    BoundingBox() = default;

    BoundingBox(const glm::vec3& min, const glm::vec3& max)
        : m_min(glm::min(min, max))
        , m_max(glm::max(min, max))
    {
    }

    BoundingBox(const glm::vec3* points, size_t numPoints)
    {
        glm::vec3 vmin(std::numeric_limits<float>::max());
        glm::vec3 vmax(std::numeric_limits<float>::lowest());

        for (size_t i = 0; i != numPoints; i++) {
            vmin = glm::min(vmin, points[i]);
            vmax = glm::max(vmax, points[i]);
        }

        m_min = vmin;
        m_max = vmax;
    }

    glm::vec3 GetSize() const
    {
        return glm::vec3(m_max[0] - m_min[0],
            m_max[1] - m_min[1],
            m_max[2] - m_min[2]);
    }

    glm::vec3 GetCenter() const
    {
        return 0.5f * glm::vec3(m_max[0] + m_min[0],
            m_max[1] + m_min[1],
            m_max[2] + m_min[2]);
    }


    void Transform(const glm::mat4& t)
    {
        glm::vec3 corners[] = {
          glm::vec3(m_min.x, m_min.y, m_min.z),
          glm::vec3(m_min.x, m_max.y, m_min.z),
          glm::vec3(m_min.x, m_min.y, m_max.z),
          glm::vec3(m_min.x, m_max.y, m_max.z),
          glm::vec3(m_max.x, m_min.y, m_min.z),
          glm::vec3(m_max.x, m_max.y, m_min.z),
          glm::vec3(m_max.x, m_min.y, m_max.z),
          glm::vec3(m_max.x, m_max.y, m_max.z),
        };

        for (auto& v : corners) {
            v = glm::vec3(t * glm::vec4(v, 1.0f));
        }

        *this = BoundingBox(corners, 8);
    }

    BoundingBox GetTransformed(const glm::mat4& t) const
    {
        BoundingBox b = *this;
        b.Transform(t);
        return b;
    }

    void CombinePoint(const glm::vec3& p)
    {
        m_min = glm::min(m_min, p);
        m_max = glm::max(m_max, p);
    }
};


enum MaterialFlags {
    MaterialFlags_CastShadow = 0x1,
    MaterialFlags_ReceiveShadow = 0x2,
    MaterialFlags_Transparent = 0x4,
};


struct MeshMaterial {
    glm::vec4 m_emissiveFactor = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    glm::vec4 m_baseColorFactor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    float m_roughness = 1.0f;
    float m_transparencyFactor = 1.0f;
    float m_alphaTest = 0.0f;
    float m_metallicFactor = 0.0f;
    // index into MeshData::textureFiles
    int m_baseColorTexture = -1;
    int m_emissiveTexture = -1;
    int m_normalTexture = -1;
    int m_opacityTexture = -1;
    u32 m_flags = MaterialFlags_CastShadow | MaterialFlags_ReceiveShadow;
};


struct MeshData {
    //VertexInput streams = {};
    std::vector<u32> m_indexData;
    std::vector<u32> m_vertexData;
    std::vector<Mesh> m_meshes;
    std::vector<BoundingBox> m_boxes;
    std::vector<MeshMaterial> m_materials;
    std::vector<std::string> m_textureFiles;

    MeshFileHeader GetMeshFileHeader() const
    {
        MeshFileHeader m = {
          .m_meshCount = (u32)m_meshes.size(),
          .m_indexDataSizeBytes = (u32)(m_indexData.size() * sizeof(u32)),
          .m_vertexDataSizeBytes = (u32)m_vertexData.size(),
        };

        return m;
    }
};


MeshFileHeader LoadMeshData(const char* meshFile, MeshData& out);