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

	Vulkan For Beginners - 
		Tutorial #28: Compute Shaders
*/

#include <array>
#include <stdio.h>
#include <stdlib.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "imGuIZMOquat.h"			// optional

#include "ogldev_vulkan_util.h"
#include "ogldev_vulkan_core.h"
#include "ogldev_vulkan_wrapper.h"
#include "ogldev_vulkan_shader.h"
#include "ogldev_vulkan_compute_pipeline.h"
#include "ogldev_vulkan_glfw.h"
#include "ogldev_glm_camera.h"
#include "ogldev_vulkan_imgui.h"

#define WINDOW_WIDTH 2560
#define WINDOW_HEIGHT 1440

#define APP_NAME "Tutorial 28"

static int CS_OUTPUT_WIDTH = 1280;
static int CS_OUTPUT_HEIGHT = 720;

#if defined(_MSC_VER)
#pragma pack(push,1)
#endif

struct UniformData {
	float time;
};

#if defined(_MSC_VER)
#pragma pack(pop)
#endif


class PipelineProgram {
public:
	PipelineProgram() {}


	void Init(OgldevVK::VulkanCore& vkCore, VkDescriptorPool DescPool, const char* pVSFilename, const char* pFSFilename)
	{
		m_descPool = DescPool;

		m_device = vkCore.GetDevice();

		m_vs = OgldevVK::CreateShaderModuleFromText(m_device, pVSFilename);

		m_fs = OgldevVK::CreateShaderModuleFromText(m_device, pFSFilename);
		
		m_descSetLayout = CreateDescSetLayout(vkCore);
		
		m_pipeline = CreatePipeline(vkCore.GetWindow(), m_vs, m_fs,
						vkCore.GetSwapChainFormat(), vkCore.GetDepthFormat(), VK_COMPARE_OP_LESS);
	}


	void Destroy()
	{
		vkDestroyShaderModule(m_device, m_vs, NULL);
		vkDestroyShaderModule(m_device, m_fs, NULL);
		vkDestroyDescriptorSetLayout(m_device, m_descSetLayout, NULL);
		vkDestroyPipelineLayout(m_device, m_pipelineLayout, NULL);		
		vkDestroyPipeline(m_device, m_pipeline, NULL);
	}


	void Bind(VkCommandBuffer CmdBuf, VkDescriptorSet DescSet)
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


