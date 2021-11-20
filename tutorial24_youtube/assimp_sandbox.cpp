#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>       // Output data structure
#include <assimp/postprocess.h> // Post processing flags


#define ASSIMP_LOAD_FLAGS (aiProcess_Triangulate | aiProcess_GenNormals |  aiProcess_JoinIdenticalVertices )


void parse_single_bone(int bone_index, const aiBone* pBone)
{
    printf(" Bone %d: '%s'\n", bone_index, pBone->mName.C_Str());
    printf("   Num vertices affected by this bone: %d\n", pBone->mNumWeights);
}


void parse_mesh_bones(const aiMesh* pMesh)
{
    printf("*******************************************************\n");
    printf("Parsing %d bones in mesh '%s'\n", pMesh->mNumBones, pMesh->mName.C_Str());

    for (int i = 0 ; i < pMesh->mNumBones ; i++) {
        parse_single_bone(i, pMesh->mBones[i]);
    }
}


void parse_bones(const aiScene* pScene)
{
    for (uint i = 0 ; i < pScene->mNumMeshes ; i++) {
        const aiMesh* pMesh = pScene->mMeshes[i];

        if (pMesh->HasBones()) {
            parse_mesh_bones(pMesh);
        }
    }
    printf("\n");
}


void parse_meshes(const aiScene* pScene)
{
    printf("*******************************************************\n");
    printf("Parsing %d meshes\n\n", pScene->mNumMeshes);

    uint total_vertices = 0;
    uint total_indices = 0;

    for (int i = 0 ; i < pScene->mNumMeshes ; i++) {
        int num_vertices = pScene->mMeshes[i]->mNumVertices;
        int num_indices = pScene->mMeshes[i]->mNumFaces * 3;
        printf("Mesh %s: vertices %d indices %d\n", pScene->mMeshes[i]->mName.C_Str(), num_vertices, num_indices);
        total_vertices += num_vertices;
        total_indices  += num_indices;
    }

    printf("\nTotal vertices %d total indices %d\n", total_vertices, total_indices);
}


void parse_scene(const aiScene* pScene)
{
    parse_meshes(pScene);

    parse_bones(pScene);
}

int main(int argc, char* argv[])
{
    if (argc != 2) {
        printf("Usage: %s <model filename>\n", argv[0]);
        return 1;
    }

    char* filename = argv[1];
    Assimp::Importer Importer;
    const aiScene* pScene = Importer.ReadFile(filename, ASSIMP_LOAD_FLAGS);

    if (!pScene) {
        printf("Error parsing '%s': '%s'\n", filename, Importer.GetErrorString());
        return 1;
    }

    parse_scene(pScene);

    return 0;
}
