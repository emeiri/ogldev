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

#include "ogldev_vulkan_renderer.h"

namespace OgldevVK {

class ModelRenderer : public VulkanRenderer {
public:
	ModelRenderer(VulkanCore& vkCore, const char* pModelFilename, const char* pTextureFilename, size_t UniformDataSize);

	virtual void FillCommandBuffer(VkCommandBuffer CmdBuf, int Image) override;

	void UpdateUniformBuffers(int Image, const void* pData, size_t Size);

private:

	void CreateShaders();

	size_t m_vertexBufferSize = 0;
	size_t m_indexBufferSize = 0;
	BufferAndMemory m_vb;
	BufferAndMemory m_ib;
	int m_numVertices = 0;
	int m_numIndices = 0;
	VulkanTexture m_texture;
	VkShaderModule m_vs = NULL;
	VkShaderModule m_fs = NULL;
};


}