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

#include <stdio.h>

#include "ogldev_types.h"
#include "ogldev_util.h"
#include "ogldev_vulkan_util.h"
#include "lighting_program.h"


namespace OgldevVK {

enum Binding {
	BindingVB = 0,
	BindingIB = 1,
	BindingUniformVS = 2,
	BindingMetaData = 3,
	BindingUniformFS = 4,
	NumBindings = 5
};


void LightingProgram::Init(VulkanCore& vkCore,
	VkDescriptorPool DescPool,
	VkDescriptorSetLayout TextureDescSetLayout,
	const std::vector<VkDescriptorSet>* pTextureDescSets,
	VkShaderModule vs,
	VkShaderModule fs,
	LIGHTING_MODE LightingMode)
{
	m_textureDescSetLayout = TextureDescSetLayout;
	m_pTextureDescSets = pTextureDescSets;
    m_numImages = vkCore.GetNumImages();

	VkSpecializationMapEntry SpecMapEntry = {
		.constantID = 0,
		.offset = 0,
		.size = sizeof(u32)
	};

	VkSpecializationInfo SpecInfo = {
		.mapEntryCount = 1,
		.pMapEntries = &SpecMapEntry,
		.dataSize = sizeof(u32),
		.pData = &LightingMode
	};

	GraphicsPipeline::Init(vkCore, DescPool, vs, fs, NULL, &SpecInfo);
}


void LightingProgram::AllocDescSets(std::vector<VkDescriptorSet>& DescSets)
{
	// The first descriptor set layout belongs to the global texture array.
    // The second descriptor set layout belongs to the per-model data (vertex/index buffers, uniforms, etc). Here
    // we allocate descriptor sets for the second layout, hence the index 1 in the call below.
	GraphicsPipeline::AllocDescSets(m_numImages, DescSets, 1);
}


void LightingProgram::Destroy()
{
	GraphicsPipeline::Destroy(false);

    // The first layout was created externally and belongs to the global texture array, so we don't destroy it here. 
	// The second layout was created by this class, so we destroy it.
	vkDestroyDescriptorSetLayout(m_device, m_descSetLayouts[1], NULL);
}


void LightingProgram::Bind(int ImageIndex, VkCommandBuffer CmdBuf, VkDescriptorSet& DescSet)
{
	std::vector<VkDescriptorSet> DescSets = { (*m_pTextureDescSets)[ImageIndex], DescSet };
	GraphicsPipeline::Bind(CmdBuf, DescSets); 
}


std::vector<VkDescriptorSetLayout> LightingProgram::CreateDescSetLayout(OgldevVK::VulkanCore& vkCore)
{
	std::vector<VkDescriptorSetLayoutBinding> Bindings{
		{.
		     binding = BindingVB,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			.pImmutableSamplers = NULL
		}, {
			.binding = BindingIB,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			.pImmutableSamplers = NULL
		}, {
			.binding = BindingUniformVS,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			.pImmutableSamplers = NULL
		}, {
			.binding = BindingMetaData,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			.pImmutableSamplers = NULL
		}, {
			.binding = BindingUniformFS,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.pImmutableSamplers = NULL
		}
	};

	VkDescriptorSetLayout l = vkCore.CreateDescSetLayout({ Bindings });

	std::vector<VkDescriptorSetLayout> ret = { m_textureDescSetLayout, l };

	return ret;
}


std::vector<VkPushConstantRange> LightingProgram::GetPushConstantRange()
{
	//VkPushConstantRange PushConstantRange =

	std::vector<VkPushConstantRange> PushConstantRanges = {
		{
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.offset = 0,
			.size = sizeof(u32)
		}
	};

	return PushConstantRanges;
}



void LightingProgram::UpdateDescriptorSets(const ModelDesc& ModelDesc,
										   std::vector<VkDescriptorSet>& DescSets,
										   std::vector<BufferAndMemory>& UniformBuffersVS,
										   std::vector<BufferAndMemory>& UniformBuffersFS)
{
	std::vector<VkDescriptorBufferInfo> BufferInfoUniformsVS(DescSets.size());

	for (int i = 0; i < (int)BufferInfoUniformsVS.size(); i++) {
		BufferInfoUniformsVS[i].buffer = UniformBuffersVS[i].m_buffer;
		BufferInfoUniformsVS[i].offset = 0;
		BufferInfoUniformsVS[i].range = VK_WHOLE_SIZE;
	}

	std::vector<VkDescriptorBufferInfo> BufferInfoUniformsFS(DescSets.size());

	for (int i = 0; i < (int)BufferInfoUniformsFS.size(); i++) {
		BufferInfoUniformsFS[i].buffer = UniformBuffersFS[i].m_buffer;
		BufferInfoUniformsFS[i].offset = 0;
		BufferInfoUniformsFS[i].range = VK_WHOLE_SIZE;
	}

	VkDescriptorBufferInfo BufferInfo_VB = {
		.buffer = ModelDesc.m_vb,
		.offset = 0,
		.range = VK_WHOLE_SIZE
	};

	VkDescriptorBufferInfo BufferInfo_IB = {
		.buffer = ModelDesc.m_ib,
		.offset = 0,
		.range = VK_WHOLE_SIZE
	};
	
	VkDescriptorBufferInfo MetaDataBufferInfo = {
		.buffer = ModelDesc.m_metaData,
		.offset = 0,
		.range = VK_WHOLE_SIZE
	};

	std::vector<VkWriteDescriptorSet> WriteDescriptorSet(DescSets.size() * NumBindings);

	u32 WdsIndex = 0;

	for (int i = 0; i < (int)DescSets.size(); i++) {
		VkDescriptorSet& DstSet = DescSets[i];

		VkWriteDescriptorSet wds = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = DstSet,
			.dstBinding = BindingVB,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.pBufferInfo = &BufferInfo_VB
		};

		assert(WdsIndex < WriteDescriptorSet.size());
		WriteDescriptorSet[WdsIndex++] = wds;

		wds = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = DstSet,
			.dstBinding = BindingIB,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.pBufferInfo = &BufferInfo_IB
		};

