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

    Vector3f GetColor(int x, int z) const;

	float GetWorldScale() const { return m_worldScale; }

 protected:
    struct TerrainPoint {
        int x = 0;
        int z = 0;

        void Print()
        {
            printf("[%d,%d]", x, z);
        }

        bool IsEqual(TerrainPoint& p) const
        {
            return ((x == p.x) && (z == p.z));
        }
    };

	void LoadHeightMapFile(const char* pFilename);

    int m_terrainSize = 0;
	float m_worldScale = 1.0f;
    Array2D<float> m_heightMap;
    TriangleList m_triangleList;
    TerrainTechnique m_terrainTech;
    float m_minHeight = 0;
    float m_maxHeight = 0;
    float m_heightScale;
};

#endif
