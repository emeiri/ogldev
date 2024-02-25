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

#include "ogldev_util.h"
#include "ogldev_vulkan_util.h"
#include "ogldev_vulkan_device.h"

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


u32 GetPhysicalDevicesCount(const VkInstance& inst)
{
    u32 NumDevices = 0;

    VkResult res = vkEnumeratePhysicalDevices(inst, &NumDevices, NULL);
    CHECK_VK_RESULT(res, "vkEnumeratePhysicalDevices error\n");

    return NumDevices;
}


void VulkanPhysicalDevices::Init(const VkInstance& inst, const VkSurfaceKHR& Surface)
{
    u32 NumDevices = GetPhysicalDevicesCount(inst);
    printf("Num physical devices %d\n\n", NumDevices);

    m_devices.resize(NumDevices);

    std::vector<VkPhysicalDevice> Devices;
    Devices.resize(NumDevices);

    VkResult res = vkEnumeratePhysicalDevices(inst, &NumDevices, Devices.data());
    CHECK_VK_RESULT(res, "vkEnumeratePhysicalDevices error\n");

    for (u32 i = 0; i < NumDevices; i++) {
        const VkPhysicalDevice& PhysDev = Devices[i];
        m_devices[i].m_physDevice = PhysDev;

        vkGetPhysicalDeviceProperties(PhysDev, &m_devices[i].m_devProps);

        printf("Device name: %s\n", m_devices[i].m_devProps.deviceName);
        u32 apiVer = m_devices[i].m_devProps.apiVersion;
        printf("    API version: %d.%d.%d\n", VK_VERSION_MAJOR(apiVer), VK_VERSION_MINOR(apiVer), VK_VERSION_PATCH(apiVer));

        u32 NumQFamily = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(PhysDev, &NumQFamily, NULL);
        printf("    Num of family queues: %d\n", NumQFamily);

        m_devices[i].m_qFamilyProps.resize(NumQFamily);
        m_devices[i].m_qSupportsPresent.resize(NumQFamily);

        vkGetPhysicalDeviceQueueFamilyProperties(PhysDev, &NumQFamily, m_devices[i].m_qFamilyProps.data());

        for (u32 q = 0; q < NumQFamily; q++) {
            res = vkGetPhysicalDeviceSurfaceSupportKHR(PhysDev, q, Surface, &(m_devices[i].m_qSupportsPresent[q]));
            CHECK_VK_RESULT(res, "vkGetPhysicalDeviceSurfaceSupportKHR error\n");
        }

        u32 NumFormats = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(PhysDev, Surface, &NumFormats, NULL);
        assert(NumFormats > 0);

        m_devices[i].m_surfaceFormats.resize(NumFormats);

        res = vkGetPhysicalDeviceSurfaceFormatsKHR(PhysDev, Surface, &NumFormats, m_devices[i].m_surfaceFormats.data());
        CHECK_VK_RESULT(res, "vkGetPhysicalDeviceSurfaceFormatsKHR\n");

        for (u32 j = 0; j < NumFormats; j++) {
            const VkSurfaceFormatKHR& SurfaceFormat = m_devices[i].m_surfaceFormats[j];
            printf("    Format %d color space %d\n", SurfaceFormat.format, SurfaceFormat.colorSpace);
        }

        res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysDev, Surface, &(m_devices[i].m_surfaceCaps));
        CHECK_VK_RESULT(res, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR\n");

        PrintImageUsageFlags(m_devices[i].m_surfaceCaps.supportedUsageFlags);

        u32 NumPresentModes = 0;

        res = vkGetPhysicalDeviceSurfacePresentModesKHR(PhysDev, Surface, &NumPresentModes, NULL);
        CHECK_VK_RESULT(res, "vkGetPhysicalDeviceSurfacePresentModesKHR (1) error\n");

        assert(NumPresentModes != 0);

        m_devices[i].m_presentModes.resize(NumPresentModes);

        res = vkGetPhysicalDeviceSurfacePresentModesKHR(PhysDev, Surface, &NumPresentModes, m_devices[i].m_presentModes.data());
        CHECK_VK_RESULT(res, "vkGetPhysicalDeviceSurfacePresentModesKHR (2) error\n");

        printf("Number of presentation modes %d\n", NumPresentModes);

        vkGetPhysicalDeviceMemoryProperties(PhysDev, &(m_devices[i].m_memProps));

        printf("Num memory types %d\n", m_devices[i].m_memProps.memoryTypeCount);
        for (u32 j = 0; j < m_devices[i].m_memProps.memoryTypeCount; j++) {
            printf("%d: (%x) ", j, m_devices[i].m_memProps.memoryTypes[j].propertyFlags);

            if (m_devices[i].m_memProps.memoryTypes[j].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
                printf("DEVICE LOCAL ");
            }

            if (m_devices[i].m_memProps.memoryTypes[j].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
                printf("HOST VISIBLE ");
            }

            if (m_devices[i].m_memProps.memoryTypes[j].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
                printf("HOST COHERENT ");
            }

            if (m_devices[i].m_memProps.memoryTypes[j].propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) {
                printf("HOST CACHED ");
            }

            if (m_devices[i].m_memProps.memoryTypes[j].propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) {
                printf("LAZILY ALLOCATED ");
            }

            if (m_devices[i].m_memProps.memoryTypes[j].propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT) {
                printf("PROTECTED ");
            }

            printf("\n");
        }
        printf("Num heap types %d\n", m_devices[i].m_memProps.memoryHeapCount);
        printf("\n");
    }
}


u32 VulkanPhysicalDevices::SelectDevice(VkQueueFlags RequiredQueueType, bool SupportsPresent)
{
    for (u32 i = 0; i < m_devices.size(); i++) {

        for (u32 j = 0; j < m_devices[i].m_qFamilyProps.size(); j++) {
            VkQueueFamilyProperties& QFamilyProp = m_devices[i].m_qFamilyProps[j];

            printf("Family %d Num queues: %d\n", j, QFamilyProp.queueCount);
            VkQueueFlags flags = QFamilyProp.queueFlags;
            printf("    GFX %s, Compute %s, Transfer %s, Sparse binding %s\n",
                (flags & VK_QUEUE_GRAPHICS_BIT) ? "Yes" : "No",
                (flags & VK_QUEUE_COMPUTE_BIT) ? "Yes" : "No",
                (flags & VK_QUEUE_TRANSFER_BIT) ? "Yes" : "No",
                (flags & VK_QUEUE_SPARSE_BINDING_BIT) ? "Yes" : "No");

            if ((flags & RequiredQueueType) && ((bool)m_devices[i].m_qSupportsPresent[j] == SupportsPresent)) {
                m_devIndex = i;
                int QueueFamily = j;
                printf("Using GFX device %d and queue family %d\n", m_devIndex, QueueFamily);
                return QueueFamily;
            }
        }
    }
    
    OGLDEV_ERROR("Required queue type %x and supports present %d not found\n", RequiredQueueType, SupportsPresent);
    
    return 0;
}


const PhysicalDevice& VulkanPhysicalDevices::Selected() const
{
    if (m_devIndex < 0) {
        OGLDEV_ERROR("A physical device has not been selected\n");
    }

    return m_devices[m_devIndex];
}
}