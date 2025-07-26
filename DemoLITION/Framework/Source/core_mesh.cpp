#include "Int/core_mesh.h"


MeshFileHeader LoadMeshData(const char* pMeshFile, MeshData& out)
{
    FILE* f = NULL;

    errno_t err = fopen_s(&f, pMeshFile, "rb");

    //assert(f);

    if (!f) {
        printf("Cannot open '%s'.\n", pMeshFile);
        assert(false);
        exit(EXIT_FAILURE);
    }

    MeshFileHeader header;

    size_t Size = fread(&header, 1, sizeof(header), f);

    if (Size != sizeof(header)) {
        printf("Unable to read mesh file header.\n");
        assert(false);
        exit(EXIT_FAILURE);
    }

    out.m_vertexSize = header.m_vertexSizeBytes;
    out.m_totalIndices = header.m_numIndices;
    out.m_totalVertices = header.m_numVertices;

   /* if (fread(&out.streams, 1, sizeof(out.streams), f) != sizeof(out.streams)) {
        printf("Unable to read vertex streams description.\n");
        assert(false);
        exit(EXIT_FAILURE);
    }*/

    out.m_meshes.resize(header.m_meshCount);

    Size = fread(out.m_meshes.data(), sizeof(Mesh), header.m_meshCount, f);

    if (Size != header.m_meshCount) {
        printf("Could not read mesh descriptors.\n");
        assert(false);
        exit(EXIT_FAILURE);
    }

    out.m_boxes.resize(header.m_meshCount);

    Size = fread(out.m_boxes.data(), sizeof(BoundingBox), header.m_meshCount, f);

    if (Size != header.m_meshCount) {
        printf("Could not read bounding m_boxes.\n");
        assert(false);
        exit(EXIT_FAILURE);
    }

    out.m_indexData.Size = header.m_indexDataSizeBytes;
    out.m_indexData.pMem = (char*)malloc(header.m_indexDataSizeBytes);

    Size = fread(out.m_indexData.pMem, 1, header.m_indexDataSizeBytes, f);

    if (Size != header.m_indexDataSizeBytes) {
        printf("Unable to read index data.\n");
        assert(false);
        exit(EXIT_FAILURE);
    }

    out.m_vertexData.Size = header.m_vertexDataSizeBytes;
    out.m_vertexData.pMem = (char*)malloc(header.m_vertexDataSizeBytes);

    Size = fread(out.m_vertexData.pMem, 1, header.m_vertexDataSizeBytes, f);

    if (Size != header.m_vertexDataSizeBytes) {
        printf("Unable to read vertex data.\n");
        assert(false);
        exit(EXIT_FAILURE);
    }

   // out.m_meshes[0].m_numVertices = header.m_vertexDataSizeBytes / header.m_vertexSizeBytes;
   // out.m_meshes[0].m_lodOffsets[0] = 0;
   // out.m_meshes[0].m_lodOffsets[1] = header.m_indexDataSizeBytes / sizeof(u32);

    fclose(f);

    return header;
}


void SaveMeshData(const char* pfileName, const MeshData& m)
{
    FILE* f = NULL;
    
    errno_t err = fopen_s(&f, pfileName, "wb");

    if (!f) {
        printf("Error opening file '%s' for writing.\n", pfileName);
        assert(false);
        exit(EXIT_FAILURE);
    }

    MeshFileHeader header = {
      .m_meshCount = (u32)m.m_meshes.size(),
      .m_numIndices = m.m_totalIndices,
      .m_indexDataSizeBytes = (u32)(m.m_indexData.Size),
      .m_numVertices = m.m_totalVertices,
      .m_vertexDataSizeBytes = (u32)(m.m_vertexData.Size),
      .m_vertexSizeBytes = m.m_vertexSize
    };

    fwrite(&header, 1, sizeof(header), f);
   // fwrite(&m.streams, 1, sizeof(m.streams), f);
    fwrite(m.m_meshes.data(), sizeof(Mesh), header.m_meshCount, f);
    fwrite(m.m_boxes.data(), sizeof(BoundingBox), header.m_meshCount, f);
    fwrite(m.m_indexData.pMem, 1, header.m_indexDataSizeBytes, f);
    fwrite(m.m_vertexData.pMem, 1, header.m_vertexDataSizeBytes, f);

    fclose(f);

    printf("Mesh saved to '%s'\n", pfileName);
}
