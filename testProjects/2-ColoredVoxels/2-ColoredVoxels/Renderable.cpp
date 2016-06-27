#include "Renderable.h"



Renderable::Renderable()
{
}


Renderable::~Renderable()
{
}

void Renderable::init(VkDevice _device, VkDescriptorPool pool, vector<VkDescriptorSetLayout> layout, VkPhysicalDeviceMemoryProperties memoryProp)
{
	device = _device;

	SetPosition(0, 0, 0, memoryProp);

	descSet.m_device = device;
	descSet.setDescPool(pool);
	descSet.CreateDescriptorSet(layout, uniformBuffer);
}

void Renderable::initIA(VkDevice _device, VkDescriptorPool pool, vector<VkDescriptorSetLayout> layout, VkPhysicalDeviceMemoryProperties memoryProp, VkSampler sampler, VkImageView* views, VkImageLayout imgLayout)
{
	device = _device;

	SetPosition(0, 0, 0, memoryProp);

	descSet.m_device = device;
	descSet.setDescPool(pool);
	descSet.CreateDescriptorSetIA(layout, uniformBuffer, sampler, views, imgLayout);
}

void Renderable::SetVertexBuffer(VkPhysicalDeviceMemoryProperties memoryProp, const void * vertexData, uint32_t dataSize, uint32_t dataStride, VkVertexInputBindingDescription & vi_binding, VkVertexInputAttributeDescription * vi_attribs)
{
	VkResult res;
	bool pass;

	VkBufferCreateInfo buf_info = {};
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buf_info.pNext = NULL;
	buf_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	buf_info.size = dataSize;
	buf_info.queueFamilyIndexCount = 0;
	buf_info.pQueueFamilyIndices = NULL;
	buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	buf_info.flags = 0;
	res = vkCreateBuffer(device, &buf_info, NULL, &vertexBuffer.buf);
	assert(res == VK_SUCCESS);

	VkMemoryRequirements mem_reqs;
	vkGetBufferMemoryRequirements(device, vertexBuffer.buf,
		&mem_reqs);

	vertexBuffer.mem_reqs = mem_reqs;

	VkMemoryAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.pNext = NULL;
	alloc_info.memoryTypeIndex = 0;

	alloc_info.allocationSize = mem_reqs.size;
	pass = MemoryTypeFromProperties(memoryProp, mem_reqs.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&alloc_info.memoryTypeIndex);
	assert(pass && "No mappable, coherent memory");

	res = vkAllocateMemory(device, &alloc_info, NULL,
		&(vertexBuffer.mem));
	assert(res == VK_SUCCESS);
	vertexBuffer.buffer_info.range = mem_reqs.size;
	vertexBuffer.buffer_info.offset = 0;

	uint8_t *pData;
	res = vkMapMemory(device, vertexBuffer.mem, 0, mem_reqs.size, 0,
		(void **)&pData);
	assert(res == VK_SUCCESS);

	memcpy(pData, vertexData, dataSize);

	vkUnmapMemory(device, vertexBuffer.mem);

	res = vkBindBufferMemory(device, vertexBuffer.buf,
		vertexBuffer.mem, 0);
	assert(res == VK_SUCCESS);

	vi_binding.binding = 0;
	vi_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; 
	vi_binding.stride = dataStride;

	//position
	vi_attribs[0].binding = 0;
	vi_attribs[0].location = 0;
	vi_attribs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	vi_attribs[0].offset = 0;
	//uv
	vi_attribs[1].binding = 0;
	vi_attribs[1].location = 1;
	vi_attribs[1].format = VK_FORMAT_R32G32_SFLOAT;
	vi_attribs[1].offset = 3 * sizeof(float);
	//normal
	vi_attribs[2].binding = 0;
	vi_attribs[2].location = 2;
	vi_attribs[2].format = VK_FORMAT_R32G32B32_SFLOAT;
	vi_attribs[2].offset = 5 * sizeof(float);
	//uv2, ..., tangent, blockID etc.
}