	void AllocDescSets(int DescCount, std::vector<VkDescriptorSet>& DescriptorSets)
	{
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


protected:

	virtual VkDescriptorSetLayout CreateDescSetLayout(OgldevVK::VulkanCore& vkCore) = 0;	

	// Default but deriving class may override
	virtual VkCullModeFlags GetCullMode()
	{
		return VK_CULL_MODE_BACK_BIT;
	}

	VkDevice m_device = VK_NULL_HANDLE;

private:

	VkPipeline CreatePipeline(GLFWwindow* pWindow, VkShaderModule vs, VkShaderModule fs,
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

	VkDescriptorSetLayout m_descSetLayout = VK_NULL_HANDLE;
	VkDescriptorPool m_descPool = VK_NULL_HANDLE;
	VkPipeline m_pipeline = VK_NULL_HANDLE;
	VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
	VkShaderModule m_vs = VK_NULL_HANDLE;
	VkShaderModule m_fs = VK_NULL_HANDLE;
};


class FullScreenQuadProgram : public PipelineProgram {
public:

	FullScreenQuadProgram() {}

	void UpdateDescSets(std::vector<VkDescriptorSet>& DescriptorSets, const OgldevVK::VulkanTexture& Tex)
	{
		std::vector<VkWriteDescriptorSet> WriteDescriptorSet(DescriptorSets.size());

		VkDescriptorImageInfo ImageInfo = {
			.sampler = Tex.m_sampler,
			.imageView = Tex.m_view,
			.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};

		for (int i = 0; i < (int)DescriptorSets.size(); i++) {
			VkDescriptorSet& DstSet = DescriptorSets[i];

			WriteDescriptorSet[i] = {
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.pNext = NULL,
				.dstSet = DstSet,
				.dstBinding = 2,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.pImageInfo = &ImageInfo,
				.pBufferInfo = NULL,
				.pTexelBufferView = NULL
			};
		}

		vkUpdateDescriptorSets(m_device, (u32)WriteDescriptorSet.size(), WriteDescriptorSet.data(), 0, NULL);
	}


	void RecordCommandBuffer(VkCommandBuffer CmdBuf)
	{
		u32 VertexCount = 3;
		u32 InstanceCount = 1;
		u32 FirstVertex = 0;
		u32 FirstInstance = 0;

		vkCmdDraw(CmdBuf, VertexCount, InstanceCount, FirstVertex, FirstInstance);
	}

protected:

	virtual VkDescriptorSetLayout CreateDescSetLayout(OgldevVK::VulkanCore& vkCore)
	{
		VkDescriptorSetLayoutBinding Binding = {
			.binding = 2,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.pImmutableSamplers = NULL
		};

		return vkCore.CreateDescSetLayout({ Binding });
	}

	virtual VkCullModeFlags GetCullMode()
	{
		return VK_CULL_MODE_FRONT_BIT;
	}
};


class VulkanApp : public OgldevVK::GLFWCallbacks
{
public:

	VulkanApp(int WindowWidth, int WindowHeight)
	{
		m_windowWidth = WindowWidth;
		m_windowHeight = WindowHeight;
	}

	~VulkanApp()
	{
		m_vkCore.FreeCommandBuffers((u32)m_cmdBufs.WithGUI.size(), m_cmdBufs.WithGUI.data());
		m_vkCore.FreeCommandBuffers((u32)m_cmdBufs.WithoutGUI.size(), m_cmdBufs.WithoutGUI.data());
		
		delete m_pComputePipeline;

		m_fsQuadProgram.Destroy();
		vkDestroyDescriptorPool(m_device, m_descPool, NULL);
			
		m_csOutput.Destroy(m_device);

		for (OgldevVK::BufferAndMemory& ubo : m_ubos) {
			ubo.Destroy(m_device);
		}

		m_imGUIRenderer.Destroy();
	}

	void Init(const char* pAppName)
	{
		m_pWindow = OgldevVK::glfw_vulkan_init(WINDOW_WIDTH, WINDOW_HEIGHT, pAppName);

		m_vkCore.Init(pAppName, m_pWindow, true, true);
		m_device = m_vkCore.GetDevice();
		m_numImages = m_vkCore.GetNumImages();
		m_pQueue = m_vkCore.GetQueue();

		VkImageUsageFlags Usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
		VkFormat Format = VK_FORMAT_R8G8B8A8_UNORM;

		m_vkCore.CreateTexture(m_csOutput, CS_OUTPUT_WIDTH, CS_OUTPUT_HEIGHT, Usage, Format);
		
		m_descPool = m_vkCore.CreateDescPool(1, 0, 0, m_numImages);

		CreatePipelines();
		CreateCommandBuffers();
		RecordCommandBuffers();
		DefaultCreateCameraPers();

		// The object is ready to receive callbacks
		OgldevVK::glfw_vulkan_set_callbacks(m_pWindow, this);
		m_imGUIRenderer.Init(&m_vkCore);
	}


	void RenderScene()
	{
		u32 ImageIndex = m_pQueue->AcquireNextImage();

		UpdateUniformBuffers(ImageIndex);

		if (m_showGui) {		
			UpdateGUI();

			VkCommandBuffer ImGUICmdBuf = m_imGUIRenderer.PrepareCommandBuffer(ImageIndex);

			VkCommandBuffer CmdBufs[] = { m_cmdBufs.WithGUI[ImageIndex], ImGUICmdBuf};

			m_pQueue->SubmitAsync(&CmdBufs[0], 2);
		} else {
			m_pQueue->SubmitAsync(m_cmdBufs.WithoutGUI[ImageIndex]);
		}

		m_pQueue->Present(ImageIndex);
	}

	
	void Key(GLFWwindow* pWindow, int Key, int Scancode, int Action, int Mods)
	{
		bool Handled = true;
		bool Press = Action != GLFW_RELEASE;

		switch (Key) {
	    case GLFW_KEY_SPACE:
	        if (Press) {
	            m_showGui = !m_showGui;
	        }
	        break;
		
		case GLFW_KEY_ESCAPE:
		case GLFW_KEY_Q:
			if (Press) {
				glfwSetWindowShouldClose(pWindow, GLFW_TRUE);
			}
			break;

		case GLFW_KEY_C:
			m_pGameCamera->Print();
			break;

		default:
			Handled = false;
		}

		if (!Handled) {
			Handled = GLFWCameraHandler(m_pGameCamera->m_movement, Key, Action, Mods);
		}
	}
	
	
	void MouseMove(GLFWwindow* pWindow, double x, double y)
	{
		m_pGameCamera->SetMousePos((float)x, (float)y);
	}


	void MouseButton(GLFWwindow* pWindow, int Button, int Action, int Mods)
	{
		if (!IsMouseControlledByImGUI()) {
			m_pGameCamera->HandleMouseButton(Button, Action, Mods);
		}		
	}
	
	
	void Execute()
	{
		float CurTime = (float)glfwGetTime();

		int Frames = 0;
		float FPSTime = 0.0f;
		while (!glfwWindowShouldClose(m_pWindow)) {
			float Time = (float)glfwGetTime();
			float dt = Time - CurTime;
			m_pGameCamera->Update(dt);
			RenderScene();
			CurTime = Time;
			glfwPollEvents();

			Frames++;
			FPSTime += dt;

			if (FPSTime >= 1.0f) {
				//printf("%d\n", Frames);
				char Title[256];
				snprintf(Title, sizeof(Title), "%s : FPS %d\n", APP_NAME, Frames);
				glfwSetWindowTitle(m_pWindow, Title);
				FPSTime = 0.0f;
				Frames = 0;
			}
		}

		glfwTerminate();
	}


private:

	void DefaultCreateCameraPers()
	{
		float FOV = 45.0f;
		float zNear = 0.1f;
		float zFar = 1500.0f;

		DefaultCreateCameraPers(FOV, zNear, zFar);
	}


	void DefaultCreateCameraPers(float FOV, float zNear, float zFar)
	{
		if ((m_windowWidth == 0) || (m_windowHeight == 0)) {
			printf("Invalid window dims: width %d height %d\n", m_windowWidth, m_windowHeight);
			exit(1);
		}

		if (m_pGameCamera) {
			printf("Camera already initialized\n");
			exit(1);
		}

		PersProjInfo persProjInfo = { FOV, (float)m_windowWidth, (float)m_windowHeight,
									  zNear, zFar };
		
		glm::vec3 Pos(0.0f, 0.0f, -0.33f);
		glm::vec3 Target(-1.0f, -0.1f, 0.0f);
		glm::vec3 Up(0.0, 1.0f, 0.0f);

		m_pGameCamera = new GLMCameraFirstPerson(Pos, Target, Up, persProjInfo);
	}


	void CreateCommandBuffers()
	{
		m_cmdBufs.WithGUI.resize(m_numImages);
		m_vkCore.CreateCommandBuffers(m_numImages, m_cmdBufs.WithGUI.data());

		m_cmdBufs.WithoutGUI.resize(m_numImages);
		m_vkCore.CreateCommandBuffers(m_numImages, m_cmdBufs.WithoutGUI.data());

		printf("Created command buffers\n");
	}

	
	void CreatePipelines()
	{
		CreateGraphicsPipeline();
		CreateComputePipeline();
	}


	void CreateGraphicsPipeline()
	{
		m_fsQuadProgram.Init(m_vkCore, m_descPool, "../VulkanCore/Shaders/FSQuad.vert", "test.frag");
		m_fsQuadProgram.AllocDescSets(m_numImages, m_descSets);
		m_fsQuadProgram.UpdateDescSets(m_descSets, m_csOutput);		
	}


	void CreateComputePipeline()
	{
		m_pComputePipeline = new OgldevVK::ComputePipeline(m_vkCore, "test.comp");

		m_ubos = m_vkCore.CreateUniformBuffers(sizeof(UniformData));

		m_pComputePipeline->UpdateDescriptorSets(m_csOutput, m_ubos);
	}


	void RecordCommandBuffers()
	{
		RecordCommandBuffersInternal(true, m_cmdBufs.WithoutGUI);

		RecordCommandBuffersInternal(false, m_cmdBufs.WithGUI);
	}


	void RecordCommandBuffersInternal(bool WithSecondBarrier, std::vector<VkCommandBuffer>& CmdBufs)
	{
		for (uint i = 0; i < CmdBufs.size(); i++) {
			VkCommandBuffer& CmdBuf = CmdBufs[i];			

			OgldevVK::BeginCommandBuffer(CmdBuf, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

			m_csOutput.ImageMemoryBarrier(CmdBuf,
										  VK_IMAGE_LAYOUT_GENERAL,                 // new layout
										  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,   // srcStage: last use was sampling
										  VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);     // dstStage: next use is compute write

			m_pComputePipeline->RecordCommandBuffer(i, CmdBuf, CS_OUTPUT_WIDTH / 16, CS_OUTPUT_HEIGHT / 16, 1);

			m_csOutput.ImageMemoryBarrier(CmdBuf,
										  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,  // new layout
										  VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,      // srcStage: last use was sampling
										  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);    // dstStage: next use is compute write

			OgldevVK::ImageMemBarrier(CmdBuf, m_vkCore.GetImage(i), m_vkCore.GetSwapChainFormat(),
				                      VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1);

			BeginRendering(CmdBuf, i);
		
			m_fsQuadProgram.Bind(CmdBuf, m_descSets[i]);

			m_fsQuadProgram.RecordCommandBuffer(CmdBuf);

			vkCmdEndRendering(CmdBuf);

			if (WithSecondBarrier) {
				OgldevVK::ImageMemBarrier(CmdBuf, m_vkCore.GetImage(i), m_vkCore.GetSwapChainFormat(),
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, 1);
			}

			VkResult res = vkEndCommandBuffer(CmdBuf);

			CHECK_VK_RESULT(res, "vkEndCommandBuffer\n");
		}

		printf("Command buffers recorded\n");
	}


	void BeginRendering(VkCommandBuffer CmdBuf, int ImageIndex)
	{
		VkClearValue ClearColor = {
			.color = {1.0f, 0.0f, 0.0f, 1.0f},
		};

		VkClearValue DepthValue = {
			.depthStencil = {.depth = 1.0f, .stencil = 0 }
		};

		m_vkCore.BeginDynamicRendering(CmdBuf, ImageIndex, &ClearColor, &DepthValue);
	}


	void UpdateGUI()
	{		
		ImGuiIO& io = ImGui::GetIO();

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();

		ImGui::Begin("Hello, world!", NULL, ImGuiWindowFlags_AlwaysAutoResize);   // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)

		static float f = 0.0f;
		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("Clear color", (float*)&m_clearColor); // Edit 3 floats representing a color

		ImGui::Begin("Transform");

		if (ImGui::CollapsingHeader("Position")) {
			ImGui::DragFloat3("##Position", &m_position.x, 0.01f);
			ImGui::SameLine();
			if (ImGui::Button("Reset##Pos")) {
				m_position = glm::vec3(0.0f);
			}
		}

		if (ImGui::CollapsingHeader("Rotation")) {
			ImGui::DragFloat3("##Rotation", &m_rotation.x, 1.0f); // Degrees
			ImGui::SameLine();
			if (ImGui::Button("Reset##Rot")) {
				m_rotation = glm::vec3(0.0f);
			}
		}

		if (ImGui::CollapsingHeader("Scale")) {
			ImGui::DragFloat("##Scale", &m_scale, 0.001f, 0.001f, 2.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
			ImGui::SameLine();
			if (ImGui::Button("Reset##Scale")) {
				m_scale = 1.0f;
			}
		}

		// For imGuIZMO, declare static or global variable or member class quaternion
		static quat qRot1 = quat(1.f, 0.f, 0.f, 0.f);
		static quat qRot2 = quat(1.f, 0.f, 0.f, 0.f);
		ImGui::gizmo3D("##gizmo1", qRot1, 200.0f/*, mode */);

		static vec4 dir(1.0f, 0.0f, 0.0f, 0.0f);
		ImGui::gizmo3D("##Dir1", dir, 200.0f, imguiGizmo::modeDirection);

		static int counter = 0;

		if (ImGui::Button("Button")) {  // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		}

		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		ImGui::End();
		ImGui::End();

		ImGui::Render();
	}


	void UpdateUniformBuffers(uint32_t ImageIndex)
	{		
		UniformData ud = {
			.time = (float)glfwGetTime()
		};

		m_ubos[ImageIndex].Update(m_device, &ud, sizeof(ud));
	}

	GLFWwindow* m_pWindow = NULL;
	OgldevVK::VulkanCore m_vkCore;
	OgldevVK::VulkanQueue* m_pQueue = NULL;
	VkDevice m_device = NULL;
	int m_numImages = 0;
	struct {
		std::vector<VkCommandBuffer> WithGUI;
		std::vector<VkCommandBuffer> WithoutGUI;
	} m_cmdBufs;
	
	OgldevVK::ComputePipeline* m_pComputePipeline = NULL;
	GLMCameraFirstPerson* m_pGameCamera = NULL;
	OgldevVK::ImGUIRenderer m_imGUIRenderer;
	int m_windowWidth = 0;
	int m_windowHeight = 0;
	bool m_showGui = false;
	glm::vec3 m_clearColor = glm::vec3(1.0f);
	glm::vec3 m_position = glm::vec3(0.0f);
	glm::vec3 m_rotation = glm::vec3(0.0f);
	float m_scale = 0.1f;	
	std::vector<OgldevVK::BufferAndMemory> m_ubos;
	OgldevVK::VulkanTexture m_csOutput;
	FullScreenQuadProgram m_fsQuadProgram;
	VkDescriptorPool m_descPool = VK_NULL_HANDLE;
	std::vector<VkDescriptorSet> m_descSets;	
};


int main(int argc, char* argv[])
{
	VulkanApp App(WINDOW_WIDTH, WINDOW_HEIGHT);

	App.Init(APP_NAME);

	App.Execute();

	return 0;
}