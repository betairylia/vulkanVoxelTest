#pragma once

#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <cassert>
#include <vulkan/vulkan.h>
#include <vulkan/vk_sdk_platform.h>

#include "structs.h"
#include "Pipeline.h"
#include "vHelper.h"

using namespace std;

class RenderPass
{
public:
	RenderPass();
	~RenderPass();

	void SetBase(
		VkDevice device, VkPhysicalDevice gpu,
		VkPhysicalDeviceMemoryProperties &memory_properties, VkCommandBuffer &cmdBuf,
		VkQueue &queue,
		int _width, int _height)
	{
		m_device = device;
		m_gpu = gpu;
		m_memProp = memory_properties;
		m_cmdBuf = cmdBuf;
		m_queue = queue;

		width = _width;
		height = _height;
	}

	int subPassCount;
	Pipeline* pipelines;

	VkRenderPass renderPass;
	vector<VkFramebuffer> framebuffers;

	VkDevice m_device;
	VkPhysicalDeviceMemoryProperties m_memProp;
	VkPhysicalDevice m_gpu;
	VkCommandBuffer m_cmdBuf;
	VkQueue m_queue;

	int width, height;
};

