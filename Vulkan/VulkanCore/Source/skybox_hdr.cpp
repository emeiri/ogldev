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

#include "ogldev_util.h"
#include "ogldev_vulkan_skybox_hdr.h"
#include "ogldev_vulkan_shader.h"

namespace OgldevVK {

enum Binding {
	BindingUniform = 0,
	BindingCubeMapFS = 1,
	NumBindings = 2
};


void SkyboxHDR::Init(VulkanCore& vkCore, VkDescriptorPool DescPool,
					 const VulkanTexture& Cubemap, VkFormat ColorFormat) {
    m_pVulkanCore = &vkCore;

    std::string VsPath = GetOgldevPath("Vulkan/VulkanCore/Shaders/skybox_hdr.vert");
    std::string FsPath = GetOgldevPath("Vulkan/VulkanCore/Shaders/skybox_hdr.frag");

	m_vs = CreateShaderModuleFromText(vkCore.GetDevice(), VsPath.c_str());
	m_fs = CreateShaderModuleFromText(vkCore.GetDevice(), FsPath.c_str());

    m_program.Init(vkCore, ColorFormat, DescPool, m_vs, m_fs, NULL, NULL);

	m_uniformBuffers = vkCore.CreateUniformBuffers(sizeof(UniformData));

    CreateDescriptorSets(Cubemap);
}


void SkyboxHDR::Destroy() 
{
    for (BufferAndMemory& ub : m_uniformBuffers) {
        ub.Destroy(m_pVulkanCore->GetDevice());
    }

    vkDestroyShaderModule(m_pVulkanCore->GetDevice(), m_vs, NULL);
    vkDestroyShaderModule(m_pVulkanCore->GetDevice(), m_fs, NULL);
    
	m_program.Destroy();
}


void SkyboxHDR::CreateDescriptorSets(const VulkanTexture& Cubemap)
{
    int NumImages = m_pVulkanCore->GetNumImages();

    m_program.AllocDescSets(NumImages, m_descSets);

	std::vector<VkDescriptorBufferInfo> BufferInfoUniforms(NumImages);
	std::vector<VkDescriptorImageInfo> CubemapImageInfos(NumImages);

	for (int i = 0; i < NumImages; i++) {
		BufferInfoUniforms[i].buffer = m_uniformBuffers[i].m_buffer;
		BufferInfoUniforms[i].offset = 0;
		BufferInfoUniforms[i].range = VK_WHOLE_SIZE;

		CubemapImageInfos[i].sampler = Cubemap.m_sampler;
		CubemapImageInfos[i].imageView = Cubemap.m_view;
		CubemapImageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}

    std::vector<VkWriteDescriptorSet> WriteDescriptorSet(NumImages * NumBindings);

	u32 WdsIndex = 0;

	for (int i = 0; i < NumImages; i++) {
		const VkDescriptorSet& DstSet = m_descSets[i];

		VkWriteDescriptorSet wds = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = DstSet,
			.dstBinding = BindingUniform,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.pBufferInfo = &BufferInfoUniforms[i]
		};

		assert(WdsIndex < WriteDescriptorSet.size());
		WriteDescriptorSet[WdsIndex++] = wds;

        wds = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = DstSet,
            .dstBinding = BindingCubeMapFS,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &CubemapImageInfos[i]
        };

		assert(WdsIndex < WriteDescriptorSet.size());
		WriteDescriptorSet[WdsIndex++] = wds;
	}

	vkUpdateDescriptorSets(m_pVulkanCore->GetDevice(), (u32)WriteDescriptorSet.size(), WriteDescriptorSet.data(), 0, NULL);
}


void SkyboxHDR::Update(int ImageIndex, const glm::mat4& VPNoTranslate, float Exposure)
{
	UniformData Data = { VPNoTranslate, glm::vec4(Exposure, 0.0f, 0.0f, 0.0f) };
	m_uniformBuffers[ImageIndex].Update(m_pVulkanCore->GetDevice(), &Data, sizeof(Data));
}


void SkyboxHDR::RecordCommandBuffer(VkCommandBuffer CmdBuf, int ImageIndex)
{
	m_program.Bind(CmdBuf, m_descSets[ImageIndex]);
	int NumVertices = 36;
	int InstanceCount = 1;
	int BaseVertex = 0;
	int FirstInstance = 0;

	vkCmdDraw(CmdBuf, NumVertices, InstanceCount, BaseVertex, FirstInstance);
}


std::vector<VkDescriptorSetLayout> SkyboxHDR::SkyboxProgram::CreateDescSetLayout(VulkanCore& vkCore)
{
	std::vector<VkDescriptorSetLayoutBinding> Bindings {
		{
			.binding = BindingUniform,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			.pImmutableSamplers = NULL
		}, {
			.binding = BindingCubeMapFS,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.pImmutableSamplers = NULL
		}
	};

	VkDescriptorSetLayout l = vkCore.CreateDescSetLayout({ Bindings });

	return { l };
}


}