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

static void PrintImageUsageFlags(const VkImageUsageFlags& flags)
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


static void PrintMemoryProperty(VkMemoryPropertyFlags PropertyFlags)
{
    if (PropertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
        printf("DEVICE LOCAL ");
    }

    if (PropertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
        printf("HOST VISIBLE ");
    }

    if (PropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
        printf("HOST COHERENT ");
    }

    if (PropertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) {
        printf("HOST CACHED ");
    }

    if (PropertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) {
        printf("LAZILY ALLOCATED ");
    }

    if (PropertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT) {
        printf("PROTECTED ");
    }
}


void VulkanPhysicalDevices::Init(const VkInstance& Instance, const VkSurfaceKHR& Surface)
{
    u32 NumDevices = 0;

    VkResult res = vkEnumeratePhysicalDevices(Instance, &NumDevices, NULL);
    CHECK_VK_RESULT(res, "vkEnumeratePhysicalDevices error (1)\n");

    printf("Num physical devices %d\n\n", NumDevices);

    m_devices.resize(NumDevices);

    std::vector<VkPhysicalDevice> Devices;
    Devices.resize(NumDevices);

    res = vkEnumeratePhysicalDevices(Instance, &NumDevices, Devices.data());
    CHECK_VK_RESULT(res, "vkEnumeratePhysicalDevices error (2)\n");

    for (u32 i = 0; i < NumDevices; i++) {
        VkPhysicalDevice PhysDev = Devices[i];
        m_devices[i].m_physDevice = PhysDev;

        vkGetPhysicalDeviceProperties(PhysDev, &m_devices[i].m_devProps);

        printf("Device name: %s\n", m_devices[i].m_devProps.deviceName);
        u32 apiVer = m_devices[i].m_devProps.apiVersion;
        printf("    API version: %d.%d.%d.%d\n", 
            VK_API_VERSION_VARIANT(apiVer), 
            VK_API_VERSION_MAJOR(apiVer), 
            VK_API_VERSION_MINOR(apiVer), 
            VK_API_VERSION_PATCH(apiVer));

        u32 NumQFamilies = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(PhysDev, &NumQFamilies, NULL);
        printf("    Num of family queues: %d\n", NumQFamilies);

        m_devices[i].m_qFamilyProps.resize(NumQFamilies);
        m_devices[i].m_qSupportsPresent.resize(NumQFamilies);

        vkGetPhysicalDeviceQueueFamilyProperties(PhysDev, &NumQFamilies, m_devices[i].m_qFamilyProps.data());

        for (u32 q = 0; q < NumQFamilies; q++) {
            const VkQueueFamilyProperties& QFamilyProp = m_devices[i].m_qFamilyProps[q];

            printf("    Family %d Num queues: %d ", q, QFamilyProp.queueCount);
            VkQueueFlags Flags = QFamilyProp.queueFlags;
            printf("    GFX %s, Compute %s, Transfer %s, Sparse binding %s\n",
                (Flags & VK_QUEUE_GRAPHICS_BIT) ? "Yes" : "No",
                (Flags & VK_QUEUE_COMPUTE_BIT) ? "Yes" : "No",
                (Flags & VK_QUEUE_TRANSFER_BIT) ? "Yes" : "No",
                (Flags & VK_QUEUE_SPARSE_BINDING_BIT) ? "Yes" : "No");

            res = vkGetPhysicalDeviceSurfaceSupportKHR(PhysDev, q, Surface, &(m_devices[i].m_qSupportsPresent[q]));
            CHECK_VK_RESULT(res, "vkGetPhysicalDeviceSurfaceSupportKHR error\n");
        }

        u32 NumFormats = 0;
        res = vkGetPhysicalDeviceSurfaceFormatsKHR(PhysDev, Surface, &NumFormats, NULL);
        CHECK_VK_RESULT(res, "vkGetPhysicalDeviceSurfaceFormatsKHR (1)\n");
        assert(NumFormats > 0);

        m_devices[i].m_surfaceFormats.resize(NumFormats);

        res = vkGetPhysicalDeviceSurfaceFormatsKHR(PhysDev, Surface, &NumFormats, m_devices[i].m_surfaceFormats.data());
        CHECK_VK_RESULT(res, "vkGetPhysicalDeviceSurfaceFormatsKHR (2)\n");

        for (u32 j = 0; j < NumFormats; j++) {
            const VkSurfaceFormatKHR& SurfaceFormat = m_devices[i].m_surfaceFormats[j];
            printf("    Format %x color space %x\n", SurfaceFormat.format, SurfaceFormat.colorSpace);
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
            printf("%d: flags %x heap %d ", j, 
                m_devices[i].m_memProps.memoryTypes[j].propertyFlags,
                m_devices[i].m_memProps.memoryTypes[j].heapIndex);

            PrintMemoryProperty(m_devices[i].m_memProps.memoryTypes[j].propertyFlags);

            printf("\n");
        }
        printf("Num heap types %d\n", m_devices[i].m_memProps.memoryHeapCount);
        printf("\n");
    }

    exit(0);
}


u32 VulkanPhysicalDevices::SelectDevice(VkQueueFlags RequiredQueueType, bool SupportsPresent)
{
    for (u32 i = 0; i < m_devices.size(); i++) {

        for (u32 j = 0; j < m_devices[i].m_qFamilyProps.size(); j++) {
            const VkQueueFamilyProperties& QFamilyProp = m_devices[i].m_qFamilyProps[j];

            if ((QFamilyProp.queueFlags & RequiredQueueType) && ((bool)m_devices[i].m_qSupportsPresent[j] == SupportsPresent)) {
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