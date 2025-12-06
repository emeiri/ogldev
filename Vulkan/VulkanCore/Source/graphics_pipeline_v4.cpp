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
#include "ogldev_vulkan_graphics_pipeline_v4.h"


namespace OgldevVK {

const u32 MAX_TEXTURES = 4096; // choose according to limits and memory

enum V4_Binding {
	V4_BindingVB = 0,
	V4_BindingIB = 1,
	V4_BindingTexture2D = 2,
	V4_BindingUniform = 3,
	V4_BindingMetaData = 4,
	V4_NumBindings = 5
};


GraphicsPipelineV4::GraphicsPipelineV4(const PipelineDesc& pd)
{
	if (pd.IsTexCube) {
		printf("GraphicsPipelineV4 doesn't support texcubes. Use GraphicsPipelineV2 instead.");
		exit(1);
	}

	m_device = pd.Device;
	m_numImages = pd.NumImages;

	CreateDescriptorSetLayout(pd.IsVB, pd.IsIB, pd.IsTex2D, pd.IsUniform);

	InitCommon(pd.pWindow, NULL, pd.vs, pd.fs, pd.ColorFormat, pd.DepthFormat, pd.DepthCompareOp);
}


GraphicsPipelineV4::~GraphicsPipelineV4()
{
	vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, NULL);
	vkDestroyPipelineLayout(m_device, m_pipelineLayout, NULL);
	vkDestroyDescriptorPool(m_device, m_descriptorPool, NULL);
	vkDestroyPipeline(m_device, m_pipeline, NULL);
}


void GraphicsPipelineV4::Bind(VkCommandBuffer CmdBuf)
{
	vkCmdBindPipeline(CmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
}


void GraphicsPipelineV4::InitCommon(GLFWwindow* pWindow, VkRenderPass RenderPass, 
									VkShaderModule vs, VkShaderModule fs,
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

	std::vector<VkDescriptorSetLayout> SetLayouts = { m_descriptorSetLayout };
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
		.pNext = RenderPass ? NULL : &RenderingInfo,
		.stageCount = ARRAY_SIZE_IN_ELEMENTS(ShaderStageCreateInfo),
		.pStages = &ShaderStageCreateInfo[0],
		.pVertexInputState = &VertexInputInfo,
		.pInputAssemblyState = &PipelineIACreateInfo,
		.pViewportState = &VPCreateInfo,
		.pRasterizationState = &RastCreateInfo,
		.pMultisampleState = &PipelineMSCreateInfo,
		.pDepthStencilState =&DepthStencilState,
		.pColorBlendState = &BlendCreateInfo,
		.layout = m_pipelineLayout,
		.renderPass = RenderPass,
		.subpass = 0,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = -1
	};

	res = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &PipelineInfo, NULL, &m_pipeline);
	CHECK_VK_RESULT(res, "vkCreateGraphicsPipelineV3s\n");

	printf("Graphics pipeline created\n");
}


void GraphicsPipelineV4::CreateDescriptorPool(u32 TextureCount, 
											  u32 UniformBufferCount, 
											  u32 StorageBufferCount, 
											  u32 MaxSets)
{
	// Pool sizes: each entry specifies how many descriptors of that type the pool can allocate.
	std::vector<VkDescriptorPoolSize> PoolSizes;

	if (TextureCount > 0) {
		VkDescriptorPoolSize TexSize = {
			.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = TextureCount
		};

		PoolSizes.push_back(TexSize);
	}

	if (UniformBufferCount > 0) {
		VkDescriptorPoolSize UboSize = {
			.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = UniformBufferCount
		};

		PoolSizes.push_back(UboSize);
	}

	if (StorageBufferCount > 0) {
		VkDescriptorPoolSize SsboSize = {
			.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = StorageBufferCount
		};

		PoolSizes.push_back(SsboSize);
	}

	// Optionally include VK_DESCRIPTOR_TYPE_SAMPLER or VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
	// if you use separate sampler/image bindings instead of combined descriptors.

	VkDescriptorPoolCreateInfo PoolCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.pNext = NULL,
		.flags = 0, 
		.maxSets = MaxSets,
		.poolSizeCount = (u32)PoolSizes.size(),
		.pPoolSizes = PoolSizes.empty() ? NULL : PoolSizes.data()
	};

	VkResult res = vkCreateDescriptorPool(m_device, &PoolCreateInfo, NULL, &m_descriptorPool);
	CHECK_VK_RESULT(res, "vkCreateDescriptorSetLayout");
}



