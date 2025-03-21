/*
		Copyright 2024 Etay Meiri

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
#include "ogldev_vulkan_graphics_pipeline.h"

#define BINDING_VB      0
#define BINDING_IB      1
#define BINDING_UNIFORM 2
#define BINDING_TEXTURE 3


namespace OgldevVK {

GraphicsPipeline::GraphicsPipeline(VkDevice Device, GLFWwindow* pWindow, VkRenderPass RenderPass,
	VkShaderModule vs, VkShaderModule fs,
	const SimpleMesh* pMesh,
	int NumImages,
	std::vector<BufferAndMemory>& UniformBuffers,
	int UniformDataSize,
	bool DepthEnabled)
{
	m_device = Device;
	m_depthEnabled = DepthEnabled;
	m_numImages = NumImages;

	if (pMesh) {
		CreateDescriptorSets(pMesh, UniformBuffers, UniformDataSize);
	}

	const BufferAndMemory* pVB = pMesh ? &pMesh->m_vb : NULL;

	InitCommon(pWindow, RenderPass, pVB, vs, fs);
}


void GraphicsPipeline::InitCommon(GLFWwindow* pWindow, VkRenderPass RenderPass, const BufferAndMemory* pVB, VkShaderModule vs, VkShaderModule fs)
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

	VkRect2D Scissor{
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
		.cullMode = VK_CULL_MODE_BACK_BIT,
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
		.depthCompareOp = VK_COMPARE_OP_LESS,
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

	VkPipelineLayoutCreateInfo LayoutInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO
	};

	if (pVB && pVB->m_buffer) {
		LayoutInfo.setLayoutCount = 1;
		LayoutInfo.pSetLayouts = &m_descriptorSetLayout;
	} else {
		LayoutInfo.setLayoutCount = 0;
		LayoutInfo.pSetLayouts = NULL;
	}

	VkResult res = vkCreatePipelineLayout(m_device, &LayoutInfo, NULL, &m_pipelineLayout);
	CHECK_VK_RESULT(res, "vkCreatePipelineLayout\n");

	VkGraphicsPipelineCreateInfo PipelineInfo = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = ARRAY_SIZE_IN_ELEMENTS(ShaderStageCreateInfo),
		.pStages = &ShaderStageCreateInfo[0],
		.pVertexInputState = &VertexInputInfo,
		.pInputAssemblyState = &PipelineIACreateInfo,
		.pViewportState = &VPCreateInfo,
		.pRasterizationState = &RastCreateInfo,
		.pMultisampleState = &PipelineMSCreateInfo,
		.pDepthStencilState = m_depthEnabled ? &DepthStencilState : VK_NULL_HANDLE,
		.pColorBlendState = &BlendCreateInfo,
		.layout = m_pipelineLayout,
		.renderPass = RenderPass,
		.subpass = 0,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = -1
	};

	res = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &PipelineInfo, NULL, &m_pipeline);
	CHECK_VK_RESULT(res, "vkCreateGraphicsPipelines\n");

	printf("Graphics pipeline created\n");
}


GraphicsPipeline::GraphicsPipeline(VkDevice Device,
	GLFWwindow* pWindow,
	VkRenderPass RenderPass,
	VkShaderModule vs,
	VkShaderModule fs,
	const VkModel& Model,
	int NumImages)
{
	m_device = Device;
	m_depthEnabled = true;
	m_numImages = NumImages;

	CreateDescriptorPool();

	bool IsVB = true;
	bool IsIB = true;
	bool IsUniform = true;
	bool IsTex = true;
	CreateDescriptorSetLayout(IsVB, IsIB, IsUniform, IsTex);

	InitCommon(pWindow, RenderPass, Model.GetVB(), vs, fs);
}


GraphicsPipeline::~GraphicsPipeline()
{
	vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, NULL);
	vkDestroyPipelineLayout(m_device, m_pipelineLayout, NULL);
	vkDestroyDescriptorPool(m_device, m_descriptorPool, NULL);
	vkDestroyPipeline(m_device, m_pipeline, NULL);
}


void GraphicsPipeline::Bind(VkCommandBuffer CmdBuf, int ImageIndex)
{
	vkCmdBindPipeline(CmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

	if (m_descriptorSets.size() > 0) {
		vkCmdBindDescriptorSets(CmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 
								0,  // firstSet
								1,  // descriptorSetCount
								&m_descriptorSets[ImageIndex], 
								0,	// dynamicOffsetCount
								NULL);	// pDynamicOffsets
	}	
}


void GraphicsPipeline::CreateDescriptorSets(const SimpleMesh* pMesh, std::vector<BufferAndMemory>& UniformBuffers, 
											int UniformDataSize)
{
	CreateDescriptorPool();

	bool IsVB = true;
	bool IsIB = false;
	bool IsUniform = UniformBuffers.size() > 0;
	bool IsTex = pMesh->m_pTex != NULL;
	CreateDescriptorSetLayout(IsVB, IsIB, IsUniform, IsTex);

	m_descriptorSets = AllocateDescriptorSets();

	UpdateDescriptorSets(m_descriptorSets, &(pMesh->m_vb), NULL, pMesh->m_pTex, UniformBuffers, UniformDataSize);
}


void GraphicsPipeline::CreateDescriptorPool()
{
	VkDescriptorPoolCreateInfo PoolInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.flags = 0,
		.maxSets = m_numImages,
		.poolSizeCount = 0,
		.pPoolSizes = NULL
	};

	VkResult res = vkCreateDescriptorPool(m_device, &PoolInfo, NULL, &m_descriptorPool);
	CHECK_VK_RESULT(res, "vkCreateDescriptorPool");
	printf("Descriptor pool created\n");
}


void GraphicsPipeline::CreateDescriptorSetLayout(bool IsVB, bool IsIB, bool IsTex, bool IsUniform)
{
	std::vector<VkDescriptorSetLayoutBinding> LayoutBindings;

	if (IsVB) {
		VkDescriptorSetLayoutBinding VertexShaderLayoutBinding_VB = {
			.binding = BINDING_VB,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		};

		LayoutBindings.push_back(VertexShaderLayoutBinding_VB);
	}

	if (IsIB) {
		VkDescriptorSetLayoutBinding VertexShaderLayoutBinding_IB = {
			.binding = BINDING_IB,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		};

		LayoutBindings.push_back(VertexShaderLayoutBinding_IB);
	}

	if (IsUniform) {
		VkDescriptorSetLayoutBinding VertexShaderLayoutBinding_Uniform = {
			.binding = BINDING_UNIFORM,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		};

		LayoutBindings.push_back(VertexShaderLayoutBinding_Uniform);
	}

	if (IsTex) { 
		VkDescriptorSetLayoutBinding FragmentShaderLayoutBinding_Tex = {
			.binding = BINDING_TEXTURE,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		};

		LayoutBindings.push_back(FragmentShaderLayoutBinding_Tex);
	}

	VkDescriptorSetLayoutCreateInfo LayoutInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,			// reserved - must be zero
		.bindingCount = (u32)LayoutBindings.size(),
		.pBindings = LayoutBindings.data()
	};

	VkResult res = vkCreateDescriptorSetLayout(m_device, &LayoutInfo, NULL, &m_descriptorSetLayout);
	CHECK_VK_RESULT(res, "vkCreateDescriptorSetLayout");
}


std::vector<VkDescriptorSet> GraphicsPipeline::AllocateDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> Layouts(m_numImages, m_descriptorSetLayout);

	VkDescriptorSetAllocateInfo AllocInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.pNext = NULL,
		.descriptorPool = m_descriptorPool,
		.descriptorSetCount = (u32)Layouts.size(),
		.pSetLayouts = Layouts.data()
	};

	std::vector<VkDescriptorSet> DescriptorSets(m_numImages);

	VkResult res = vkAllocateDescriptorSets(m_device, &AllocInfo, DescriptorSets.data());
	CHECK_VK_RESULT(res, "vkAllocateDescriptorSets");

	return DescriptorSets;
}


void GraphicsPipeline::AllocateDescriptorSets(int NumSubmeshes, std::vector<std::vector<VkDescriptorSet>>& DescriptorSets)
{
	std::vector<VkDescriptorSetLayout> Layouts(NumSubmeshes, m_descriptorSetLayout);

	VkDescriptorSetAllocateInfo AllocInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.pNext = NULL,
		.descriptorPool = m_descriptorPool,
		.descriptorSetCount = (u32)Layouts.size(),
		.pSetLayouts = Layouts.data()
	};

	DescriptorSets.resize(m_numImages);

	for (int ImageIndex = 0; ImageIndex < DescriptorSets.size(); ImageIndex++) {
		DescriptorSets[ImageIndex].resize(NumSubmeshes);

		VkResult res = vkAllocateDescriptorSets(m_device, &AllocInfo, DescriptorSets[ImageIndex].data());
		CHECK_VK_RESULT(res, "vkAllocateDescriptorSets");
	}
}



void GraphicsPipeline::PrepareDescriptorSets(const ModelDesc& ModelDesc,
											 const std::vector<std::vector<VkDescriptorSet>>& DescriptorSets)
{
	std::vector<VkWriteDescriptorSet> WriteDescriptorSet;

	u32 NumSubmeshes = (u32)DescriptorSets[0].size();

	for (u32 ImageIndex = 0; ImageIndex < m_numImages; ImageIndex++) {
		for (u32 SubmeshIndex = 0; SubmeshIndex < NumSubmeshes; SubmeshIndex++) {
			VkDescriptorSet DstSet = DescriptorSets[ImageIndex][SubmeshIndex];

			VkDescriptorBufferInfo BufferInfo_VB = {
				.buffer = ModelDesc.m_vb,
				.offset = ModelDesc.m_ranges[SubmeshIndex].m_vbRange.m_offset,
				.range = ModelDesc.m_ranges[SubmeshIndex].m_vbRange.m_range
			};

			WriteDescriptorSet.push_back(
				VkWriteDescriptorSet{
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.dstSet = DstSet,
					.dstBinding = BINDING_VB,
					.dstArrayElement = 0,
					.descriptorCount = 1,
					.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
					.pBufferInfo = &BufferInfo_VB
				}
			);

			VkDescriptorBufferInfo BufferInfo_IB = {
				.buffer = ModelDesc.m_ib,
				.offset = ModelDesc.m_ranges[SubmeshIndex].m_ibRange.m_offset,
				.range = ModelDesc.m_ranges[SubmeshIndex].m_ibRange.m_range,
			};

			WriteDescriptorSet.push_back(
				VkWriteDescriptorSet{
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.dstSet = DstSet,
					.dstBinding = BINDING_IB,
					.dstArrayElement = 0,
					.descriptorCount = 1,
					.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
					.pBufferInfo = &BufferInfo_IB
				}
			);

			VkDescriptorBufferInfo BufferInfo_Uniform = {
				.buffer = ModelDesc.m_uniforms[ImageIndex],
				.offset = ModelDesc.m_ranges[SubmeshIndex].m_uniformRange.m_offset,
				.range = ModelDesc.m_ranges[SubmeshIndex].m_uniformRange.m_range,
			};

			WriteDescriptorSet.push_back(
				VkWriteDescriptorSet{
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.dstSet = DstSet,
					.dstBinding = BINDING_UNIFORM,
					.dstArrayElement = 0,
					.descriptorCount = 1,
					.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					.pBufferInfo = &BufferInfo_Uniform
				}
			);

			VkDescriptorImageInfo ImageInfo = {
				.sampler = ModelDesc.m_materials[SubmeshIndex].m_sampler,
				.imageView = ModelDesc.m_materials[SubmeshIndex].m_imageView,
				.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			};

			WriteDescriptorSet.push_back(
				VkWriteDescriptorSet{
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.dstSet = DstSet,
					.dstBinding = BINDING_TEXTURE,
					.dstArrayElement = 0,
					.descriptorCount = 1,
					.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.pImageInfo = &ImageInfo
				}
			);
		}
	}

	vkUpdateDescriptorSets(m_device, (u32)WriteDescriptorSet.size(), WriteDescriptorSet.data(), 0, NULL);
}


void GraphicsPipeline::UpdateDescriptorSets(std::vector<VkDescriptorSet>& DescriptorSets,
											const BufferAndMemory* pVB, const BufferAndMemory* pIB, VulkanTexture* pTex, 
										    std::vector<BufferAndMemory>& UniformBuffers, int UniformDataSize)
{
	VkDescriptorBufferInfo BufferInfo_VB = {
		.buffer = pVB->m_buffer,
		.offset = 0,
		.range = VK_WHOLE_SIZE
	};

	VkDescriptorBufferInfo BufferInfo_IB;

	if (pIB) {
		BufferInfo_IB.buffer = pIB->m_buffer;
		BufferInfo_IB.offset = 0;
		BufferInfo_IB.range = VK_WHOLE_SIZE;
	};

	VkDescriptorImageInfo ImageInfo;
	
	if (pTex) {
		ImageInfo.sampler = pTex->m_sampler;
		ImageInfo.imageView = pTex->m_view;
		ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}
	
	std::vector<VkWriteDescriptorSet> WriteDescriptorSet;

	for (u32 i = 0; i < m_numImages; i++) {

		WriteDescriptorSet.push_back(
			VkWriteDescriptorSet{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = DescriptorSets[i],
				.dstBinding = BINDING_VB,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				.pBufferInfo = &BufferInfo_VB
			}
		);

		if (pIB) {
			WriteDescriptorSet.push_back(
				VkWriteDescriptorSet{
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.dstSet = DescriptorSets[i],
					.dstBinding = BINDING_IB,
					.dstArrayElement = 0,
					.descriptorCount = 1,
					.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
					.pBufferInfo = &BufferInfo_IB
				}
			);
		}

		if (UniformBuffers.size() > 0) {
			VkDescriptorBufferInfo BufferInfo_Uniform = {
				.buffer = UniformBuffers[i].m_buffer,
				.offset = 0,
				.range = (VkDeviceSize)UniformDataSize,
			};

			WriteDescriptorSet.push_back(
				VkWriteDescriptorSet{
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.dstSet = DescriptorSets[i],
					.dstBinding = BINDING_UNIFORM,
					.dstArrayElement = 0,
					.descriptorCount = 1,
					.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					.pBufferInfo = &BufferInfo_Uniform
				}
			);
		}
		
		if (pTex) {
			WriteDescriptorSet.push_back(
				VkWriteDescriptorSet{
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.dstSet = DescriptorSets[i],
					.dstBinding = BINDING_TEXTURE,
					.dstArrayElement = 0,
					.descriptorCount = 1,
					.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.pImageInfo = &ImageInfo
				}
			);
		}
	}

	vkUpdateDescriptorSets(m_device, (u32)WriteDescriptorSet.size(), WriteDescriptorSet.data(), 0, NULL);
}

}