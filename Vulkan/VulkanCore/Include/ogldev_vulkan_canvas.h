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
*/

#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <vector>

#include "ogldev_vulkan_renderer.h"

namespace OgldevVK {

class CanvasRenderer : public VulkanRenderer {
public:
	CanvasRenderer(VulkanCore& vkCore);

	~CanvasRenderer();

	virtual void FillCommandBuffer(VkCommandBuffer CmdBuf, int Image) override;

	void Clear();
	void Line(const glm::vec3& p1, const glm::vec3& p2, const glm::vec4& Color);
	void Plane3D(const glm::vec3& Orig, const glm::vec3& v1, const glm::vec3& v2, int n1, int n2, 
			     float s1, float s2, const glm::vec4& Color, const glm::vec4& OutlineColor);
	void UpdateBuffer(int Image);
	void UpdateUniformBuffer(int Image, const glm::mat4& WVP, float Time);

private:

	struct VertexData
	{
		glm::vec3 Position;
		glm::vec4 Color;
	};

	struct UniformBuffer
	{
		glm::mat4 WVP;
		float Time;
	};

	void CreateShaders();
	void CreateVertexBuffers();

	std::vector<BufferAndMemory> m_VBs;
	int m_numVertices = 0;
	std::vector<VertexData> m_lines;

	VkShaderModule m_vs = NULL;
	VkShaderModule m_fs = NULL;

	static constexpr u32 MaxLinesCount = 65536;
	static constexpr u32 MaxLinesDataSize = MaxLinesCount * sizeof(CanvasRenderer::VertexData) * 2;
};


}