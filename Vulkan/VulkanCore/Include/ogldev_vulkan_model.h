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

#include "Int/core_model.h"
#include "ogldev_vulkan_texture.h"

namespace OgldevVK {

class VulkanCore;

class VkModel : public CoreModel
{
public:

	VkModel() {}

	void Init(VulkanCore* pVulkanCore) { m_pVulkanCore = pVulkanCore; }

	virtual void Render(DemolitionRenderCallbacks* pRenderCallbacks = NULL) { assert(0); }

	virtual void Render(uint DrawIndex, uint PrimID) { assert(0); }

	virtual void Render(uint NumInstances, const Matrix4f* WVPMats, const Matrix4f* WorldMats) { assert(0); }

	virtual void SetColorTexture(int TextureHandle) { assert(0); }

	virtual void SetNormalMap(int TextureHandle) { assert(0); }

	virtual void SetHeightMap(int TextureHandle) { assert(0); }

	virtual void SetTextureScale(float Scale) { assert(0); }

protected:

	virtual void AllocBuffers() { /* Nothing to do here */ }

	virtual Texture* AllocTexture2D();

	virtual void InitGeometryPost() { /* Nothing to do here */ }

	virtual void PopulateBuffersSkinned(vector<SkinnedVertex>& Vertices) { assert(0); }

	virtual void PopulateBuffers(vector<Vertex>& Vertices);

private:
	VulkanCore* m_pVulkanCore = NULL;
};

}
