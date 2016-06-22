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
