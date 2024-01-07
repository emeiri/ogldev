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

#include <assert.h>

#include "vulkan_utils.h"
#include "vk_device.h"

namespace OgldevVK {

void PrintImageUsageFlags(const VkImageUsageFlags& flags)
{
    if (flags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
        printf("Image usage transfer src is supported\n");
    }

    if (flags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
        printf("Image usage transfer dest is supported\n");
    }

    if (flags & VK_IMAGE_USAGE_SAMPLED_BIT) {
        printf("Image usage sampled is supported\n");
    }

    if (flags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
        printf("Image usage color attachment is supported\n");
    }

    if (flags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        printf("Image usage depth stencil attachment is supported\n");
    }

    if (flags & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) {
        printf("Image usage transient attachment is supported\n");
    }

    if (flags & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) {
        printf("Image usage input attachment is supported\n");
    }
}


uint GetPhysicalDevicesCount(const VkInstance& inst)
{
    uint NumDevices = 0;

    VkResult res = vkEnumeratePhysicalDevices(inst, &NumDevices, NULL);
    CHECK_VK_RESULT(res, "vkEnumeratePhysicalDevices error\n");

    return NumDevices;
}


void VulkanPhysicalDevices::Allocate(int NumDevices)
{
    m_devices.resize(NumDevices);
    m_devProps.resize(NumDevices);
    m_qFamilyProps.resize(NumDevices);
    m_qSupportsPresent.resize(NumDevices);
    m_surfaceFormats.resize(NumDevices);
    m_surfaceCaps.resize(NumDevices);
    m_memProps.resize(NumDevices);
    m_presentModes.resize(NumDevices);
}


void VulkanPhysicalDevices::Init(const VkInstance& inst, const VkSurfaceKHR& Surface)
{
    uint NumDevices = GetPhysicalDevicesCount(inst);
    printf("Num physical devices %d\n", NumDevices);

    Allocate(NumDevices);

    VkResult res = vkEnumeratePhysicalDevices(inst, &NumDevices, &m_devices[0]);
    CHECK_VK_RESULT(res, "vkEnumeratePhysicalDevices error\n");

    for (uint i = 0; i < NumDevices; i++) {
        const VkPhysicalDevice& PhysDev = m_devices[i];
        vkGetPhysicalDeviceProperties(PhysDev, &m_devProps[i]);

        printf("Device name: %s\n", m_devProps[i].deviceName);
        uint32_t apiVer = m_devProps[i].apiVersion;
        printf("    API version: %d.%d.%d\n", VK_VERSION_MAJOR(apiVer), VK_VERSION_MINOR(apiVer), VK_VERSION_PATCH(apiVer));
        uint NumQFamily = 0;

        vkGetPhysicalDeviceQueueFamilyProperties(PhysDev, &NumQFamily, NULL);

        printf("    Num of family queues: %d\n", NumQFamily);

        m_qFamilyProps[i].resize(NumQFamily);
        m_qSupportsPresent[i].resize(NumQFamily);

        vkGetPhysicalDeviceQueueFamilyProperties(PhysDev, &NumQFamily, &(m_qFamilyProps[i][0]));

        for (uint q = 0; q < NumQFamily; q++) {
            res = vkGetPhysicalDeviceSurfaceSupportKHR(PhysDev, q, Surface, &(m_qSupportsPresent[i][q]));
            CHECK_VK_RESULT(res, "vkGetPhysicalDeviceSurfaceSupportKHR error\n");
        }

        uint NumFormats = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(PhysDev, Surface, &NumFormats, NULL);
        assert(NumFormats > 0);

        m_surfaceFormats[i].resize(NumFormats);

        res = vkGetPhysicalDeviceSurfaceFormatsKHR(PhysDev, Surface, &NumFormats, &(m_surfaceFormats[i][0]));
        CHECK_VK_RESULT(res, "vkGetPhysicalDeviceSurfaceFormatsKHR\n");

        for (uint j = 0; j < NumFormats; j++) {
            const VkSurfaceFormatKHR& SurfaceFormat = m_surfaceFormats[i][j];
            printf("    Format %d color space %d\n", SurfaceFormat.format, SurfaceFormat.colorSpace);
        }

        res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysDev, Surface, &(m_surfaceCaps[i]));
        CHECK_VK_RESULT(res, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR\n");

        PrintImageUsageFlags(m_surfaceCaps[i].supportedUsageFlags);

        uint NumPresentModes = 0;

        res = vkGetPhysicalDeviceSurfacePresentModesKHR(PhysDev, Surface, &NumPresentModes, NULL);
        CHECK_VK_RESULT(res, "vkGetPhysicalDeviceSurfacePresentModesKHR (1) error\n");

        assert(NumPresentModes != 0);

        m_presentModes[i].resize(NumPresentModes);

        res = vkGetPhysicalDeviceSurfacePresentModesKHR(PhysDev, Surface, &NumPresentModes, m_presentModes[i].data());
        CHECK_VK_RESULT(res, "vkGetPhysicalDeviceSurfacePresentModesKHR (2) error\n");

        printf("Number of presentation modes %d\n", NumPresentModes);

        vkGetPhysicalDeviceMemoryProperties(PhysDev, &m_memProps[i]);

        printf("Num memory types %d\n", m_memProps[i].memoryTypeCount);
        for (uint j = 0; j < m_memProps[i].memoryTypeCount; j++) {
            printf("%d: (%x) ", j, m_memProps[i].memoryTypes[j].propertyFlags);

            if (m_memProps[i].memoryTypes[j].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
                printf("DEVICE LOCAL ");
            }

            if (m_memProps[i].memoryTypes[j].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
                printf("HOST VISIBLE ");
            }

            if (m_memProps[i].memoryTypes[j].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
                printf("HOST COHERENT ");
            }

            if (m_memProps[i].memoryTypes[j].propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) {
                printf("HOST CACHED ");
            }

            if (m_memProps[i].memoryTypes[j].propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) {
                printf("LAZILY ALLOCATED ");
            }

            if (m_memProps[i].memoryTypes[j].propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT) {
                printf("PROTECTED ");
            }

            printf("\n");
        }
        printf("Num heap types %d\n", m_memProps[i].memoryHeapCount);
    }
}


DeviceAndQueue VulkanPhysicalDevices::SelectDevice(VkQueueFlags RequiredQueueType, bool SupportsPresent)
{
    DeviceAndQueue ret;

    for (uint i = 0; i < m_devices.size(); i++) {

        for (uint j = 0; j < m_qFamilyProps[i].size(); j++) {
            VkQueueFamilyProperties& QFamilyProp = m_qFamilyProps[i][j];

            printf("Family %d Num queues: %d\n", j, QFamilyProp.queueCount);
            VkQueueFlags flags = QFamilyProp.queueFlags;
            printf("    GFX %s, Compute %s, Transfer %s, Sparse binding %s\n",
                (flags & VK_QUEUE_GRAPHICS_BIT) ? "Yes" : "No",
                (flags & VK_QUEUE_COMPUTE_BIT) ? "Yes" : "No",
                (flags & VK_QUEUE_TRANSFER_BIT) ? "Yes" : "No",
                (flags & VK_QUEUE_SPARSE_BINDING_BIT) ? "Yes" : "No");

            if ((flags & RequiredQueueType) && ((bool)m_qSupportsPresent[i][j] == SupportsPresent)) {
                ret.Device = i;
                ret.Queue = j;
                printf("Using GFX device %d and queue family %d\n", i, j);
                return ret;
            }
        }
    }

    fprintf(stderr, "Required queue type %x and supports present %d not found\n", RequiredQueueType, SupportsPresent);
    exit(1);
}

}