#pragma once

#define WIN32_LEAN_AND_MEAN
#define VK_USE_PLATFORM_WIN32_KHR

#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <cassert>
#include <vulkan/vulkan.h>

#include "SPIRV/GlslangToSpv.h"

#include "structs.h"
#include "BlockGroup.h"
#include "Renderable.h"

using namespace std;

class BlockGroupToBufferWorker
{
public:
	BlockGroupToBufferWorker();
	~BlockGroupToBufferWorker();

	void workCreate(VkDevice device, Renderable &renderable, BlockGroup &blockGroup);
	void workUpdate(VkDevice device, Renderable &renderable, BlockGroup &blockGroup);
};

