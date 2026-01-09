/*
		Copyright 2025 Etay Meiri

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

#pragma once

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace OgldevVK {

struct GraphicsPipelineDesc {
	VkDevice Device = NULL;
	GLFWwindow* pWindow = NULL;
	VkShaderModule vs = NULL;
	VkShaderModule fs = NULL;
	int NumImages = 0;
	VkFormat ColorFormat = VK_FORMAT_UNDEFINED;
	VkFormat DepthFormat = VK_FORMAT_UNDEFINED;
	VkCompareOp DepthCompareOp = VK_COMPARE_OP_LESS;
	bool IsVB = false;
	bool IsIB = false;
	bool IsUniform = false;
	bool IsTex2D = false;
	bool IsTexCube = false;
};


struct ComputePipelineDesc {
	VkDevice Device = NULL;
	GLFWwindow* pWindow = NULL;
	VkShaderModule cs = NULL;
};

}