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

class BigTextureArray {

public:

    BigTextureArray() {}

    void Init(VkDevice Device, VkDescriptorPool DescPool, int NumImages, u32 MaxTextures, u32 BindingPoint);

	void CreateTextureArray(const std::vector<OgldevVK::ModelDesc>& ModelDescs);
    
	void Destroy();

    VkDescriptorSetLayout GetDescSetLayout() const { return m_descSetLayout; }

    const std::vector<VkDescriptorSet>& GetDescSets() const { return m_descSets; }

private:

	void CreateDescSetLayout(u32 MaxTextures);

	void AllocDescSets(VkDescriptorPool DescPool, u32 MaxTextures, int NumImages);

	VkDevice m_device = VK_NULL_HANDLE;
	VkDescriptorSetLayout m_descSetLayout = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> m_descSets;	// one set per swapchain image
    u32 m_bindingPoint = 0;
};