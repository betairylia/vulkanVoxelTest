#pragma once

#define GLM_FORCE_RADIANS
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include <vulkan/vulkan.h>

#define X_POSITIVE 0x0001
#define Y_POSITIVE 0x0002
#define Z_POSITIVE 0x0004
#define X_NEGATIVE 0x0006
#define Y_NEGATIVE 0x0005
#define Z_NEGATIVE 0x0003

typedef struct _Vertex {
	float posX, posY, posZ; // Position data
	float u, v;             // uv
	float norX, norY, norZ; // Normal
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

typedef struct _uniformData {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
}uniformData;

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

typedef struct _GraphicsImage {
	VkImage image;
	VkDeviceMemory mem;
	VkImageView view;
	VkFormat format;
} GraphicsImage;