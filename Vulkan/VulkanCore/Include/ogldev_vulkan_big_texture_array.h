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
*/


#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "Int/model_desc.h"

namespace OgldevVK {

class BigTextureArray {

public:

	BigTextureArray() {}

	void Init(VkDevice Device, VkDescriptorPool DescPool, u32 MaxTextures, u32 BindingPoint);

	void CreateTextureArray(const std::vector<OgldevVK::ModelDesc>& ModelDescs);

	void Destroy();

	VkDescriptorSetLayout GetDescSetLayout() const { return m_descSetLayout; }

	VkDescriptorSet GetDescSet() const { return m_descSet; }

private:

	void CreateDescSetLayout(u32 MaxTextures);

	void AllocDescSet(VkDescriptorPool DescPool, u32 MaxTextures);

	VkDevice m_device = VK_NULL_HANDLE;
	VkDescriptorSetLayout m_descSetLayout = VK_NULL_HANDLE;
	VkDescriptorSet m_descSet;
	u32 m_bindingPoint = 0;
};

}