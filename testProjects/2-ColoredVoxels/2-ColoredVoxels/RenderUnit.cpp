#include "RenderUnit.h"



RenderUnit::RenderUnit()
{
}


RenderUnit::~RenderUnit()
{
}

void RenderUnit::init(bool isPostEffect, VkDevice device, VkPhysicalDevice gpu, VkPhysicalDeviceMemoryProperties & memory_properties, VkCommandBuffer & cmdBuf, VkQueue & queue, int _width, int _height, std::vector<GraphicsImage> inputImageList, std::vector<GraphicsImage> outputImageList, VkDescriptorPool descPool, VkSampler sampler, VkVertexInputBindingDescription & vi_binding, VkVertexInputAttributeDescription * vi_attribs, const char * vertShaderText, const char * fragShaderText, bool useDepth)
{
	drawScreenQuad = isPostEffect;

	m_device = device;
	m_gpu = gpu;
	m_memProp = memory_properties;
	m_cmdBuf = cmdBuf;
	m_queue = queue;

	width = _width;
	height = _height;

	VkResult res;
	int i;
	int oSize = outputImageList.size(), iSize = inputImageList.size();
	iCount = iSize;
	oCount = oSize;
	clearCount = oSize + (useDepth ? 1 : 0);
	hasDepth = useDepth;

	//s1. set desc set & pipeline layout
	layout.m_device = device;
	layout.initWithCount(iSize);

	//s2. init render pass & frame buffers
	framebuffers.resize(1);//uses 1 buffer

	VkAttachmentDescription attachments[10];
	VkAttachmentReference color_reference[10];
	VkAttachmentReference depth_reference = {};

	for (i = 0; i < oSize; i++)
	{
		attachments[i].format = outputImageList[i].format;
		attachments[i].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		//clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[i].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachments[i].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachments[i].flags = 0;

		color_reference[i].attachment = i;
		color_reference[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		clearValues[i].color.float32[0] = 0.1f;
		clearValues[i].color.float32[1] = 0.1f;
		clearValues[i].color.float32[2] = 0.1f;
		clearValues[i].color.float32[3] = 1.0f;
	}

	if (useDepth)
	{
		vHelper::createImage(
			width, height,
			VK_FORMAT_D16_UNORM,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			&depth,
			m_cmdBuf, m_queue, m_device, m_memProp);

		attachments[oSize].format = depth.format;
		attachments[oSize].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[oSize].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		//clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[oSize].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[oSize].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[oSize].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[oSize].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachments[oSize].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachments[oSize].flags = 0;

		depth_reference.attachment = oSize;
		depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		clearValues[oSize].depthStencil.depth = 1.0f;
		clearValues[oSize].depthStencil.stencil = 0;
	}

	VkSubpassDescription subpass[1];
	subpass[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass[0].flags = 0;
	subpass[0].inputAttachmentCount = 0;
	subpass[0].pInputAttachments = NULL;
	subpass[0].colorAttachmentCount = oSize;
	subpass[0].pColorAttachments = color_reference;
	subpass[0].pResolveAttachments = NULL;
	subpass[0].pDepthStencilAttachment = useDepth ? &depth_reference : NULL;
	subpass[0].preserveAttachmentCount = 0;
	subpass[0].pPreserveAttachments = NULL;

	VkRenderPassCreateInfo rp_info = {};
	rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rp_info.pNext = NULL;
	rp_info.attachmentCount = oSize + (useDepth ? 1 : 0);
	rp_info.pAttachments = attachments;
	rp_info.subpassCount = 1;
	rp_info.pSubpasses = subpass;
	rp_info.dependencyCount = 0;
	rp_info.pDependencies = NULL;

	res = vkCreateRenderPass(m_device, &rp_info, NULL, &renderPass);
	assert(res == VK_SUCCESS);

	//fb
	VkImageView fb_attachments[10];

	for (i = 0; i < oSize; i++)
	{
		fb_attachments[i] = outputImageList[i].view;
	}

	if (useDepth)
	{
		fb_attachments[oSize] = depth.view;
	}

	VkFramebufferCreateInfo fb_info = {};
	fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fb_info.pNext = NULL;
	fb_info.renderPass = renderPass;
	fb_info.attachmentCount = oSize + (useDepth ? 1 : 0);
	fb_info.pAttachments = fb_attachments;
	fb_info.width = width;
	fb_info.height = height;
	fb_info.layers = 1;

	res = vkCreateFramebuffer(m_device, &fb_info, NULL,
		&framebuffers[0]);
	assert(res == VK_SUCCESS);

	//s3. init desc set
	descSet.m_device = device;
	descSet.setDescPool(descPool);
	descSet.CreateDescriptorSetPassScale(layout.descLayout[2], sampler, inputImageList, VK_IMAGE_LAYOUT_GENERAL);

	//s4. init pipeline
	pipeline.m_device = device;
	pipeline.InitShader(vertShaderText, fragShaderText);
	pipeline.initPipelineForUnit(vi_binding, vi_attribs, layout.pipelineLayout, pipeline.info, renderPass, oSize, useDepth);
}

void RenderUnit::initAsLastUnit(bool isPostEffect, VkDevice device, VkPhysicalDevice gpu, VkPhysicalDeviceMemoryProperties & memory_properties, VkCommandBuffer & cmdBuf, VkQueue & queue, int _width, int _height, std::vector<GraphicsImage> inputImageList, SwapChainBuffer * buffers, int swapchainImageCount, VkFormat imgFormat, VkDescriptorPool descPool, VkSampler sampler, VkVertexInputBindingDescription & vi_binding, VkVertexInputAttributeDescription * vi_attribs, const char * vertShaderText, const char * fragShaderText, bool useDepth)
{
	asLast = true;
	drawScreenQuad = isPostEffect;

	m_device = device;
	m_gpu = gpu;
	m_memProp = memory_properties;
	m_cmdBuf = cmdBuf;
	m_queue = queue;

	width = _width;
	height = _height;

	VkResult res;
	int i;
	int iSize = inputImageList.size();
	iCount = iSize;
	oCount = 1;
	clearCount = 1 + (useDepth ? 1 : 0);
	hasDepth = useDepth;

	//s1. set desc set & pipeline layout
	layout.m_device = device;
	layout.initWithCount(iSize);

	//s2. init render pass & frame buffers
	framebuffers.resize(1);//uses 1 buffer

	VkAttachmentDescription attachments[10];
	VkAttachmentReference color_reference[10];
	VkAttachmentReference depth_reference = {};

	attachments[0].format = imgFormat;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	//clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachments[0].flags = 0;

	color_reference[0].attachment = 0;
	color_reference[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	clearValues[0].color.float32[0] = 0.1f;
	clearValues[0].color.float32[1] = 0.1f;
	clearValues[0].color.float32[2] = 0.1f;
	clearValues[0].color.float32[3] = 1.0f;

	if (useDepth)
	{
		vHelper::createImage(
			width, height,
			VK_FORMAT_D16_UNORM,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			&depth,
			m_cmdBuf, m_queue, m_device, m_memProp);

		attachments[1].format = depth.format;
		attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		//clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachments[1].flags = 0;

		depth_reference.attachment = 1;
		depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		clearValues[1].depthStencil.depth = 1.0f;
		clearValues[1].depthStencil.stencil = 0;
	}

	VkSubpassDescription subpass[1];
	subpass[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass[0].flags = 0;
	subpass[0].inputAttachmentCount = 0;
	subpass[0].pInputAttachments = NULL;
	subpass[0].colorAttachmentCount = 1;
	subpass[0].pColorAttachments = color_reference;
	subpass[0].pResolveAttachments = NULL;
	subpass[0].pDepthStencilAttachment = useDepth ? &depth_reference : NULL;
	subpass[0].preserveAttachmentCount = 0;
	subpass[0].pPreserveAttachments = NULL;

	VkRenderPassCreateInfo rp_info = {};
	rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rp_info.pNext = NULL;
	rp_info.attachmentCount = 1 + (useDepth ? 1 : 0);
	rp_info.pAttachments = attachments;
	rp_info.subpassCount = 1;
	rp_info.pSubpasses = subpass;
	rp_info.dependencyCount = 0;
	rp_info.pDependencies = NULL;

	res = vkCreateRenderPass(m_device, &rp_info, NULL, &renderPass);
	assert(res == VK_SUCCESS);

	//fb
	VkImageView fb_attachments[10];

	if (useDepth)
	{
		fb_attachments[1] = depth.view;
	}

	VkFramebufferCreateInfo fb_info = {};
	fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fb_info.pNext = NULL;
	fb_info.renderPass = renderPass;
	fb_info.attachmentCount = 1 + (useDepth ? 1 : 0);
	fb_info.pAttachments = fb_attachments;
	fb_info.width = width;
	fb_info.height = height;
	fb_info.layers = 1;
	
	framebuffers.resize(swapchainImageCount);

	for (i = 0;i < swapchainImageCount;i++)
	{
		fb_attachments[0] = buffers[i].view;

		res = vkCreateFramebuffer(m_device, &fb_info, NULL,
			&framebuffers[i]);
		assert(res == VK_SUCCESS);
	}

	//s3. init desc set
	descSet.m_device = device;
	descSet.setDescPool(descPool);
	descSet.CreateDescriptorSetPassScale(layout.descLayout[2], sampler, inputImageList, VK_IMAGE_LAYOUT_GENERAL);

	//s4. init pipeline
	pipeline.m_device = device;
	pipeline.InitShader(vertShaderText, fragShaderText);
	pipeline.initPipelineForUnit(vi_binding, vi_attribs, layout.pipelineLayout, pipeline.info, renderPass, 1, useDepth);
}
