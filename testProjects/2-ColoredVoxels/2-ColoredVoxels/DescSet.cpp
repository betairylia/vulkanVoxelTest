#include "DescSet.h"



DescSet::DescSet()
{
}


DescSet::~DescSet()
{
}

void DescSet::setDescPool(VkDescriptorPool pool)
{
	m_descPool = pool;
}

void DescSet::CreateDescriptorSet(vector<VkDescriptorSetLayout>& desc_layout, UniformBuffer & uniform_data)
{
	VkResult res;

	VkDescriptorSetAllocateInfo alloc_info[2];
	alloc_info[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info[0].pNext = NULL;
	alloc_info[0].descriptorPool = m_descPool;
	alloc_info[0].descriptorSetCount = descSetCount;
	alloc_info[0].pSetLayouts = desc_layout.data();

	set.resize(descSetCount);
	res =
		vkAllocateDescriptorSets(m_device, alloc_info, set.data());
	assert(res == VK_SUCCESS);

	VkWriteDescriptorSet writes[2];

	writes[0] = {};
	writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[0].pNext = NULL;
	writes[0].dstSet = set[0];
	writes[0].descriptorCount = 1;
	writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writes[0].pBufferInfo = &uniform_data.buffer_info;
	writes[0].dstArrayElement = 0;
	writes[0].dstBinding = 0;

	vkUpdateDescriptorSets(m_device, 1, writes, 0, NULL);
}

void DescSet::CreateDescriptorSetIA(vector<VkDescriptorSetLayout>& desc_layout, UniformBuffer & uniform_data, VkSampler sampler, VkImageView* views, VkImageLayout imgLayout)
{
	VkResult res;

	VkDescriptorSetAllocateInfo alloc_info[1];
	alloc_info[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info[0].pNext = NULL;
	alloc_info[0].descriptorPool = m_descPool;
	alloc_info[0].descriptorSetCount = descSetCount;
	alloc_info[0].pSetLayouts = desc_layout.data();

	set.resize(descSetCount);
	res =
		vkAllocateDescriptorSets(m_device, alloc_info, set.data());
	assert(res == VK_SUCCESS);

	VkDescriptorImageInfo positionInfo = {};
	positionInfo.imageLayout = imgLayout;
	positionInfo.imageView = views[0];
	positionInfo.sampler = sampler;

	VkDescriptorImageInfo normalInfo = {};
	normalInfo.imageLayout = imgLayout;
	normalInfo.imageView = views[1];
	normalInfo.sampler = sampler;

	VkDescriptorImageInfo colorInfo = {};
	colorInfo.imageLayout = imgLayout;
	colorInfo.imageView = views[2];
	colorInfo.sampler = sampler;

	VkDescriptorImageInfo resultInfo = {};
	resultInfo.imageLayout = imgLayout;
	resultInfo.imageView = views[3];
	resultInfo.sampler = sampler;

	VkWriteDescriptorSet writes[5];

	writes[0] = {};
	writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[0].pNext = NULL;
	writes[0].dstSet = set[0];
	writes[0].descriptorCount = 1;
	writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writes[0].pBufferInfo = &uniform_data.buffer_info;
	writes[0].dstArrayElement = 0;
	writes[0].dstBinding = 0;

	writes[1] = {};
	writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[1].pNext = NULL;
	writes[1].dstSet = set[0];
	writes[1].descriptorCount = 1;
	writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writes[1].pImageInfo = &positionInfo;
	writes[1].dstArrayElement = 0;
	writes[1].dstBinding = 1;

	writes[2] = {};
	writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[2].pNext = NULL;
	writes[2].dstSet = set[0];
	writes[2].descriptorCount = 1;
	writes[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writes[2].pImageInfo = &normalInfo;
	writes[2].dstArrayElement = 0;
	writes[2].dstBinding = 2;

	writes[3] = {};
	writes[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[3].pNext = NULL;
	writes[3].dstSet = set[0];
	writes[3].descriptorCount = 1;
	writes[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writes[3].pImageInfo = &colorInfo;
	writes[3].dstArrayElement = 0;
	writes[3].dstBinding = 3;

	writes[4] = {};
	writes[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[4].pNext = NULL;
	writes[4].dstSet = set[0];
	writes[4].descriptorCount = 1;
	writes[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writes[4].pImageInfo = &resultInfo;
	writes[4].dstArrayElement = 0;
	writes[4].dstBinding = 4;

	vkUpdateDescriptorSets(m_device, 5, writes, 0, NULL);
}