void Renderable::SetIndexBuffer(VkPhysicalDeviceMemoryProperties memoryProp, const void * vertexData, uint32_t dataSize, uint32_t dataStride)
{
	VkResult res;
	bool pass;

	VkBufferCreateInfo buf_info = {};
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buf_info.pNext = NULL;
	buf_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	buf_info.size = dataSize;
	buf_info.queueFamilyIndexCount = 0;
	buf_info.pQueueFamilyIndices = NULL;
	buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	buf_info.flags = 0;
	res = vkCreateBuffer(device, &buf_info, NULL, &indexBuffer.buf);
	assert(res == VK_SUCCESS);

	VkMemoryRequirements mem_reqs;
	vkGetBufferMemoryRequirements(device, indexBuffer.buf,
		&mem_reqs);

	indexBuffer.mem_reqs = mem_reqs;

	VkMemoryAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.pNext = NULL;
	alloc_info.memoryTypeIndex = 0;

	alloc_info.allocationSize = mem_reqs.size;
	pass = MemoryTypeFromProperties(memoryProp, mem_reqs.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&alloc_info.memoryTypeIndex);
	assert(pass && "No mappable, coherent memory");

	res = vkAllocateMemory(device, &alloc_info, NULL,
		&(indexBuffer.mem));
	assert(res == VK_SUCCESS);
	indexBuffer.buffer_info.range = mem_reqs.size;
	indexBuffer.buffer_info.offset = 0;

	uint8_t *pData;
	res = vkMapMemory(device, indexBuffer.mem, 0, mem_reqs.size, 0,
		(void **)&pData);
	assert(res == VK_SUCCESS);

	memcpy(pData, vertexData, dataSize);

	vkUnmapMemory(device, indexBuffer.mem);

	res = vkBindBufferMemory(device, indexBuffer.buf,
		indexBuffer.mem, 0);
	assert(res == VK_SUCCESS);
}

void Renderable::SetUniformBuffer(void * content, int size, VkPhysicalDeviceMemoryProperties memoryProperties)
{
	VkBufferCreateInfo buf_info = {};
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buf_info.pNext = NULL;
	buf_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	buf_info.size = size;
	buf_info.queueFamilyIndexCount = 0;
	buf_info.pQueueFamilyIndices = NULL;
	buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	buf_info.flags = 0;
	VkResult res = vkCreateBuffer(device, &buf_info, NULL, &uniformBuffer.buf);
	assert(res == VK_SUCCESS);

	VkMemoryRequirements mem_reqs;
	vkGetBufferMemoryRequirements(device, uniformBuffer.buf,
		&mem_reqs);

	uniformBuffer.mem_reqs = mem_reqs;

	VkMemoryAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.pNext = NULL;
	alloc_info.memoryTypeIndex = 0;

	alloc_info.allocationSize = mem_reqs.size;
	bool pass = MemoryTypeFromProperties(memoryProperties, mem_reqs.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&alloc_info.memoryTypeIndex);
	assert(pass && "No mappable, coherent memory");

	res = vkAllocateMemory(device, &alloc_info, NULL,
		&(uniformBuffer.mem));
	assert(res == VK_SUCCESS);

	uint8_t *pData;
	res = vkMapMemory(device, uniformBuffer.mem, 0, mem_reqs.size, 0,
		(void **)&pData);
	assert(res == VK_SUCCESS);

	memcpy(pData, content, size);

	vkUnmapMemory(device, uniformBuffer.mem);

	res = vkBindBufferMemory(device, uniformBuffer.buf,
		uniformBuffer.mem, 0);
	assert(res == VK_SUCCESS);

	uniformBuffer.buffer_info.buffer = uniformBuffer.buf;
	uniformBuffer.buffer_info.offset = 0;
	uniformBuffer.buffer_info.range = size;
}

