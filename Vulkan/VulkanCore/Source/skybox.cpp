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

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ogldev_vulkan_skybox.h"
#include "ogldev_vulkan_shader.h"


namespace OgldevVK {

#define UNIFORM_BUFFER_SIZE sizeof(glm::mat4)


void Skybox::Init(VulkanCore* pVulkanCore, const char* pFilename)
{
	m_pVulkanCore = pVulkanCore; 
	m_numImages = pVulkanCore->GetNumImages();
	
	m_uniformBuffers = pVulkanCore->CreateUniformBuffers(UNIFORM_BUFFER_SIZE);
	
	m_cubemapTex.Init(pVulkanCore);
	m_cubemapTex.LoadEctCubemap(pFilename, false);
	
	m_vs = CreateShaderModuleFromText(pVulkanCore->GetDevice(), "../VulkanCore/Shaders/skybox.vert");
	m_fs = CreateShaderModuleFromText(pVulkanCore->GetDevice(), "../VulkanCore/Shaders/skybox.frag");
	
	OgldevVK::PipelineDesc pd;
	pd.Device = pVulkanCore->GetDevice();
	pd.pWindow = pVulkanCore->GetWindow();
	pd.vs = m_vs;
	pd.fs = m_fs;
	pd.NumImages = m_numImages;
	pd.ColorFormat = m_pVulkanCore->GetSwapChainFormat();
	pd.DepthFormat = m_pVulkanCore->GetDepthFormat();
	pd.IsTexCube = true;
	pd.IsUniform = true;
	
	m_pPipeline = new OgldevVK::GraphicsPipelineV2(pd);

	CreateDescriptorSets();
}


void Skybox::Destroy()
{
	for (int i = 0; i < m_uniformBuffers.size(); i++) {
		m_uniformBuffers[i].Destroy(m_pVulkanCore->GetDevice());
	}

	vkDestroyShaderModule(m_pVulkanCore->GetDevice(), m_vs, NULL);
	vkDestroyShaderModule(m_pVulkanCore->GetDevice(), m_fs, NULL);

	m_cubemapTex.Destroy(m_pVulkanCore->GetDevice());

	delete m_pPipeline;
}
	

void Skybox::CreateDescriptorSets()
{
	int NumSubmeshes = 1;
	m_pPipeline->AllocateDescriptorSets(NumSubmeshes, m_descriptorSets);

	int NumBindings = 2; // Uniform, Cubemap

	std::vector<VkWriteDescriptorSet> WriteDescriptorSet(m_numImages * NumBindings);

	VkDescriptorImageInfo ImageInfo = {
		.sampler = m_cubemapTex.m_sampler,
		.imageView = m_cubemapTex.m_view,
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	};

	std::vector<VkDescriptorBufferInfo> BufferInfo_Uniforms(m_numImages);

	int WdsIndex = 0;

	for (int ImageIndex = 0; ImageIndex < m_numImages; ImageIndex++) {
		BufferInfo_Uniforms[ImageIndex].buffer = m_uniformBuffers[ImageIndex].m_buffer;
		BufferInfo_Uniforms[ImageIndex].offset = 0;
		BufferInfo_Uniforms[ImageIndex].range = VK_WHOLE_SIZE;
	}

	for (int ImageIndex = 0; ImageIndex < m_numImages; ImageIndex++) {
		VkDescriptorSet DstSet = m_descriptorSets[ImageIndex][0];

		VkWriteDescriptorSet wds = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = DstSet,
			.dstBinding = BindingUniform,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.pBufferInfo = &BufferInfo_Uniforms[ImageIndex]
		};

		assert(WdsIndex < WriteDescriptorSet.size());
		WriteDescriptorSet[WdsIndex++] = wds;

		wds = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = DstSet,
			.dstBinding = BindingTextureCube,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.pImageInfo = &ImageInfo
		};

		assert(WdsIndex < WriteDescriptorSet.size());
		WriteDescriptorSet[WdsIndex++] = wds;
	}

	vkUpdateDescriptorSets(m_pVulkanCore->GetDevice(), 
		                   (u32)WriteDescriptorSet.size(), WriteDescriptorSet.data(), 0, NULL);
}


void Skybox::RecordCommandBuffer(VkCommandBuffer CmdBuf, int ImageIndex)
{
	m_pPipeline->Bind(CmdBuf);

	vkCmdBindDescriptorSets(CmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
							m_pPipeline->GetPipelineLayout(),
							0,  // firstSet
							1,  // descriptorSetCount
							m_descriptorSets[ImageIndex].data(),
							0,	// dynamicOffsetCount
							NULL);	// pDynamicOffsets

	int NumVertices = 36;
	int InstanceCount = 1;
	int BaseVertex = 0;
	int FirstInstance = 0;

	vkCmdDraw(CmdBuf, NumVertices, InstanceCount, BaseVertex, FirstInstance);
}


void Skybox::Update(int ImageIndex, const glm::mat4& Transformation)
{
	m_uniformBuffers[ImageIndex].Update(m_pVulkanCore->GetDevice(), glm::value_ptr(Transformation), sizeof(Transformation));
}

};