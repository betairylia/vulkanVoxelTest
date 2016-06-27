#pragma once
#include "RenderPass.h"
class SimpleTestPass :
	public RenderPass
{
public:
	SimpleTestPass();
	~SimpleTestPass();

	void initPass(SwapChainBuffer *buffers, int swapchainImageCount, VkFormat imgFormat);

	void CreateFrameBuffers(SwapChainBuffer *buffers, int swapchainImageCount);
	void CreateDepth();

	DepthMap depth;
};

