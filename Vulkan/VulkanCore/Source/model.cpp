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

#include <vulkan/vulkan.h>

#include "ogldev_vulkan_core.h"
#include "ogldev_vulkan_model.h"

namespace OgldevVK {


void VkModel::Destroy()
{
	m_vb.Destroy(m_pVulkanCore->GetDevice());
	m_ib.Destroy(m_pVulkanCore->GetDevice());
}


Texture* VkModel::AllocTexture2D()
{
	assert(m_pVulkanCore);

	return new VulkanTexture(m_pVulkanCore);
}

struct NewVertex {
	glm::vec3 Pos;
	glm::vec2 Tex;
};


void VkModel::PopulateBuffers(vector<Vertex>& Vertices)
{
	std::vector<NewVertex> foo;
	foo.resize(Vertices.size());
	for (int i = 0; i < Vertices.size(); i++) {
		foo[i].Pos.x = Vertices[i].Position.x;
		foo[i].Pos.y = Vertices[i].Position.y;
		foo[i].Pos.z = Vertices[i].Position.z;
		foo[i].Tex.x = Vertices[i].TexCoords.x;
		foo[i].Tex.y = Vertices[i].TexCoords.y;
	}
	m_vb = m_pVulkanCore->CreateVertexBuffer(foo.data(), ARRAY_SIZE_IN_BYTES(foo));
    //	printf("%d\n", sizeof(Vertices[0]));
	m_ib = m_pVulkanCore->CreateVertexBuffer(m_Indices.data(), ARRAY_SIZE_IN_BYTES(m_Indices));
}

}