#pragma once

#define WIN32_LEAN_AND_MEAN
#define VK_USE_PLATFORM_WIN32_KHR
#define NOMINMAX /* Don't let Windows define min() or max() */

#define GLM_FORCE_RADIANS
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <cassert>
#include <vulkan/vulkan.h>
#include <vulkan/vk_sdk_platform.h>

#include "SPIRV/GlslangToSpv.h"

#include <Windows.h>

#include "structs.h"

class vHelper
{
public:
	vHelper();
	~vHelper();

	static bool MemoryTypeFromProperties(VkPhysicalDeviceMemoryProperties memory_properties, uint32_t typeBits, VkFlags requirements_mask, uint32_t * typeIndex);
	static void SetImageLayout(VkCommandBuffer & cmdBuf, VkQueue & queue, VkImage image, VkImageAspectFlags aspectMask, VkImageLayout old_image_layout, VkImageLayout new_image_layout);

	static void createImage(int width, int height, VkFormat format, int usage, GraphicsImage *attachment, VkCommandBuffer layoutCmd, VkQueue layoutQueue, VkDevice device, VkPhysicalDeviceMemoryProperties memoryProp);
};

