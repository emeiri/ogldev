/*

        Copyright 2024 Etay Meiri

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

    Terrain Rendering - demo 14 - Perlin Noise
*/

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <random>

#include "demolition.h"
#include "demolition_base_gl_app.h"
#include "Services/perlin.h"

#define WINDOW_WIDTH  2560
#define WINDOW_HEIGHT 1440

/*float GetTerrainHeightAt(float WorldX, float WorldZ, const std::vector<float>& HeightMapData, const PerlinConfig& Config)
{
    // 1. Convert world coordinates back into un-scaled grid coordinates
    float UnscaledX = WorldX / Config.horizontalScale;
    float UnscaledZ = WorldZ / Config.horizontalScale;

    // 2. Find the integer base corner of the current quad cell
    int CellX = (int)(std::floor(UnscaledX));

    // THE MIRROR FIX: Invert the Z tracker to align the CPU layout with OpenGL's native internal memory packing
    int CellZ = (Config.height - 1) - (int)(std::floor(UnscaledZ));

    // 3. Strict Map Boundary Safety Protection
    if (CellX < 0 || CellX >= Config.width - 1 || CellZ < 1 || CellZ >= Config.height) {
        return 0.0f;
    }

    // 4. Fetch the 4 exact heights directly from your direct-memory CPU layout matching the Z flip
    // Because Z moves down rows now, we adjust indices to maintain correct relative directions
    float H_00 = HeightMapData[CellZ * Config.width + CellX];         // Close-Left
    float H_10 = HeightMapData[CellZ * Config.width + (CellX + 1)];   // Close-Right
    float H_01 = HeightMapData[(CellZ - 1) * Config.width + CellX];   // Far-Left
    float H_11 = HeightMapData[(CellZ - 1) * Config.width + (CellX + 1)]; // Far-Right

    // 5. Get local fractional distance coordinates inside this 1x1 meter grid cell
    float FracX = UnscaledX - (float)(CellX);
    float FracZ = UnscaledZ - (float)((int)(std::floor(WorldZ / Config.horizontalScale))); // Keep fractional Z moving forward

    float FinalNormalizedHeight = 0.0f;

    // 6. Identify the triangle sector matching your geometric mesh structure
    if (FracX + FracZ <= 1.0f) {
        FinalNormalizedHeight = H_00 + FracX * (H_10 - H_00) + FracZ * (H_01 - H_00);
    } else {
        FinalNormalizedHeight = H_11 + (1.0f - FracX) * (H_01 - H_11) + (1.0f - FracZ) * (H_10 - H_11);
    }

    // 7. Output scaled world height in meters
    return FinalNormalizedHeight;
}
*/


class Terrain14 : public BaseGLApp {

public:

    Terrain14() : BaseGLApp(WINDOW_WIDTH, WINDOW_HEIGHT, "Terrain Rendering 14 - Perlin Noise")
    {
    }

    ~Terrain14() {}

    void Start()
    {
        m_pScene = m_pRenderingSystem->CreateEmptyScene();

        CreatePerlinMap(m_terrainConfig, m_heightMap);

        void* pTerrain = m_pRenderingSystem->CreateTerrainGrid(m_terrainConfig.width, m_terrainConfig.height);

        TextureConfig TexConfig;
        TexConfig.m_wrapMode = WRAP_MODE_CLAMP_TO_EDGE;
        TexConfig.m_numChannels = 1;
        TexConfig.m_isFloat = true;
        TexConfig.m_genMipmaps = true;
        m_terrainTexHeightMap = m_pRenderingSystem->LoadTexture2D(m_heightMap.data(), m_terrainConfig.width, m_terrainConfig.height, &TexConfig);

        int SandTexture = m_pRenderingSystem->LoadTexture2D("../Content/textures/Polyhaven/forrest_sand_01_diff_2k.jpg");
        int GrassTexture = m_pRenderingSystem->LoadTexture2D("../Content/textures/Polyhaven/rocky_terrain_02_diff_2k.jpg");
        int RockTexture = m_pRenderingSystem->LoadTexture2D("../Content/textures/Polyhaven/aerial_rocks_01_diff_2k.jpg");
        int SnowTexture = m_pRenderingSystem->LoadTexture2D("../Content/textures/Polyhaven/snow_03_diff_2k.jpg");

        m_pScene = m_pRenderingSystem->CreateEmptyScene();

        m_pScene->SetClearColor(Vector4f(0.5f, 0.5f, 0.5f, 1.0f));

        SceneConfig* pConfig = m_pScene->GetConfig();
        pConfig->ControlShadowMapping(false);
        pConfig->SetTerrainGrid(pTerrain);
        pConfig->SetTerrainHeightMap(m_terrainTexHeightMap);
     //   pConfig->SetTerrainHorizontalScale(m_terrainConfig.horizontalScale);
     //   pConfig->SetTerrainMaxHeight(m_terrainConfig.maxHeight);
        pConfig->SetTerrainTexture(0, SandTexture);
        pConfig->SetTerrainTexture(1, GrassTexture);
        pConfig->SetTerrainTexture(2, RockTexture);
        pConfig->SetTerrainTexture(3, SnowTexture);

        m_pRenderingSystem->SetScene(m_pScene);

        // SceneObject* pSceneObject = m_pScene->CreateSceneObject(pModel);
        // pSceneObject->SetScale(0.01f);
       //  m_pScene->AddToRenderList(pSceneObject);

        float CameraX = 0.0f;
        float CameraZ = 0.0f;//-m_terrainConfig.height * m_terrainConfig.horizontalScale;
        float CameraY = 0.0f;

     /*   if (m_cameraOnGround) {
            float GroundHeight = GetTerrainHeightAt(CameraX, CameraZ, m_heightMap, m_terrainConfig) * m_terrainConfig.maxHeight;
            CameraY = 1.7f; // Add human height factor to the ground elevation for camera eye level
            CameraZ = m_terrainConfig.height * m_terrainConfig.horizontalScale / 2.0f;
        } else {
            CameraY = 650.0f;
        }*/

        CameraX = 0.f;
        CameraY = 100.0;
        CameraZ = 0.0f;
        Vector3f Target(0.0f, -0.99f, 0.0f);

        m_pScene->SetCamera(Vector3f(CameraX, CameraY, CameraZ), Target);
        m_pScene->SetCameraZRange(0.5f, 8000.0f);
        
        m_pRenderingSystem->Execute();
    }


