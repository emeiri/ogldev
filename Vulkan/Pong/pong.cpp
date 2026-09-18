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

#include <array>
#include <stdio.h>
#include <stdlib.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "imGuIZMOquat.h"

#include "ogldev_vulkan_util.h"
#include "ogldev_vulkan_core.h"
#include "ogldev_vulkan_wrapper.h"
#include "ogldev_vulkan_shader.h"
#include "ogldev_vulkan_glfw.h"
#include "ogldev_vulkan_model.h"
#include "ogldev_glm_camera.h"
#include "ogldev_vulkan_imgui.h"
#include "ogldev_vulkan_big_texture_array.h"
#include "Int/model_desc.h"
#include "lighting_program.h"
#include "prebaked_renderer.h"

#define WINDOW_WIDTH 2560
#define WINDOW_HEIGHT 1440

#define MAX_TEXTURES 4096

#define BIG_TEXTURE_ARRAY_BINDING 0

#define NAME_OF_THE_GAME "Pong"

class Pong {

public:
    
    Pong(int WindowWidth, int WindowHeight) : m_renderer(WindowWidth, WindowHeight, NAME_OF_THE_GAME)
    {
    }

    void Execute()
    {
		std::string AssetPath = "../../Games/Pong/Pong.glb";
		m_renderer.Init(AssetPath);
        m_renderer.Execute();
    }

private:
    PreBakedRenderer m_renderer;
};


int main(int argc, char* argv[])
{
    Pong Game(WINDOW_WIDTH, WINDOW_HEIGHT);

	Game.Execute();

	return 0;
}