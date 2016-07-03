#include "DescPipelineLayout.h"



DescPipelineLayout::DescPipelineLayout()
{
}


DescPipelineLayout::~DescPipelineLayout()
{
}

void DescPipelineLayout::init()
{
	VkDescriptorSetLayoutBinding layout_bindings[2];
	layout_bindings[0].binding = 0;
	layout_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layout_bindings[0].descriptorCount = 1;
	layout_bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layout_bindings[0].pImmutableSamplers = NULL;

	/* Next take layout bindings and use them to create a descriptor set layout
	*/
	VkDescriptorSetLayoutCreateInfo descriptor_layout = {};
	descriptor_layout.sType =
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptor_layout.pNext = NULL;
	descriptor_layout.bindingCount = 1;
	descriptor_layout.pBindings = layout_bindings;

	VkResult res;

	descLayout.resize(1);
	res = vkCreateDescriptorSetLayout(m_device, &descriptor_layout, NULL,
		descLayout.data());
	assert(res == VK_SUCCESS);

	/* Now use the descriptor layout to create a pipeline layout */
	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
	pPipelineLayoutCreateInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pPipelineLayoutCreateInfo.pNext = NULL;
	pPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pPipelineLayoutCreateInfo.pPushConstantRanges = NULL;
	pPipelineLayoutCreateInfo.setLayoutCount = 1;
	pPipelineLayoutCreateInfo.pSetLayouts = descLayout.data();

	res = vkCreatePipelineLayout(m_device, &pPipelineLayoutCreateInfo, NULL,
		&pipelineLayout);
	assert(res == VK_SUCCESS);
}

void DescPipelineLayout::initIA()
{
	VkDescriptorSetLayoutBinding layout_bindings[5];
	layout_bindings[0].binding = 0;
	layout_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layout_bindings[0].descriptorCount = 1;
	layout_bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layout_bindings[0].pImmutableSamplers = NULL;

	layout_bindings[1].binding = 1;
	layout_bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	layout_bindings[1].descriptorCount = 1;
	layout_bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	layout_bindings[1].pImmutableSamplers = NULL;

	layout_bindings[2].binding = 2;
	layout_bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	layout_bindings[2].descriptorCount = 1;
	layout_bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	layout_bindings[2].pImmutableSamplers = NULL;

	layout_bindings[3].binding = 3;
	layout_bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	layout_bindings[3].descriptorCount = 1;
	layout_bindings[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	layout_bindings[3].pImmutableSamplers = NULL;

	layout_bindings[4].binding = 4;
	layout_bindings[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	layout_bindings[4].descriptorCount = 1;
	layout_bindings[4].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	layout_bindings[4].pImmutableSamplers = NULL;

	/* Next take layout bindings and use them to create a descriptor set layout
	*/
	VkDescriptorSetLayoutCreateInfo descriptor_layout = {};
	descriptor_layout.sType =
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptor_layout.pNext = NULL;
	descriptor_layout.bindingCount = 5;
	descriptor_layout.pBindings = layout_bindings;

	VkResult res;

	descLayout.resize(1);
	res = vkCreateDescriptorSetLayout(m_device, &descriptor_layout, NULL,
		descLayout.data());
	assert(res == VK_SUCCESS);

	/* Now use the descriptor layout to create a pipeline layout */
	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
	pPipelineLayoutCreateInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pPipelineLayoutCreateInfo.pNext = NULL;
	pPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pPipelineLayoutCreateInfo.pPushConstantRanges = NULL;
	pPipelineLayoutCreateInfo.setLayoutCount = 1;
	pPipelineLayoutCreateInfo.pSetLayouts = descLayout.data();

	res = vkCreatePipelineLayout(m_device, &pPipelineLayoutCreateInfo, NULL,
		&pipelineLayout);
	assert(res == VK_SUCCESS);
}

void DescPipelineLayout::initWithCount(int imgCount)
{
	descLayout.resize(5);
	VkResult res;

	VkDescriptorSetLayoutBinding layout_bindings[10];
	layout_bindings[0].binding = 0;
	layout_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layout_bindings[0].descriptorCount = 1;
	layout_bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layout_bindings[0].pImmutableSamplers = NULL;

	VkDescriptorSetLayoutCreateInfo descriptor_layout = {};
	descriptor_layout.sType =
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptor_layout.pNext = NULL;
	descriptor_layout.bindingCount = 1;
	descriptor_layout.pBindings = layout_bindings;

	//Set 0: Model-scale Set
	res = vkCreateDescriptorSetLayout(m_device, &descriptor_layout, NULL,
		&descLayout[0]);
	assert(res == VK_SUCCESS);

	//Set 1: Frame-scale Set
	res = vkCreateDescriptorSetLayout(m_device, &descriptor_layout, NULL,
		&descLayout[1]);
	assert(res == VK_SUCCESS);

	for (int i = 0; i < imgCount; i++)
	{
		layout_bindings[i].binding = i;
		layout_bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		layout_bindings[i].descriptorCount = 1;
		layout_bindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		layout_bindings[i].pImmutableSamplers = NULL;
	}

	/* Next take layout bindings and use them to create a descriptor set layout
	*/
	descriptor_layout.sType =
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptor_layout.pNext = NULL;
	descriptor_layout.bindingCount = imgCount;
	descriptor_layout.pBindings = layout_bindings;

	//Set 2: Pass-scale Set
	res = vkCreateDescriptorSetLayout(m_device, &descriptor_layout, NULL,
		&descLayout[2]);
	assert(res == VK_SUCCESS);

	/* Now use the descriptor layout to create a pipeline layout */
	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
	pPipelineLayoutCreateInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pPipelineLayoutCreateInfo.pNext = NULL;
	pPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pPipelineLayoutCreateInfo.pPushConstantRanges = NULL;
	pPipelineLayoutCreateInfo.setLayoutCount = 3;
	pPipelineLayoutCreateInfo.pSetLayouts = descLayout.data();
	//pPipelineLayoutCreateInfo.setLayoutCount = 1;
	//pPipelineLayoutCreateInfo.pSetLayouts = &descLayout[2];

	res = vkCreatePipelineLayout(m_device, &pPipelineLayoutCreateInfo, NULL,
		&pipelineLayout);
	assert(res == VK_SUCCESS);
}
