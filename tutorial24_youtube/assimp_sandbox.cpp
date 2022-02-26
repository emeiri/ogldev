#include <assimp/Importer.hpp>  // C++ importer interface
#include <assimp/scene.h>       // Output data structure
#include <assimp/postprocess.h> // Post processing flags


void parse_single_bone(int bone_index, const aiBone* pBone)
{
    printf("      Bone %d: '%s' num vertices affected by this bone: %d\n", bone_index, pBone->mName.C_Str(), pBone->mNumWeights);

    for (unsigned int i = 0 ; i < pBone->mNumWeights ; i++) {
        if (i == 0) printf("\n");
        const aiVertexWeight& vw = pBone->mWeights[i];
        printf("       %d: vertex id %d weight %.2f\n", i, vw.mVertexId, vw.mWeight);
    }

    printf("\n");
}


void parse_mesh_bones(const aiMesh* pMesh)
{
    for (unsigned int i = 0 ; i < pMesh->mNumBones ; i++) {
        parse_single_bone(i, pMesh->mBones[i]);
    }
}


void parse_meshes(const aiScene* pScene)
{
    printf("*******************************************************\n");
    printf("Parsing %d meshes\n\n", pScene->mNumMeshes);

    int total_vertices = 0;
    int total_indices = 0;
    int total_bones = 0;

    for (unsigned int i = 0 ; i < pScene->mNumMeshes ; i++) {
        const aiMesh* pMesh = pScene->mMeshes[i];
        int num_vertices = pMesh->mNumVertices;
        int num_indices = pMesh->mNumFaces * 3;
        int num_bones = pMesh->mNumBones;
        printf("  Mesh %d '%s': vertices %d indices %d bones %d\n\n", i, pMesh->mName.C_Str(), num_vertices, num_indices, num_bones);
        total_vertices += num_vertices;
        total_indices  += num_indices;
        total_bones += num_bones;

        if (pMesh->HasBones()) {
            parse_mesh_bones(pMesh);
        }

        printf("\n");
    }

    printf("\nTotal vertices %d total indices %d total bones %d\n", total_vertices, total_indices, total_bones);
}


void parse_scene(const aiScene* pScene)
{
    parse_meshes(pScene);
}

#define ASSIMP_LOAD_FLAGS (aiProcess_Triangulate | aiProcess_GenNormals |  aiProcess_JoinIdenticalVertices )

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
