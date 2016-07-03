#pragma once

#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <cassert>
#include <vulkan/vulkan.h>
#include <vulkan/vk_sdk_platform.h>

#include "structs.h"
#include "Renderable.h"
#include "DescSet.h"
#include "Pipeline.h"
#include "DescPipelineLayout.h"
#include "vHelper.h"

using namespace std;

class RenderUnit
{
public:
	RenderUnit();
	~RenderUnit();

	void init(
		bool isPostEffect,
		VkDevice device, VkPhysicalDevice gpu,
		VkPhysicalDeviceMemoryProperties &memory_properties, VkCommandBuffer &cmdBuf,
		VkQueue &queue,
		int _width, int _height,
		std::vector<GraphicsImage> inputImageList,
		std::vector<GraphicsImage> outputImageList,
		VkDescriptorPool descPool, VkSampler sampler,
		VkVertexInputBindingDescription & vi_binding, VkVertexInputAttributeDescription * vi_attribs,
		const char * vertShaderText, const char * fragShaderText,
		bool useDepth = false);//Supports 1 subpass, vs & fs only; 1 uniform buffer for vs now. descSet: 0-model 1-frame 2-pass 3-empty 4-application

	void initAsLastUnit(
		bool isPostEffect,
		VkDevice device, VkPhysicalDevice gpu,
		VkPhysicalDeviceMemoryProperties &memory_properties, VkCommandBuffer &cmdBuf,
		VkQueue &queue,
		int _width, int _height,
		std::vector<GraphicsImage> inputImageList,
		SwapChainBuffer *buffers, int swapchainImageCount, VkFormat imgFormat,
		VkDescriptorPool descPool, VkSampler sampler,
		VkVertexInputBindingDescription & vi_binding, VkVertexInputAttributeDescription * vi_attribs,
		const char * vertShaderText, const char * fragShaderText,
		bool useDepth = false);

	bool drawScreenQuad;
	bool asLast = false;

	bool hasDepth = false;
	int iCount, oCount, clearCount;

	VkRenderPass renderPass;
	vector<VkFramebuffer> framebuffers;

	VkDevice m_device;
	VkPhysicalDeviceMemoryProperties m_memProp;
	VkPhysicalDevice m_gpu;
	VkCommandBuffer m_cmdBuf;
	VkQueue m_queue;

	DescSet descSet;
	Pipeline pipeline;
	DescPipelineLayout layout;

	GraphicsImage depth;

	VkClearValue clearValues[10];

	int width, height;
};

