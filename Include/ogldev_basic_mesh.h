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

//#define USE_MESH_OPTIMIZER

class BasicMesh : public MeshCommon
{
public:
    BasicMesh() {};

    ~BasicMesh();

    bool LoadMesh(const std::string& Filename, int AssimpFlags = ASSIMP_LOAD_FLAGS);

    void Render(IRenderCallbacks* pRenderCallbacks = NULL);

    void Render(uint DrawIndex, uint PrimID);

    void Render(uint NumInstances, const Matrix4f* WVPMats, const Matrix4f* WorldMats);

    const Material& GetMaterial();

    PBRMaterial& GetPBRMaterial() { return m_Materials[0].PBRmaterial; };

    void GetLeadingVertex(uint DrawIndex, uint PrimID, Vector3f& Vertex);

    void SetPBR(bool IsPBR) { m_isPBR = IsPBR; }

protected:

    void Clear();
    virtual void ReserveSpace(uint NumVertices, uint NumIndices);
    virtual void InitSingleMesh(uint MeshIndex, const aiMesh* paiMesh);
    virtual void InitSingleMeshOpt(uint MeshIndex, const aiMesh* paiMesh);
    virtual void PopulateBuffers();
    virtual void PopulateBuffersNonDSA();
    virtual void PopulateBuffersDSA();

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

    vector<uint> m_Indices;

    enum BUFFER_TYPE {
        INDEX_BUFFER = 0,
        VERTEX_BUFFER = 1,
        WVP_MAT_BUFFER = 2,  // required only for instancing
        WORLD_MAT_BUFFER = 3,  // required only for instancing
        NUM_BUFFERS = 4
    };

    GLuint m_VAO = 0;

    GLuint m_Buffers[NUM_BUFFERS] = { 0 };

private:
    struct Vertex {
        Vector3f Position;
        Vector2f TexCoords;
        Vector3f Normal;
    };

    bool InitFromScene(const aiScene* pScene, const std::string& Filename);
    void CountVerticesAndIndices(const aiScene* pScene, uint& NumVertices, uint& NumIndices);
    void InitAllMeshes(const aiScene* pScene);
    void OptimizeMesh(int MeshIndex, std::vector<uint>& Indices, std::vector<Vertex>& Vertices);
    bool InitMaterials(const aiScene* pScene, const std::string& Filename);
    void LoadTextures(const string& Dir, const aiMaterial* pMaterial, int index);

    void LoadDiffuseTexture(const string& Dir, const aiMaterial* pMaterial, int index);
    void LoadDiffuseTextureEmbedded(const aiTexture* paiTexture, int MaterialIndex);
    void LoadDiffuseTextureFromFile(const string& dir, const aiString& Path, int MaterialIndex);

    void LoadSpecularTexture(const string& Dir, const aiMaterial* pMaterial, int index);
    void LoadSpecularTextureEmbedded(const aiTexture* paiTexture, int MaterialIndex);
    void LoadSpecularTextureFromFile(const string& dir, const aiString& Path, int MaterialIndex);

    void LoadAlbedoTexture(const string& Dir, const aiMaterial* pMaterial, int index);
    void LoadAlbedoTextureEmbedded(const aiTexture* paiTexture, int MaterialIndex);
    void LoadAlbedoTextureFromFile(const string& dir, const aiString& Path, int MaterialIndex);

    void LoadMetalnessTexture(const string& Dir, const aiMaterial* pMaterial, int index);
    void LoadMetalnessTextureEmbedded(const aiTexture* paiTexture, int MaterialIndex);
    void LoadMetalnessTextureFromFile(const string& dir, const aiString& Path, int MaterialIndex);

    void LoadRoughnessTexture(const string& Dir, const aiMaterial* pMaterial, int index);
    void LoadRoughnessTextureEmbedded(const aiTexture* paiTexture, int MaterialIndex);
    void LoadRoughnessTextureFromFile(const string& dir, const aiString& Path, int MaterialIndex);

    void LoadColors(const aiMaterial* pMaterial, int index);

    void SetupRenderMaterialsPhong(unsigned int MeshIndex, unsigned int MaterialIndex, IRenderCallbacks* pRenderCallbacks);
    void SetupRenderMaterialsPBR();

    std::vector<Material> m_Materials;
    
    // Temporary space for vertex stuff before we load them into the GPU
    vector<Vertex> m_Vertices;

    Assimp::Importer m_Importer;

    bool m_isPBR = false;
};


#endif  /* OGLDEV_BASIC_MESH_H */
