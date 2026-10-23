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
#include "ogldev_vulkan_shader.h"
#include "shadow_program.h"


namespace OgldevVK {

enum Binding {
	BindingVB = 0,
	BindingIB = 1,
	BindingUniformVS = 2,
	BindingMetaData = 3,
	BindingUniformFS = 4,
	BindingShadowMapFS = 5,
	NumBindings = 6
};


void ShadowProgram::Init(VulkanCore& vkCore, VkDescriptorPool DescPool, 
					     const std::vector<VkDescriptorSetLayout>& DescSetLayouts)
{
    m_numImages = vkCore.GetNumImages();
    m_lightingDescSetLayouts = DescSetLayouts;

	m_vs = OgldevVK::CreateShaderModuleFromText(vkCore.GetDevice(), "../Common/shadow.vert");

	m_fs = OgldevVK::CreateShaderModuleFromText(vkCore.GetDevice(), "../Common/shadow.frag");

	GraphicsPipeline::Init(vkCore, VK_FORMAT_UNDEFINED, DescPool, m_vs, m_fs, NULL, NULL);
}


std::vector<VkDescriptorSetLayout> ShadowProgram::CreateDescSetLayout(VulkanCore& vkCore)
{
	return m_lightingDescSetLayouts;                // reuse, don't create
}


void ShadowProgram::Destroy()
{
	GraphicsPipeline::Destroy(false);

	vkDestroyShaderModule(m_device, m_vs, NULL);
	vkDestroyShaderModule(m_device, m_fs, NULL);
}


void ShadowProgram::Bind(int ImageIndex, 
						 VkCommandBuffer CmdBuf, 
						 VkDescriptorSet DescSet, 
						 VkDescriptorSet TextureDescSet,
						 u32 LightIndex)
{
	std::vector<VkDescriptorSet> DescSets = { TextureDescSet, DescSet };
	GraphicsPipeline::Bind(CmdBuf, DescSets);

	vkCmdPushConstants(CmdBuf,
		m_pipelineLayout,
		VK_SHADER_STAGE_VERTEX_BIT, // Must match shader stage
		0,                            // Offset in push constant block
		sizeof(u32),				  // Size of data
		&LightIndex);
}


std::vector<VkPushConstantRange> ShadowProgram::GetPushConstantRange()
{
	std::vector<VkPushConstantRange> PushConstantRanges = {
		{
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			.offset = 0,
			.size = sizeof(u32)
		}
	};

	return PushConstantRanges;
}

}