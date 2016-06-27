#pragma once

#define WIN32_LEAN_AND_MEAN
#define VK_USE_PLATFORM_WIN32_KHR

#include <cstdlib>
#include <string>
#include <vector>
#include <cassert>

#include <vulkan/vulkan.h>

#include "structs.h"

using namespace std;

class DescPipelineLayout
{
public:
	DescPipelineLayout();
	~DescPipelineLayout();
	void init();

	void initIA();

	vector<VkDescriptorSetLayout> descLayout;
	VkPipelineLayout pipelineLayout;

	VkDevice m_device;
};

