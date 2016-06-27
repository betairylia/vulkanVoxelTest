#pragma once

#define WIN32_LEAN_AND_MEAN
#define VK_USE_PLATFORM_WIN32_KHR

#include <cstdlib>
#include <string>
#include <vector>
#include <cassert>

#include <vulkan/vulkan.h>

#include "structs.h"
#include "DescSet.h"

#define GLM_FORCE_RADIANS
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

using namespace std;

class Renderable
{
public:
	Renderable();
	~Renderable();

	VertexBuffer	vertexBuffer;
	IndexBuffer		indexBuffer;
	UniformBuffer	uniformBuffer;

	uint32_t indicesCount;

	void init(VkDevice _device, VkDescriptorPool pool, vector<VkDescriptorSetLayout> layout, VkPhysicalDeviceMemoryProperties memoryProp);

	void initIA(VkDevice _device, VkDescriptorPool pool, vector<VkDescriptorSetLayout> layout, VkPhysicalDeviceMemoryProperties memoryProp, VkSampler sampler, VkImageView* views, VkImageLayout imgLayout);

	void SetVertexBuffer(VkPhysicalDeviceMemoryProperties memoryProp, const void * vertexData, uint32_t dataSize, uint32_t dataStride, VkVertexInputBindingDescription & vi_binding, VkVertexInputAttributeDescription * vi_attribs);
	void SetIndexBuffer(VkPhysicalDeviceMemoryProperties memoryProp, const void * indexData, uint32_t dataSize, uint32_t dataStride);
	void SetUniformBuffer(void * content, int size, VkPhysicalDeviceMemoryProperties memoryProperties);
	void SetPosition(float x, float y, float z, VkPhysicalDeviceMemoryProperties memoryProperties);
	void SetScreenQuad(VkPhysicalDeviceMemoryProperties memoryProperties);

	void UpdateVertexBuffer(VkDevice device, const void * vertexData, uint32_t dataSize);
	void UpdateIndexBuffer(VkDevice device, const void * indexData, uint32_t dataSize);
	void UpdatePosition(float x, float y, float z);

	bool MemoryTypeFromProperties(VkPhysicalDeviceMemoryProperties memory_properties, uint32_t typeBits, VkFlags requirements_mask, uint32_t * typeIndex);

	DescSet descSet;

private:

	VkDevice device;
	uniformData uData;
};

