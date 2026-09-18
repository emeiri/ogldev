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

    Pong In Vulkan
*/

#include "prebaked_renderer.h"
#include "pong.h"

#define WINDOW_WIDTH 2560
#define WINDOW_HEIGHT 1440

#define NAME_OF_THE_GAME "Pong"

class VulkanPong {

public:
    
    VulkanPong(int WindowWidth, int WindowHeight) : m_renderer(WindowWidth, WindowHeight, NAME_OF_THE_GAME)
    {
    }

    void Execute()
    {
		std::string AssetPath = "../../Games/Pong/Pong2.glb";
		m_renderer.Init(AssetPath);
        m_renderer.Execute();
    }

private:
    PreBakedRenderer m_renderer;
};


int main(int argc, char* argv[])
{
    VulkanPong Game(WINDOW_WIDTH, WINDOW_HEIGHT);

	Game.Execute();

	return 0;
}