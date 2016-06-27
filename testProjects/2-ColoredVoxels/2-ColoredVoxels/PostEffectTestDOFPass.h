#pragma once
#include "RenderPass.h"
class PostEffectTestDOFPass :
	public RenderPass
{
public:
	PostEffectTestDOFPass();
	~PostEffectTestDOFPass();

	void initPass(SwapChainBuffer *buffers, int swapchainImageCount, VkFormat imgFormat);
};

