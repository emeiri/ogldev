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

#include <vector>

#include <vulkan/vulkan.h>

namespace OgldevVK {

struct RangeDesc {
	VkDeviceSize m_offset = 0;
	VkDeviceSize m_range = 0;
};

struct SubmeshRanges {
	RangeDesc m_vbRange;
	RangeDesc m_ibRange;
	RangeDesc m_uniformRange;
};

struct TextureInfo {
	VkSampler m_sampler;
	VkImageView m_imageView;
};

struct ModelDesc {
	VkBuffer m_vb;
	VkBuffer m_ib;
	std::vector<VkBuffer> m_uniforms;
	std::vector<TextureInfo> m_materials;
	std::vector<SubmeshRanges> m_ranges;
};

};