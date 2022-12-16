#include <stdio.h>
#include <vector>

#include "ogldev_math_3d.h"
#include "triangle_list.h"
#include "terrain.h"


TriangleList::TriangleList()
{
}


void TriangleList::CreateTriangleList(int Width, int Depth, const BaseTerrain* pTerrain)
{
	m_width = Width;
	m_depth = Depth;

	CreateGLState();

	PopulateBuffers(pTerrain);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


void TriangleList::CreateGLState()
{
	glGenVertexArrays(1, &m_vao);

	glBindVertexArray(m_vao);

	glGenBuffers(1, &m_vb);

	glBindBuffer(GL_ARRAY_BUFFER, m_vb);

	glGenBuffers(1, &m_ib);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ib);

	int POS_LOC = 0;

	size_t NumFloats = 0;
	
	glEnableVertexAttribArray(POS_LOC);
	glVertexAttribPointer(POS_LOC, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(NumFloats * sizeof(float)));
	NumFloats += 3;
}


void TriangleList::PopulateBuffers(const BaseTerrain* pTerrain)
{
	std::vector<Vertex> Vertices;
	Vertices.resize(m_width * m_depth);

	InitVertices(pTerrain, Vertices);

	std::vector<unsigned int> Indices;
	int NumQuads = (m_width - 1) * (m_depth - 1);
	Indices.resize(NumQuads * 6);
	InitIndices(Indices);

	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices[0]) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices[0]) * Indices.size(), &Indices[0], GL_STATIC_DRAW);
}


void TriangleList::Vertex::InitVertex(const BaseTerrain* pTerrain, int x, int z)
{
	float y = pTerrain->GetHeight(x, z);

	float WorldScale = pTerrain->GetWorldScale();

	Pos = Vector3f(x * WorldScale, y, z * WorldScale);
}


void TriangleList::InitVertices(const BaseTerrain* pTerrain, std::vector<Vertex>& Vertices)
{
	int Index = 0;

	for (int z = 0; z < m_depth; z++) {
		for (int x = 0; x < m_width; x++) {
			assert(Index < Vertices.size());
			Vertices[Index].InitVertex(pTerrain, x, z);
			Index++;
		}
	}

    assert(Index == Vertices.size());
}


void TriangleList::InitIndices(std::vector<unsigned int>& Indices)
{
	int Index = 0;

	for (int z = 0; z < m_depth - 1; z++) {
		for (int x = 0; x < m_width - 1; x++) {
			unsigned int IndexBottomLeft = z * m_width + x;
			unsigned int IndexTopLeft = (z + 1) * m_width + x;
			unsigned int IndexTopRight = (z + 1) * m_width + x + 1;
			unsigned int IndexBottomRight = z * m_width + x + 1;

			// Add top left triangle
            assert(Index < Indices.size());
			Indices[Index++] = IndexBottomLeft;
            assert(Index < Indices.size());
			Indices[Index++] = IndexTopLeft;
            assert(Index < Indices.size());
			Indices[Index++] = IndexTopRight;

			// Add bottom right triangle
            assert(Index < Indices.size());
			Indices[Index++] = IndexBottomLeft;
            assert(Index < Indices.size());
			Indices[Index++] = IndexTopRight;
            assert(Index < Indices.size());
			Indices[Index++] = IndexBottomRight;
		}
	}

    assert(Index == Indices.size());
}


void TriangleList::Render()
{
	glBindVertexArray(m_vao);

	glDrawElements(GL_TRIANGLES, (m_depth - 1) * (m_width - 1) * 6, GL_UNSIGNED_INT, NULL);

	glBindVertexArray(0);
}












