		assert(WdsIndex < WriteDescriptorSet.size());
		WriteDescriptorSet[WdsIndex++] = wds;

		wds = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = DstSet,
			.dstBinding = BindingUniformVS,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.pBufferInfo = &BufferInfoUniformsVS[i]
		};

		assert(WdsIndex < WriteDescriptorSet.size());
		WriteDescriptorSet[WdsIndex++] = wds;

		wds = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = NULL,
			.dstSet = DstSet,
			.dstBinding = BindingMetaData,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.pImageInfo = NULL,
			.pBufferInfo = &MetaDataBufferInfo,
			.pTexelBufferView = NULL
		};

		assert(WdsIndex < WriteDescriptorSet.size());
		WriteDescriptorSet[WdsIndex++] = wds;

		wds = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = DstSet,
			.dstBinding = BindingUniformFS,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.pBufferInfo = &BufferInfoUniformsFS[i]
		};

		assert(WdsIndex < WriteDescriptorSet.size());
		WriteDescriptorSet[WdsIndex++] = wds;
	}

	vkUpdateDescriptorSets(m_device, WdsIndex, WriteDescriptorSet.data(), 0, NULL);
}


void LightingProgram::UpdateUniformBuffers(int ImageIndex, 
                                            const glm::mat4& WVP, 
                                            const glm::mat4& World,				
                                            const std::vector<glm::mat4>& SubmeshTransformations,
                                            const glm::vec4& AmbientLight,
                                            const glm::vec3& LightDirection,
                                            std::vector<BufferAndMemory>& UniformBuffersVS,
                                            std::vector<BufferAndMemory>& UniformBuffersFS)
{
	std::vector<UniformDataVS> UboDataVS(SubmeshTransformations.size());

	for (int i = 0; i < SubmeshTransformations.size(); i++) {
		UboDataVS[i].WVP = WVP * SubmeshTransformations[i];

		glm::mat4 WorldAndBase = World * SubmeshTransformations[i];
		glm::mat3 NormalMatrix = glm::transpose(glm::inverse(glm::mat3(WorldAndBase)));

		UboDataVS[i].NormalMatrix = NormalMatrix;
	}

	UniformBuffersVS[ImageIndex].Update(m_device, UboDataVS.data(), sizeof(UniformDataVS) * UboDataVS.size());

	UniformDataFS UboDataFS = {
		.AmbientLight = AmbientLight,
		.LightDirection = glm::vec4(LightDirection, 0.0f),
		.LightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)
	};

	UniformBuffersFS[ImageIndex].Update(m_device, &UboDataFS, sizeof(UboDataFS));	
}


}