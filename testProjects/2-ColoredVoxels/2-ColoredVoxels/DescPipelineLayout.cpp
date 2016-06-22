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