void GraphicsPipelineV4::CreateDescriptorSetLayout(bool IsVB, bool IsIB, bool IsTex, bool IsUniform)
{
	std::vector<VkDescriptorSetLayoutBinding> LayoutBindings;

	if (IsVB) {
		VkDescriptorSetLayoutBinding VertexShaderLayoutBinding_VB = {
			.binding = V4_BindingVB,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		};

		LayoutBindings.push_back(VertexShaderLayoutBinding_VB);
	}

	if (IsIB) {
		VkDescriptorSetLayoutBinding VertexShaderLayoutBinding_IB = {
			.binding = V4_BindingIB,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		};

		LayoutBindings.push_back(VertexShaderLayoutBinding_IB);
	}

	if (IsTex) {
		VkDescriptorSetLayoutBinding VertexShaderLayoutBinding_Tex = {
			.binding = V4_BindingTexture2D,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = MAX_TEXTURES,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.pImmutableSamplers = NULL
		};

		LayoutBindings.push_back(VertexShaderLayoutBinding_Tex);
	}

	if (IsUniform) {
		VkDescriptorSetLayoutBinding VertexShaderLayoutBinding_Uniform = {
			.binding = V4_BindingUniform,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		};

		LayoutBindings.push_back(VertexShaderLayoutBinding_Uniform);
	}

	VkDescriptorSetLayoutBinding VertexShaderLayoutBinding_MetaData = {
		.binding = V4_BindingMetaData,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		.pImmutableSamplers = NULL
	};

	LayoutBindings.push_back(VertexShaderLayoutBinding_MetaData);

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


void GraphicsPipelineV4::AllocateDescriptorSets(std::vector<VkDescriptorSet>& DescriptorSets)
{
	u32 TextureCount = MAX_TEXTURES;
	u32 UniformBufferCount = m_numImages;
	u32 StorageBufferCount = 3;	// IB, VB, MetaData
	u32 MaxSets = m_numImages;

	CreateDescriptorPool(TextureCount, UniformBufferCount, StorageBufferCount, MaxSets);

	AllocateDescriptorSetsInternal(DescriptorSets);
}


void GraphicsPipelineV4::AllocateDescriptorSetsInternal(std::vector<VkDescriptorSet>& DescriptorSets)
{
	std::vector<VkDescriptorSetLayout> Layouts(m_numImages, m_descriptorSetLayout);

	VkDescriptorSetAllocateInfo AllocInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.pNext = NULL,
		.descriptorPool = m_descriptorPool,
		.descriptorSetCount = (u32)Layouts.size(),
		.pSetLayouts = Layouts.data()
	};

	DescriptorSets.resize(m_numImages);

    VkResult res = vkAllocateDescriptorSets(m_device, &AllocInfo, DescriptorSets.data());
	CHECK_VK_RESULT(res, "vkAllocateDescriptorSets");	
}


void GraphicsPipelineV4::UpdateDescriptorSets(const ModelDesc& ModelDesc,
											  std::vector<VkDescriptorSet>& DescriptorSets)
{
	int NumBindings = V4_NumBindings;

	std::vector<VkWriteDescriptorSet> WriteDescriptorSet(m_numImages * NumBindings);

	std::vector<VkDescriptorBufferInfo> BufferInfo_Uniforms(m_numImages);

	for (int ImageIndex = 0; ImageIndex < m_numImages; ImageIndex++) {		
		BufferInfo_Uniforms[ImageIndex].buffer = ModelDesc.m_uniforms[ImageIndex];
		BufferInfo_Uniforms[ImageIndex].offset = 0;
		BufferInfo_Uniforms[ImageIndex].range = VK_WHOLE_SIZE;
	}

	u32 TextureCount = (u32)ModelDesc.m_materials.size();
	std::vector<VkDescriptorImageInfo> ImageInfos;
	ImageInfos.resize(TextureCount);

	for (u32 i = 0; i < TextureCount; ++i) {
		ImageInfos[i].sampler = ModelDesc.m_materials[i].m_sampler;
		ImageInfos[i].imageView = ModelDesc.m_materials[i].m_imageView;
		ImageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
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

	u32 WdsIndex = 0;

	for (int ImageIndex = 0; ImageIndex < m_numImages; ImageIndex++) {
		VkDescriptorSet& DstSet = DescriptorSets[ImageIndex];

		VkWriteDescriptorSet wds = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = DstSet,
			.dstBinding = V4_BindingVB,
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
			.dstBinding = V4_BindingIB,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.pBufferInfo = &BufferInfo_IB
		};

		assert(WdsIndex < WriteDescriptorSet.size());
		WriteDescriptorSet[WdsIndex++] = wds;

		wds = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = NULL,
			.dstSet = DstSet,
			.dstBinding = V4_BindingTexture2D,
			.dstArrayElement = 0,
			.descriptorCount = TextureCount,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.pImageInfo = ImageInfos.data(),
			.pBufferInfo = NULL,
			.pTexelBufferView = NULL
		};

		assert(WdsIndex < WriteDescriptorSet.size());
		WriteDescriptorSet[WdsIndex++] = wds;

		wds = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = DstSet,
			.dstBinding = V4_BindingUniform,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.pBufferInfo = &BufferInfo_Uniforms[ImageIndex]
		};

		assert(WdsIndex < WriteDescriptorSet.size());
		WriteDescriptorSet[WdsIndex++] = wds;

		wds = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = NULL,
			.dstSet = DstSet,
			.dstBinding = V4_BindingMetaData,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.pImageInfo = NULL,
			.pBufferInfo = &MetaDataBufferInfo,
			.pTexelBufferView = NULL
		};

		assert(WdsIndex < WriteDescriptorSet.size());
		WriteDescriptorSet[WdsIndex++] = wds;
	}

	vkUpdateDescriptorSets(m_device, WdsIndex, WriteDescriptorSet.data(), 0, NULL);
}


}