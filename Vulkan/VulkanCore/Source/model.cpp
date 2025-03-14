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


void VkModel::PopulateBuffers(vector<Vertex>& Vertices)
{
	m_vb = m_pVulkanCore->CreateVertexBuffer(Vertices.data(), ARRAY_SIZE_IN_BYTES(Vertices));
    //	printf("%d\n", sizeof(Vertices[0]));
	m_ib = m_pVulkanCore->CreateVertexBuffer(m_Indices.data(), ARRAY_SIZE_IN_BYTES(m_Indices));
}

}