void Renderable::SetPosition(float x, float y, float z, VkPhysicalDeviceMemoryProperties memoryProperties)
{
	//Create UniformBuffer
	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 1.6f, 0.1f, 100.0f);
	glm::mat4 View = glm::lookAt(
		glm::vec3(0, 3, 10), // Camera is at (2.5,3,10), in World Space
		glm::vec3(0, 0, -10),  // and looks at the origin
		glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
		);
	glm::mat4 Model = glm::mat4(1.0f);
	Model = glm::translate(Model, glm::vec3(x, y, z));
	// Vulkan clip space has inverted Y and half Z.
	glm::mat4 Clip = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f,
		0.0f, 0.0f, 0.5f, 1.0f);

	//glm::mat4 MVP = Clip * Projection * View * Model;
	uData.model = Model;
	uData.view = View;
	uData.proj = Clip * Projection;

	SetUniformBuffer(&uData, sizeof(uData), memoryProperties);
}

void Renderable::SetScreenQuad(VkPhysicalDeviceMemoryProperties memoryProperties)
{
	//Create UniformBuffer
	glm::mat4 Projection = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);
	glm::mat4 Model = glm::mat4();

	glm::mat4 MVP = Projection * Model;

	SetUniformBuffer(&MVP, sizeof(MVP), memoryProperties);
}

void Renderable::UpdateVertexBuffer(VkDevice device, const void * vertexData, uint32_t dataSize)
{
	uint8_t *pData;

	VkResult res = vkMapMemory(device, vertexBuffer.mem, 0, vertexBuffer.mem_reqs.size, 0,
		(void **)&pData);
	assert(res == VK_SUCCESS);

	memcpy(pData, vertexData, dataSize);

	vkUnmapMemory(device, vertexBuffer.mem);
}

void Renderable::UpdateIndexBuffer(VkDevice device, const void * indexData, uint32_t dataSize)
{
	uint8_t *pData;

	VkResult res = vkMapMemory(device, indexBuffer.mem, 0, indexBuffer.mem_reqs.size, 0,
		(void **)&pData);
	assert(res == VK_SUCCESS);

	memcpy(pData, indexData, dataSize);

	vkUnmapMemory(device, indexBuffer.mem);
}

void Renderable::UpdatePosition(float x, float y, float z)
{
	//Create UniformBuffer
	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 1.6f, 0.1f, 100.0f);
	glm::mat4 View = glm::lookAt(
		glm::vec3(0, 3, 10), // Camera is at (2.5,3,10), in World Space
		glm::vec3(0, 0, -10),  // and looks at the origin
		glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
		);
	glm::mat4 Model = glm::mat4(1.0f);
	Model = glm::translate(Model, glm::vec3(x, y, z));
	// Vulkan clip space has inverted Y and half Z.
	glm::mat4 Clip = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f,
		0.0f, 0.0f, 0.5f, 1.0f);

	//glm::mat4 MVP = Clip * Projection * View * Model;
	uData.model = Model;
	uData.view = View;
	uData.proj = Clip * Projection;

	uint8_t *pData;

	VkResult res = vkMapMemory(device, uniformBuffer.mem, 0, uniformBuffer.mem_reqs.size, 0,
		(void **)&pData);
	assert(res == VK_SUCCESS);

	memcpy(pData, (const void*)&uData, sizeof(uData));

	vkUnmapMemory(device, uniformBuffer.mem);
}

bool Renderable::MemoryTypeFromProperties(VkPhysicalDeviceMemoryProperties memory_properties, uint32_t typeBits, VkFlags requirements_mask, uint32_t * typeIndex)
{
	// Search memtypes to find first index with those properties
	for (uint32_t i = 0; i < 32; i++) {
		if ((typeBits & 1) == 1) {
			// Type is available, does it match user properties?
			if ((memory_properties.memoryTypes[i].propertyFlags &
				requirements_mask) == requirements_mask) {
				*typeIndex = i;
				return true;
			}
		}
		typeBits >>= 1;
	}
	// No memory types matched, return failure
	return false;
}
