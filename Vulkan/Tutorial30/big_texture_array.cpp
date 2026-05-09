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

#include "ogldev_types.h"
#include "ogldev_vulkan_util.h"
#include "big_texture_array.h"


void BigTextureArray::Init(VkDevice Device, VkDescriptorPool DescPool, int NumImages, u32 MaxTextures, u32 BindingPoint)
{
    m_device = Device;
    m_bindingPoint = BindingPoint;
    CreateDescSetLayout(MaxTextures);
    AllocDescSets(DescPool, MaxTextures, NumImages);
}


void BigTextureArray::CreateDescSetLayout(u32 MaxTextures)
{
	VkDescriptorBindingFlags BindingFlags =
		VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
		VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
		VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;

	VkDescriptorSetLayoutBindingFlagsCreateInfo BindingFlagsInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
		.bindingCount = 1,         // Matches number of bindings in layout
		.pBindingFlags = &BindingFlags
	};

	VkDescriptorSetLayoutBinding LayoutBindings = {
		.binding = m_bindingPoint,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = MaxTextures,
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		.pImmutableSamplers = NULL
	};

	VkDescriptorSetLayoutCreateInfo LayoutCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = &BindingFlagsInfo,
		.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
		.bindingCount = 1,
		.pBindings = &LayoutBindings
	};

	VkResult res = vkCreateDescriptorSetLayout(m_device, &LayoutCreateInfo, NULL, &m_descSetLayout);
	CHECK_VK_RESULT(res, "vkCreateDescriptorSetLayout");
}


void BigTextureArray::AllocDescSets(VkDescriptorPool DescPool, u32 MaxTextures, int NumImages)
{
	size_t NumDescSets = NumImages;

	std::vector<VkDescriptorSetLayout> Layouts(NumDescSets, m_descSetLayout);

	std::vector<u32> TextureCounts(NumDescSets, MaxTextures);

	VkDescriptorSetVariableDescriptorCountAllocateInfo VariableCountInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
		.pNext = NULL,
		.descriptorSetCount = (u32)NumDescSets,
		.pDescriptorCounts = TextureCounts.data()
	};

	VkDescriptorSetAllocateInfo AllocInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.pNext = &VariableCountInfo,
		.descriptorPool = DescPool,
		.descriptorSetCount = (u32)Layouts.size(),
		.pSetLayouts = Layouts.data()
	};

	m_descSets.resize(NumDescSets);

	VkResult res = vkAllocateDescriptorSets(m_device, &AllocInfo, m_descSets.data());
	CHECK_VK_RESULT(res, "vkAllocateDescriptorSets");
}


void BigTextureArray::CreateTextureArray(const std::vector<OgldevVK::ModelDesc>& ModelDescs)
{
	u32 TotalTextureCount = 0;

	for (int i = 0; i < (int)ModelDescs.size(); i++) {
		const OgldevVK::ModelDesc& md = ModelDescs[i];
		TotalTextureCount += (u32)md.m_materials.size();
	}

	std::vector<VkDescriptorImageInfo> ImageInfos;
	ImageInfos.resize(TotalTextureCount);

	int Index = 0;
	for (const OgldevVK::ModelDesc& md : ModelDescs) {
		u32 TextureCount = (u32)md.m_materials.size();

		for (u32 i = 0; i < TextureCount; ++i) {
			ImageInfos[Index].sampler = md.m_materials[i].m_sampler;
			ImageInfos[Index].imageView = md.m_materials[i].m_imageView;
			ImageInfos[Index].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			Index++;
		}
	}

	std::vector<VkWriteDescriptorSet> WriteDescriptorSet(m_descSets.size());

	for (int i = 0; i < (int)m_descSets.size(); i++) {
		VkDescriptorSet& DstSet = m_descSets[i];

		WriteDescriptorSet[i] = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = NULL,
			.dstSet = DstSet,
			.dstBinding = m_bindingPoint,
			.dstArrayElement = 0,
			.descriptorCount = TotalTextureCount,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.pImageInfo = ImageInfos.data(),
			.pBufferInfo = NULL,
			.pTexelBufferView = NULL
		};
	}

	vkUpdateDescriptorSets(m_device, (u32)WriteDescriptorSet.size(), WriteDescriptorSet.data(), 0, NULL);
}



void BigTextureArray::Destroy()
{
	vkDestroyDescriptorSetLayout(m_device, m_descSetLayout, NULL);
}