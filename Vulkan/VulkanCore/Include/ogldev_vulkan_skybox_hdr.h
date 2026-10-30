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

#pragma once

#include <glm/glm.hpp>

#include "ogldev_vulkan_core.h"
#include "ogldev_vulkan_texture.h"
#include "ogldev_vulkan_pipeline_program.h"

namespace OgldevVK {

class SkyboxHDR {

public:
    void Init(VulkanCore& vkCore, VkDescriptorPool DescPool,
        const VulkanTexture& Cubemap, VkFormat ColorFormat);

    void Destroy();
    
    void Update(int ImageIndex, const glm::mat4& VPNoTranslate, float Exposure);
    
    void RecordCommandBuffer(VkCommandBuffer CmdBuf, int ImageIndex);

private:

    class SkyboxProgram : public GraphicsPipeline {

    protected:
        std::vector<VkDescriptorSetLayout> CreateDescSetLayout(VulkanCore& vkCore) override;
        VkCompareOp GetDepthCompareOp() const override { return VK_COMPARE_OP_LESS_OR_EQUAL; }
    };

    struct UniformData {
        glm::mat4 VP;
        glm::vec4 Config;     // .x = exposure
    };

    void CreateDescriptorSets(const VulkanTexture& Cubemap);

    VulkanCore* m_pVulkanCore = NULL;
    SkyboxProgram m_program;
    std::vector<BufferAndMemory> m_uniformBuffers;
    std::vector<VkDescriptorSet> m_descSets;
    VkShaderModule m_vs = VK_NULL_HANDLE;
    VkShaderModule m_fs = VK_NULL_HANDLE;
};
}
