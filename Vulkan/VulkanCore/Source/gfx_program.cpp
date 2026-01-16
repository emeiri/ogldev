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
#include "ogldev_vulkan_pipeline_program.h"
#include "ogldev_vulkan_shader.h"

namespace OgldevVK {

void GraphicsPipeline::Init(VulkanCore& vkCore, VkDescriptorPool DescPool, const char* pVSFilename, const char* pFSFilename)
{
	m_descPool = DescPool;

	m_device = vkCore.GetDevice();

	m_vs = CreateShaderModuleFromText(m_device, pVSFilename);

	m_fs = CreateShaderModuleFromText(m_device, pFSFilename);

	m_descSetLayout = CreateDescSetLayout(vkCore);

	m_pipeline = CreatePipeline(vkCore.GetWindow(), m_vs, m_fs,
		vkCore.GetSwapChainFormat(), vkCore.GetDepthFormat(), VK_COMPARE_OP_LESS);
}


void GraphicsPipeline::Destroy()
{
	vkDestroyShaderModule(m_device, m_vs, NULL);
	vkDestroyShaderModule(m_device, m_fs, NULL);
	vkDestroyDescriptorSetLayout(m_device, m_descSetLayout, NULL);
	vkDestroyPipelineLayout(m_device, m_pipelineLayout, NULL);
	vkDestroyPipeline(m_device, m_pipeline, NULL);
}


void GraphicsPipeline::Bind(VkCommandBuffer CmdBuf, VkDescriptorSet DescSet)
{
	vkCmdBindPipeline(CmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

	vkCmdBindDescriptorSets(CmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
		m_pipelineLayout,
		0,      // firstSet
		1,      // descriptorSetCount						
		&DescSet,
		0,	    // dynamicOffsetCount
		NULL);	// pDynamicOffsets
}


void GraphicsPipeline::AllocDescSets(int DescCount, std::vector<VkDescriptorSet>& DescriptorSets)
{
	assert(DescriptorSets.size() == 0);

	std::vector<VkDescriptorSetLayout> Layouts(DescCount, m_descSetLayout);

	VkDescriptorSetAllocateInfo AllocInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.pNext = NULL,
		.descriptorPool = m_descPool,
		.descriptorSetCount = (u32)Layouts.size(),
		.pSetLayouts = Layouts.data()
	};

	DescriptorSets.resize(DescCount);

	VkResult res = vkAllocateDescriptorSets(m_device, &AllocInfo, DescriptorSets.data());
	CHECK_VK_RESULT(res, "vkAllocateDescriptorSets");
}



VkPipeline GraphicsPipeline::CreatePipeline(GLFWwindow* pWindow, VkShaderModule vs, VkShaderModule fs,
	                                       VkFormat ColorFormat, VkFormat DepthFormat, VkCompareOp DepthCompareOp)
{
	VkPipelineShaderStageCreateInfo ShaderStageCreateInfo[2] = {
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = vs,
			.pName = "main",
		},
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = fs,
			.pName = "main"
		}
	};

	VkPipelineVertexInputStateCreateInfo VertexInputInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
	};

	VkPipelineInputAssemblyStateCreateInfo PipelineIACreateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE
	};

	int WindowWidth, WindowHeight;
	glfwGetWindowSize(pWindow, &WindowWidth, &WindowHeight);

	VkViewport VP = {
		.x = 0.0f,
		.y = 0.0f,
		.width = (float)WindowWidth,
		.height = (float)WindowHeight,
		.minDepth = 0.0f,
		.maxDepth = 1.0f
	};

	VkRect2D Scissor = {
		.offset = {
			.x = 0,
			.y = 0,
		},
		.extent = {
			.width = (u32)WindowWidth,
			.height = (u32)WindowHeight
		}
	};

	VkPipelineViewportStateCreateInfo VPCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.pViewports = &VP,
		.scissorCount = 1,
		.pScissors = &Scissor
	};

	VkPipelineRasterizationStateCreateInfo RastCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = GetCullMode(),
		.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
		.lineWidth = 1.0f
	};

	VkPipelineMultisampleStateCreateInfo PipelineMSCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable = VK_FALSE,
		.minSampleShading = 1.0f
	};

	VkPipelineDepthStencilStateCreateInfo DepthStencilState = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.depthTestEnable = VK_TRUE,
		.depthWriteEnable = VK_TRUE,
		.depthCompareOp = DepthCompareOp,
		.depthBoundsTestEnable = VK_FALSE,
		.stencilTestEnable = VK_FALSE,
		.front = {},
		.back = {},
		.minDepthBounds = 0.0f,
		.maxDepthBounds = 1.0f
	};

	VkPipelineColorBlendAttachmentState BlendAttachState = {
		.blendEnable = VK_FALSE,
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
	};

	VkPipelineColorBlendStateCreateInfo BlendCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = 1,
		.pAttachments = &BlendAttachState
	};

	VkPipelineRenderingCreateInfo RenderingInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
		.pNext = NULL,
		.viewMask = 0,
		.colorAttachmentCount = 1,
		.pColorAttachmentFormats = &ColorFormat,
		.depthAttachmentFormat = DepthFormat,
		.stencilAttachmentFormat = VK_FORMAT_UNDEFINED
	};

	std::vector<VkDescriptorSetLayout> SetLayouts = { m_descSetLayout };
	VkPipelineLayoutCreateInfo LayoutInfo = {};

	LayoutInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = (u32)SetLayouts.size(),
		.pSetLayouts = SetLayouts.data()
	};

	VkResult res = vkCreatePipelineLayout(m_device, &LayoutInfo, NULL, &m_pipelineLayout);
	CHECK_VK_RESULT(res, "vkCreatePipelineLayout\n");

	VkGraphicsPipelineCreateInfo PipelineInfo = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = &RenderingInfo,
		.stageCount = ARRAY_SIZE_IN_ELEMENTS(ShaderStageCreateInfo),
		.pStages = &ShaderStageCreateInfo[0],
		.pVertexInputState = &VertexInputInfo,
		.pInputAssemblyState = &PipelineIACreateInfo,
		.pViewportState = &VPCreateInfo,
		.pRasterizationState = &RastCreateInfo,
		.pMultisampleState = &PipelineMSCreateInfo,
		.pDepthStencilState = &DepthStencilState,
		.pColorBlendState = &BlendCreateInfo,
		.layout = m_pipelineLayout,
		.renderPass = VK_NULL_HANDLE,
		.subpass = 0,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = -1
	};

	VkPipeline Pipeline = VK_NULL_HANDLE;
	res = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &PipelineInfo, NULL, &Pipeline);
	CHECK_VK_RESULT(res, "vkCreateGraphicsPipelineV3s\n");

	printf("Graphics pipeline created\n");

	return Pipeline;
}

}