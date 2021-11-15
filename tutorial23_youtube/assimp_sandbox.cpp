#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>       // Output data structure
#include <assimp/postprocess.h> // Post processing flags


#define ASSIMP_LOAD_FLAGS (aiProcess_Triangulate | aiProcess_GenNormals |  aiProcess_JoinIdenticalVertices )


void print_assimp_matrix_4x4(const aiMatrix4x4& m)
{
    printf("%f %f %f %f\n", m.a1, m.a2, m.a3, m.a4);
    printf("%f %f %f %f\n", m.b1, m.b2, m.b3, m.b4);
    printf("%f %f %f %f\n", m.c1, m.c2, m.c3, m.c4);
    printf("%f %f %f %f\n", m.d1, m.d2, m.d3, m.d4);
}


void count_vertices_and_indices(const aiScene* pScene)
{
    uint NumVertices = 0;
    uint NumIndices = 0;

    // Count the number of vertices and indices
    for (uint i = 0 ; i < pScene->mNumMeshes ; i++) {
        NumVertices += pScene->mMeshes[i]->mNumVertices;
        NumIndices  += pScene->mMeshes[i]->mNumFaces * 3;
    }

    printf("Num vertices %d num indices %d\n", NumVertices, NumIndices);
}


void parse_single_bone(int bone_index, const aiBone* pBone)
{
    printf(" Bone %d: '%s'\n", bone_index, pBone->mName.C_Str());
    printf("   Num vertices affected by this bone: %d\n", pBone->mNumWeights);
    printf("   Offset matrix\n");
    print_assimp_matrix_4x4(pBone->mOffsetMatrix);
}


void parse_bones(const aiMesh* paiMesh)
{
    printf("Parsing bones mesh '%s'\n", paiMesh->mName.C_Str());

    for (int i = 0 ; i < paiMesh->mNumBones ; i++) {
        parse_single_bone(i, paiMesh->mBones[i]);
    }
}

void load_assimp_scene(const aiScene* pScene)
{
    count_vertices_and_indices(pScene);

    for (uint i = 0 ; i < pScene->mNumMeshes ; i++) {
        const aiMesh* paiMesh = pScene->mMeshes[i];

        if (paiMesh->HasBones()) {
            parse_bones(paiMesh);
        }
    }
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

    load_assimp_scene(pScene);

    return 0;
}
