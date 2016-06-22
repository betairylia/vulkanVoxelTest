#pragma once

#include <vulkan/vulkan.h>

typedef struct _Vertex {
	float posX, posY, posZ, posW; // Position data
	float r, g, b, a;             // Color
} Vertex;

typedef struct _swap_chain_buffers {
	VkImage image;
	VkImageView view;
} SwapChainBuffer;

typedef struct _depth {
	VkFormat format;

	VkImage image;
	VkImageView view;
	VkDeviceMemory memory;
} DepthMap;

typedef struct _uniform {
	VkBuffer buf;
	VkDeviceMemory mem;
	VkDescriptorBufferInfo buffer_info;
	VkMemoryRequirements mem_reqs;
} UniformBuffer;

typedef struct _vb {
	VkBuffer buf;
	VkDeviceMemory mem;
	VkDescriptorBufferInfo buffer_info;
	VkMemoryRequirements mem_reqs;
} VertexBuffer;

typedef struct _ib {
	VkBuffer buf;
	VkDeviceMemory mem;
	VkDescriptorBufferInfo buffer_info;
	VkMemoryRequirements mem_reqs;
} IndexBuffer;