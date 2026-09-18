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

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "imGuIZMOquat.h"

#include "ogldev_vulkan_imgui.h"
#include "ogldev_vulkan_glfw.h"
#include "ogldev_vulkan_big_texture_array.h"
#include "ogldev_glm_camera.h"
#include "tone_mapping_program.h"
#include "lighting_program.h"
#include "ogldev_vulkan_model.h"


struct ModelContext {
	OgldevVK::VkModel* m_pModel = NULL;
	std::vector<VkDescriptorSet> m_descSets;
	std::vector<OgldevVK::BufferAndMemory> m_uniformBuffersVS;
	std::vector<OgldevVK::BufferAndMemory> m_uniformBuffersFS;
	int m_baseTextureIndex = 0;

	void Destroy(VkDevice Device)
	{
		for (OgldevVK::BufferAndMemory& ub : m_uniformBuffersVS) {
			ub.Destroy(Device);
		}

		for (OgldevVK::BufferAndMemory& ub : m_uniformBuffersFS) {
			ub.Destroy(Device);
		}
		delete m_pModel;
	}
};


class PreBakedRenderer : public OgldevVK::GLFWCallbacks
{
public:

	PreBakedRenderer(int WindowWidth, int WindowHeight, const std::string& AppName);

	~PreBakedRenderer();

	void Init(const std::string& AssetPath);

	void Key(GLFWwindow* pWindow, int Key, int Scancode, int Action, int Mods);

	void MouseMove(GLFWwindow* pWindow, double x, double y);

	void MouseButton(GLFWwindow* pWindow, int Button, int Action, int Mods);

	void Execute();

private:

	void InitCameraFromModel();

	void DefaultCreateCameraPers();

	void DefaultCreateCameraPers(float FOV, float zNear, float zFar);

	void CreateCommandBuffers();

	void CreateDescriptorPool();

	void InitBigTextureArray();

	void CreateOffscreenImages();

	void CreateMeshe(const std::string& AssetPath);

	void CreateShaders();

	void CreatePipelines();

	void CreateUniformBuffers(int MeshIndex);

	void CreateDescriptorSets(int MeshIndex, OgldevVK::ModelDesc& md);

	void RecordCommandBuffers();

	void RecordCommandBuffersInternal(int LightingMode, std::vector<VkCommandBuffer>& CmdBufs);

	void RecordFallbackCopyCommandBuffers();

	void RecordToneMappingCommandBuffers();

	void RecordSwapChainColorToPresentCommandBuffers();

	void BeginRendering(VkCommandBuffer CmdBuf, VkImageView ImageView, VkImageView DepthView);

	void UpdateGUI();

	void UpdateUniformBuffers(int ImageIndex);

	void RenderScene();

	GLFWwindow* m_pWindow = NULL;
	OgldevVK::VulkanCore m_vkCore;
	VkDescriptorPool m_descPool = VK_NULL_HANDLE;
	OgldevVK::VulkanQueue* m_pQueue = NULL;
	VkDevice m_device = NULL;
	std::vector<OgldevVK::OffscreenImage> m_offscreenImages;
	int m_numImages = 0;
	struct MeshCmdBufs {
		std::vector<VkCommandBuffer> BaseMeshDraw; // Size: m_numImages
	};
	std::vector<MeshCmdBufs> m_cmdBufs;	// dim: lighting modes
	std::vector<VkCommandBuffer> m_swapChainColorToPresentCmdBufs;
	std::vector<VkCommandBuffer> m_toneMappingCmdBufs;
	std::vector<VkCommandBuffer> m_fallbackCopyCmdBufs;
	VkShaderModule m_vs = VK_NULL_HANDLE;
	VkShaderModule m_fs = VK_NULL_HANDLE;
	OgldevVK::LightingProgram m_pipelines[OgldevVK::NUM_LIGHTING_MODES];
	OgldevVK::ToneMappingProgram m_toneMappingPipeline;
	ModelContext m_modelContext;
	std::vector<VkDescriptorSet> m_toneMappingDescSets;
	GLMCameraFirstPerson* m_pGameCamera = NULL;
	OgldevVK::ImGUIRenderer m_imGUIRenderer;
	int m_windowWidth = 0;
	int m_windowHeight = 0;
	OgldevVK::BigTextureArray m_bigTextureArray;

	// GUI state
	bool m_showGui = false;
	bool m_enableToneMapping = true;
	glm::vec3 m_position = glm::vec3(0.0f);
	glm::vec3 m_rotation = glm::vec3(0.0f);
	float m_scale = 0.1f;
	OgldevVK::LIGHTING_MODE m_lightingMode = OgldevVK::LIGHTING_MODE_FULL;
	vec3 m_lightDir = vec3(0.0f, 0.14f, 1.0f);
	float m_ambientLight = 0.1f;
	float m_diffuseLight = 1.0f;
	vec3 m_lightColor = vec3(1.0f, 1.0f, 1.0f);
	std::string m_appName;
};



