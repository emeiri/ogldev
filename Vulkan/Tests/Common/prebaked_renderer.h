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
#include "ogldev_vulkan_model.h"
#include "ogldev_glm_camera.h"
#include "lighting_program.h"
#include "shadow_program.h"


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


class GameCallbacks {

public:

	virtual void UpdateGameState(float DeltaTime) {}

    virtual void OnKey(int Key, int Scancode, int Action, int Mods) {}

    virtual void OnMouseMove(double x, double y) {}

    virtual void OnMouseButton(int Button, int Action, int Mods) {}

};

class PreBakedRenderer : public OgldevVK::GLFWCallbacks
{
public:

	PreBakedRenderer(int WindowWidth, int WindowHeight, const std::string& AppName, GameCallbacks* pGameCallbacks);

	~PreBakedRenderer();

	void Init(const std::string& AssetPath);

	virtual void Key(GLFWwindow* pWindow, int Key, int Scancode, int Action, int Mods);

	virtual void MouseMove(GLFWwindow* pWindow, double x, double y);

	virtual void MouseButton(GLFWwindow* pWindow, int Button, int Action, int Mods);

	void Execute();

    OgldevVK::VkModel& GetModel() { return *m_modelContext.m_pModel; }

private:

    void InitDefaultCamera();

	void InitCameraFromModel();

	void DefaultCreateCameraPers();

	void DefaultCreateCameraPers(float FOV, float zNear, float zFar);

	void CreateCommandBuffers();

	void CreateDescriptorPool();

	void InitBigTextureArray();

	void CreateOffscreenImages();

    void CreateShadowMaps();

	void CreateMesh(const std::string& AssetPath);

    void LoadLightsFromModel();

	void LoadPBRTextures();

	void CreateShaders();

	void CreatePipelines();

	void CreateUniformBuffers(int MeshIndex);

	void CreateDescriptorSets(int MeshIndex, OgldevVK::ModelDesc& md);

	void RecordCommandBuffers();

	void RecordShadowPassCommandBuffers();

	void RecordCommandBuffersInternal(int LightingMode, bool IsPBR, std::vector<VkCommandBuffer>& CmdBufs);

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
	std::vector< std::vector<OgldevVK::VulkanTexture > > m_shadowMaps;   
	std::vector<OgldevVK::BufferAndMemory> m_uniformBuffersLightTransforms;
	int m_numImages = 0;
	struct MeshCmdBufs {
		std::vector<VkCommandBuffer> m_cbs; // Size: m_numImages
	};
	std::vector<MeshCmdBufs> m_lightPassCmdBufs;	// dim: lighting modes
	std::vector<MeshCmdBufs> m_pbrLightPassCmdBufs;	// dim: lighting modes
    std::vector< std::vector<VkCommandBuffer> > m_shadowPassCmdBufs; // outer dim: m_numImages, inner dim: MAX_NUM_LIGHTS
	std::vector<VkCommandBuffer> m_swapChainColorToPresentCmdBufs;
	std::vector<VkCommandBuffer> m_fallbackCopyCmdBufs;
    VkCommandBuffer m_utilCmdBuf = VK_NULL_HANDLE;
	VkShaderModule m_vs = VK_NULL_HANDLE;
	VkShaderModule m_fs = VK_NULL_HANDLE;
	VkShaderModule m_pbrFs = VK_NULL_HANDLE;
	OgldevVK::LightingProgram m_pipelines[OgldevVK::NUM_LIGHTING_MODES];
	OgldevVK::LightingProgram m_pbrPipelines[OgldevVK::NUM_LIGHTING_MODES];
    OgldevVK::ShadowProgram m_shadowProgram;
	ModelContext m_modelContext;
	GLMCameraFirstPerson* m_pGameCamera = NULL;
    GLMCameraFirstPerson m_defaultCameraPers;
	OgldevVK::ImGUIRenderer m_imGUIRenderer;
	int m_windowWidth = 0;
	int m_windowHeight = 0;
	OgldevVK::BigTextureArray m_bigTextureArray;
	OgldevVK::VulkanTexture m_brdfLUT;
	OgldevVK::VulkanTexture m_irradianceMap;
	OgldevVK::VulkanTexture m_envMap;        // prefiltered specular

	// GUI state
	bool m_showGui = false;
    bool m_isPBR = true;
	OgldevVK::LIGHTING_MODE m_lightingMode = OgldevVK::LIGHTING_MODE_FULL;
	std::string m_appName;
	GameCallbacks* m_pGameCallbacks = NULL;
    float m_exposure = 0.6f;
    float m_iblIntensity = 1.0f;
    std::vector<OgldevVK::LightSource> m_lights;
};
