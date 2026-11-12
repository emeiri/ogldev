/*

		Copyright 2026 Etay Meiri

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

    Vulkan Lighting Test
*/

#include "../Common/prebaked_renderer.h"

#define WINDOW_WIDTH 2560
#define WINDOW_HEIGHT 1440

#define APP_NAME "Vulkan Lighting Test"

static const float FIXED_DELTA_TIME = 1.0f / 120.0f;

class VulkanPong : public GameCallbacks {

public:
    
    VulkanPong(int WindowWidth, int WindowHeight) 
		: m_renderer(WindowWidth, WindowHeight, APP_NAME, this)
    {        
    }

    void Init()
    {
       // std::string AssetPath = "lighting_test_2_spots_sun.glb";
        //std::string AssetPath = "G:/Models/glTF-Sample-Assets/Models/Sponza/glTF/Sponza.gltf";
    //   std::string AssetPath = "../../../Content/glTF-Sample-Assets/Sponza/Sponza.glb";
      //  std::string AssetPath = "G:/Models/McGuire/lost-empire/lost_empire.glb";
      //  std::string AssetPath = "G:/Models/McGuire/sibenik/sibenik.glb";
        std::string AssetPath = "../../../Content/DamagedHelmet/DamagedHelmet.gltf";
        //std::string AssetPath = "G:/Models/glTF-Sample-Assets/Models/ABeautifulGame/glTF/ABeautifulGame.gltf";
        //std::string AssetPath = "G:/Models/glTF-Sample-Assets/Models/AnisotropyBarnLamp/glTF/AnisotropyBarnLamp.gltf";
        //std::string AssetPath = "../../../Content/Sketchfab/adjustable_spannerwrench/scene.gltf";
       // std::string AssetPath = "../../../Content/Sketchfab/back_street_alley_photogrammetry_scan/scene.gltf";
       // std::string AssetPath = "../../../Content/Sketchfab/futuristic_spaceship/scene.gltf";
       // std::string AssetPath = "../../../Content/Sketchfab/old_street_store/scene.gltf";
      //  std::string AssetPath = "G:/Models/coast_land_rocks_03_4k.gltf/coast_land_rocks_03_4k.gltf";
       // std::string AssetPath = "G:/Models/concrete_cat_statue_4k.gltf/concrete_cat_statue_4k.gltf";
       // std::string AssetPath = "G:/Models/GLTF-Assets/Bistro/BistroExterior.gltf"; // KTX2 problem
        //std::string AssetPath = "G:/Models/ship_pinnace_2k.gltf/ship_pinnace_2k.gltf";
       // std::string AssetPath = "G:/Models/ship_pinnace_4k.gltf/ship_pinnace_4k.gltf";
        //std::string AssetPath = "G:/Models/Sketchfab/art_gallery/scene.gltf"; // doesn't look good
        //std::string AssetPath = "G:/Models/Sketchfab/dragon_sculpture/scene.gltf";
     //   std::string AssetPath = "G:/Models/Sketchfab/plane/scene.gltf";
        //std::string AssetPath = "G:/Models/Sketchfab/wizards_triplet_towers.glb";
      //  std::string AssetPath = "G:/Models/Sketchfab/blood_pool__location.glb";// lots of z fights

        m_renderer.Init(AssetPath);
    }


    void Execute()
    {
        m_renderer.Execute();
    }

private:
    PreBakedRenderer m_renderer;
};


int main(int argc, char* argv[])
{
    VulkanPong Game(WINDOW_WIDTH, WINDOW_HEIGHT);

	Game.Init();
	Game.Execute();

	return 0;
}