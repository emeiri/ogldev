/*
    Copyright 2022 Etay Meiri

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


#ifndef TERRAIN_H
#define TERRAIN_H

#include "ogldev_types.h"
#include "ogldev_basic_glfw_camera.h"
#include "ogldev_array_2d.h"
#include "ogldev_texture.h"

#include "quad_list.h"
#include "terrain_technique.h"
#include "ogldev_skydome.h"

class BaseTerrain
{
 public:
    BaseTerrain() : m_heightMapTexture(GL_TEXTURE_2D) {}

    ~BaseTerrain();

    void Destroy();

	void InitTerrain(float WorldScale, float TextureScale, const std::vector<string>& TextureFilenames);

    void Render(const BasicCamera& Camera);

    void LoadFromFile(const char* pFilename);

    void SaveToFile(const char* pFilename);

	float GetHeight(int x, int z) const { return m_heightMap.Get(x, z); }
	
    float GetHeightInterpolated(float x, float z) const;

	float GetWorldScale() const { return m_worldScale; }

    float GetTextureScale() const { return m_textureScale; }

    int GetSize() const { return m_terrainSize; }

    void SetTexture(Texture* pTexture) { m_pTextures[0] = pTexture; }
	
    void SetTextureHeights(float Tex0Height, float Tex1Height, float Tex2Height, float Tex3Height);
	
    void SetLightDir(const Vector3f& Dir) { m_lightDir = Dir; }	

    float GetMaxHeight() const { return m_maxHeight; }

    float GetWorldSize() const { return m_numPatches * m_worldScale; }

    Vector3f ConstrainCameraPosToTerrain(const Vector3f& CameraPos);

 protected:

	void LoadHeightMapFile(const char* pFilename);

    void SetMinMaxHeight(float MinHeight, float MaxHeight);

    void Finalize();    

    float GetWorldHeight(float x, float z) const;

    int m_terrainSize = 0;
    int m_numPatches = 0;
	float m_worldScale = 1.0f;
    Array2D<float> m_heightMap;

private:
    float m_textureScale = 1.0f;
    Texture* m_pTextures[4] = { 0 };
    Texture m_heightMapTexture;
    QuadList m_quadList;
    float m_minHeight = 0.0f;
    float m_maxHeight = 0.0f;
    TerrainTechnique m_terrainTech;
    Vector3f m_lightDir;
    float m_cameraHeight = 2.0f;
    Skydome* m_pSkydome = NULL;
};

#endif
