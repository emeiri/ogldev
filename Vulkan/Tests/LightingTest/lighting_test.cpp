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
        std::string AssetPath = "lighting_test.glb";
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