/*

        Copyright 2023 Etay Meiri

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


#include <stdio.h>
#include <vector>

#include "ogldev_math_3d.h"
#include "geomip_grid.h"
#include "terrain.h"


GeomipGrid::GeomipGrid()
{
}


GeomipGrid::~GeomipGrid()
{
    Destroy();
}


void GeomipGrid::Destroy()
{
    if (m_vao > 0) {
        glDeleteVertexArrays(1, &m_vao);
    }

    if (m_vb > 0) {
        glDeleteBuffers(1, &m_vb);
    }

    if (m_ib > 0) {
        glDeleteBuffers(1, &m_ib);
    }
}


void GeomipGrid::CreateGeomipGrid(int Width, int Depth, int PatchSize, const BaseTerrain* pTerrain)
{
    if ((Width - 1) % (PatchSize - 1) != 0) {
        int RecommendedWidth = ((Width - 1 + PatchSize - 1) / (PatchSize - 1)) * (PatchSize - 1) + 1;
        printf("Width minus 1 (%d) must be divisible by PatchSize minus 1 (%d)\n", Width, PatchSize);
        printf("Try using Width = %d\n", RecommendedWidth);
        exit(0);
    }

    if ((Depth - 1) % (PatchSize - 1) != 0) {
        int RecommendedDepth = ((Depth - 1 + PatchSize - 1) / (PatchSize - 1)) * (PatchSize - 1) + 1;
        printf("Depth minus 1 (%d) must be divisible by PatchSize minus 1 (%d)\n", Depth, PatchSize);
        printf("Try using Width = %d\n", RecommendedDepth);
        exit(0);
    }

    if (PatchSize < 3) {
        printf("The minimum patch size is 3 (%d)\n", PatchSize);
        exit(0);
    }

    if (PatchSize % 2 == 0) {
        printf("Patch size must be an odd number (%d)\n", PatchSize);
        exit(0);
    }

    m_width = Width;
    m_depth = Depth;
    m_patchSize = PatchSize;

    CreateGLState();

	PopulateBuffers(pTerrain);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


void GeomipGrid::CreateGLState()
{
    glGenVertexArrays(1, &m_vao);

    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vb);

    glBindBuffer(GL_ARRAY_BUFFER, m_vb);

    glGenBuffers(1, &m_ib);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ib);

    int POS_LOC = 0;
    int TEX_LOC = 1;
	int NORMAL_LOC = 2;

	size_t NumFloats = 0;
	
    glEnableVertexAttribArray(POS_LOC);
    glVertexAttribPointer(POS_LOC, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(NumFloats * sizeof(float)));
    NumFloats += 3;

    glEnableVertexAttribArray(TEX_LOC);
    glVertexAttribPointer(TEX_LOC, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(NumFloats * sizeof(float)));
    NumFloats += 2;

    glEnableVertexAttribArray(NORMAL_LOC);
    glVertexAttribPointer(NORMAL_LOC, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(NumFloats * sizeof(float)));
    NumFloats += 3;
}


void GeomipGrid::PopulateBuffers(const BaseTerrain* pTerrain)
{
    std::vector<Vertex> Vertices;
    Vertices.resize(m_width * m_depth);

    InitVertices(pTerrain, Vertices);

    std::vector<unsigned int> Indices;
    int NumQuads = (m_patchSize - 1) * (m_patchSize - 1);
    Indices.resize(NumQuads * 6);
    InitIndices(Indices);

    CalcNormals(Vertices, Indices);

    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices[0]) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices[0]) * Indices.size(), &Indices[0], GL_STATIC_DRAW);
}


void GeomipGrid::Vertex::InitVertex(const BaseTerrain* pTerrain, int x, int z)
{
    float y = pTerrain->GetHeight(x, z);

	float WorldScale = pTerrain->GetWorldScale();
	Pos = Vector3f(x * WorldScale, y, z * WorldScale);
	
    float Size = (float)pTerrain->GetSize();
    float TextureScale = pTerrain->GetTextureScale();
    Tex = Vector2f(TextureScale * (float)x / Size, TextureScale * (float)z / Size);	
}


void GeomipGrid::InitVertices(const BaseTerrain* pTerrain, std::vector<Vertex>& Vertices)
{
    int Index = 0;

    for (int z = 0 ; z < m_depth ; z++) {
        for (int x = 0 ; x < m_width ; x++) {
            assert(Index < Vertices.size());
			Vertices[Index].InitVertex(pTerrain, x, z);
			Index++;
        }
    }

    assert(Index == Vertices.size());
}


void GeomipGrid::InitIndices(std::vector<unsigned int>& Indices)
{
    int Index = 0;

    for (int z = 0 ; z < m_patchSize - 1 ; z += 2) {
        for (int x = 0 ; x < m_patchSize - 1 ; x += 2) {
            uint IndexCenter = (z + 1) * m_width + x + 1;

            uint IndexTemp1 = z * m_width + x;
            uint IndexTemp2 = (z + 1) * m_width + x;

            Index = AddTriangle(Index, Indices, IndexCenter, IndexTemp1, IndexTemp2);

            IndexTemp1 = IndexTemp2;
            IndexTemp2 += m_width;

            Index = AddTriangle(Index, Indices, IndexCenter, IndexTemp1, IndexTemp2);

            IndexTemp1 = IndexTemp2;
            IndexTemp2++;

            Index = AddTriangle(Index, Indices, IndexCenter, IndexTemp1, IndexTemp2);

            IndexTemp1 = IndexTemp2;
            IndexTemp2++;

            Index = AddTriangle(Index, Indices, IndexCenter, IndexTemp1, IndexTemp2);

            IndexTemp1 = IndexTemp2;
            IndexTemp2 -= m_width;

            Index = AddTriangle(Index, Indices, IndexCenter, IndexTemp1, IndexTemp2);

            IndexTemp1 = IndexTemp2;
            IndexTemp2 -= m_width;

            Index = AddTriangle(Index, Indices, IndexCenter, IndexTemp1, IndexTemp2);

            IndexTemp1 = IndexTemp2;
            IndexTemp2--;

            Index = AddTriangle(Index, Indices, IndexCenter, IndexTemp1, IndexTemp2);

            IndexTemp1 = IndexTemp2;
            IndexTemp2--;

            Index = AddTriangle(Index, Indices, IndexCenter, IndexTemp1, IndexTemp2);
        }
    }

    assert(Index == Indices.size());
}


uint GeomipGrid::AddTriangle(uint Index, std::vector<uint>& Indices, uint v1, uint v2, uint v3)
{
    //printf("Add triangle %d %d %d\n", v1, v2, v3);
    assert(Index < Indices.size());
    Indices[Index++] = v1;
    assert(Index < Indices.size());
    Indices[Index++] = v2;
    assert(Index < Indices.size());
    Indices[Index++] = v3;

    return Index;
}


void GeomipGrid::CalcNormals(std::vector<Vertex>& Vertices, std::vector<uint>& Indices)
{
    unsigned int Index = 0;

    // Accumulate each triangle normal into each of the triangle vertices
    for (int z = 0 ; z < m_depth - 1 ; z += (m_patchSize - 1)) {
        for (int x = 0 ; x < m_width - 1; x += (m_patchSize - 1)) {
            int BaseVertex = z * m_width + x;
            //printf("Base index %d\n", BaseVertex);
    		for (unsigned int i = 0 ; i < Indices.size() ; i += 3) {
                unsigned int Index0 = BaseVertex + Indices[i];
                unsigned int Index1 = BaseVertex + Indices[i + 1];
                unsigned int Index2 = BaseVertex + Indices[i + 2];
		        Vector3f v1 = Vertices[Index1].Pos - Vertices[Index0].Pos;
		        Vector3f v2 = Vertices[Index2].Pos - Vertices[Index0].Pos;
		        Vector3f Normal = v1.Cross(v2);
		        Normal.Normalize();

		        Vertices[Index0].Normal += Normal;
		        Vertices[Index1].Normal += Normal;
		        Vertices[Index2].Normal += Normal;
    		}
        }
    }

    // Normalize all the vertex normals
    for (unsigned int i = 0 ; i < Vertices.size() ; i++) {
        Vertices[i].Normal.Normalize();
    }
}


void GeomipGrid::Render()
{
    glBindVertexArray(m_vao);

    for (int z = 0 ; z < m_depth - 1 ; z += (m_patchSize - 1)) {
        for (int x = 0 ; x < m_width - 1 ; x += (m_patchSize - 1)) {
            int BaseVertex = z * m_width + x;
            glDrawElementsBaseVertex(GL_TRIANGLES, (m_patchSize - 1) * (m_patchSize - 1) * 6, GL_UNSIGNED_INT, NULL, BaseVertex);
        }
    }

    glBindVertexArray(0);
}
