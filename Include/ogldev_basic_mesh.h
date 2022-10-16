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

#ifndef OGLDEV_BASIC_MESH_H
#define OGLDEV_BASIC_MESH_H

#include <map>
#include <vector>
#include <GL/glew.h>

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>       // Output data structure
#include <assimp/postprocess.h> // Post processing flags

#include "ogldev_util.h"
#include "ogldev_math_3d.h"
#include "ogldev_texture.h"
#include "ogldev_world_transform.h"
#include "ogldev_material.h"
#include "ogldev_mesh_common.h"

#define INVALID_MATERIAL 0xFFFFFFFF

class BasicMesh : public MeshCommon
{
public:
    BasicMesh() {};

    ~BasicMesh();

    bool LoadMesh(const std::string& Filename);

    void Render(IRenderCallbacks* pRenderCallbacks = NULL);

    void Render(uint DrawIndex, uint PrimID);

    void Render(uint NumInstances, const Matrix4f* WVPMats, const Matrix4f* WorldMats);

    const Material& GetMaterial();

    PBRMaterial& GetPBRMaterial() { return m_Materials[0].PBRmaterial; };

    void GetLeadingVertex(uint DrawIndex, uint PrimID, Vector3f& Vertex);

protected:

    void Clear();

    virtual void ReserveSpace(uint NumVertices, uint NumIndices);

    virtual void InitSingleMesh(uint MeshIndex, const aiMesh* paiMesh);

    virtual void PopulateBuffers();

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
    };

    std::vector<BasicMeshEntry> m_Meshes;

    const aiScene* m_pScene;

    Matrix4f m_GlobalInverseTransform;

private:

    bool InitFromScene(const aiScene* pScene, const std::string& Filename);

    void CountVerticesAndIndices(const aiScene* pScene, uint& NumVertices, uint& NumIndices);

    void InitAllMeshes(const aiScene* pScene);

    bool InitMaterials(const aiScene* pScene, const std::string& Filename);

    void LoadTextures(const string& Dir, const aiMaterial* pMaterial, int index);

    void LoadDiffuseTexture(const string& Dir, const aiMaterial* pMaterial, int index);
    void LoadDiffuseTextureEmbedded(const aiTexture* paiTexture, int MaterialIndex);
    void LoadDiffuseTextureFromFile(const string& dir, const aiString& Path, int MaterialIndex);

    void LoadSpecularTexture(const string& Dir, const aiMaterial* pMaterial, int index);
    void LoadSpecularTextureEmbedded(const aiTexture* paiTexture, int MaterialIndex);
    void LoadSpecularTextureFromFile(const string& dir, const aiString& Path, int MaterialIndex);

    void LoadColors(const aiMaterial* pMaterial, int index);

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

    std::vector<Material> m_Materials;

    // Temporary space for vertex stuff before we load them into the GPU
    vector<Vector3f> m_Positions;
    vector<Vector3f> m_Normals;
    vector<Vector2f> m_TexCoords;
    vector<uint> m_Indices;

    Assimp::Importer m_Importer;
};


#endif  /* OGLDEV_BASIC_MESH_H */
