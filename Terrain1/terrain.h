#ifndef TERRAIN_H
#define TERRAIN_H

#include "ogldev_types.h"
#include "ogldev_basic_glfw_camera.h"
#include "ogldev_array_2d.h"
#include "triangle_list.h"
#include "terrain_technique.h"


class BaseTerrain
{
public:
	BaseTerrain() {}

	void InitTerrain(float WorldScale);

	void Render(const BasicCamera& Camera);

	void LoadFromFile(const char* pFilename);

	float GetHeight(int x, int z) const { return m_heightMap.Get(x, z); }

	float GetWorldScale() const { return m_worldScale; }

protected:
	void LoadHeightMapFile(const char* pFilename);

	int m_terrainSize = 0;
	float m_worldScale = 1.0f;
	Array2D<float> m_heightMap;
	TriangleList m_triangleList;
	TerrainTechnique m_terrainTech;
};

#endif