    void OnFrame(double DeltaTime)
    {
        if (m_cameraOnGround) {
            glm::vec3 PlayerPosition = m_pScene->GetCurrentCamera()->GetPosition();

            // Get the exact ground height in meters directly underneath the player's boots

            float TerrainMaxHeight = m_pScene->GetConfig()->GetTerrainMaxHeight();
         //   m_terrainConfig.maxHeight = TerrainMaxHeight;
            float TerrainHorizontalScale = m_pScene->GetConfig()->GetTerrainHorizontalScale();
         //   m_terrainConfig.horizontalScale = TerrainHorizontalScale;

        //    float GroundHeight = GetTerrainHeightAt(PlayerPosition.x, PlayerPosition.z, m_heightMap, m_terrainConfig) * TerrainMaxHeight;

            // Set camera eye level: Ground elevation + 1.7 meters human height factor
        //    PlayerPosition.y = GroundHeight + 1.7f;

            m_pScene->GetCurrentCamera()->SetPos(PlayerPosition);
        }
    }

    void OnFrameEnd()
    {
        if (m_showGui) {
            ShowGUI();
        }
    }

private:

    void ShowGUI()
    {
        m_pScene->StartSceneGUI("Perlin Noise");

        if (ImGui::TreeNode("Perlin")) {
            bool IsDirty = false;

            //  IsDirty |= ImGui::SliderInt("Width", &m_terrainConfig.width, 1, 1024);
            //  IsDirty |= ImGui::SliderInt("Height", &m_terrainConfig.height, 1, 1024);
          //  IsDirty |= ImGui::SliderInt("Octaves", &m_terrainConfig.octaves, 1, 8);
         //   IsDirty |= ImGui::SliderFloat("Lacunarity", &m_terrainConfig.lacunarity, 1.0f, 4.0f);
         //   IsDirty |= ImGui::SliderFloat("Persistence", &m_terrainConfig.persistence, 0.1f, 1.0f);
            IsDirty |= ImGui::SliderFloat("Scale", &m_terrainConfig.scale, 1.0, 200.0f);
         //   IsDirty |= ImGui::SliderInt("Seed", &m_terrainConfig.seed, 0, 1000);

            //   ImGui::Begin("Heightmap");
            int TextureID = m_pRenderingSystem->GetTextureAPIHandle(m_terrainTexHeightMap);
            ImTextureID TexturePtr = (ImTextureID)TextureID;
            ImGui::Image(TexturePtr, ImVec2(512.0f, 512.0f));
            //  ImGui::End();

            if (IsDirty) {
                CreatePerlinMap(m_terrainConfig, m_heightMap);
                m_pRenderingSystem->UpdateTexture2D(m_terrainTexHeightMap, m_heightMap.data());
                //m_isTerrainConfigDirty = true;
            }

            ImGui::TreePop();
        }
        
        m_pScene->EndSceneGUI();
    }


    virtual bool OnKeyboard(int key, int action)
    {
        bool HandledByMe = true;

        if (action == GLFW_PRESS) {
            switch (key) {

            case GLFW_KEY_SPACE:
                if (action == GLFW_PRESS) {
                    m_showGui = !m_showGui;
                }
                break;

            case GLFW_KEY_G:
                m_cameraOnGround = !m_cameraOnGround;
                break;

            default:
                HandledByMe = BaseGLApp::OnKeyboard(key, action);
            }
        } else {
            HandledByMe = BaseGLApp::OnKeyboard(key, action);
        }

        return HandledByMe;
    }


    Scene* m_pScene = NULL;
    std::vector<float> m_heightMap;
    PerlinConfig m_terrainConfig;
    int m_terrainTexHeightMap = -1;
    bool m_cameraOnGround = false;
    bool m_showGui = false;
};


int main(int argc, char* arg[])
{
    Terrain14 demo;
    demo.Start();
}
