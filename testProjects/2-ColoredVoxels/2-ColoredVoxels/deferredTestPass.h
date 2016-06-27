#pragma once
#include "RenderPass.h"
class DeferredTestPass :
	public RenderPass
{
public:
	DeferredTestPass();
	~DeferredTestPass();

	void initPass();

	struct {
		GraphicsImage position;
		GraphicsImage normal;
		GraphicsImage color;
		GraphicsImage depth;
		GraphicsImage output;
	}bufferImages;
};

