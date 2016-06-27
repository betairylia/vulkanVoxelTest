#include "deferredTestPass.h"



DeferredTestPass::DeferredTestPass()
{
}


DeferredTestPass::~DeferredTestPass()
{
}

void DeferredTestPass::initPass()
{
	framebuffers.resize(1);//uses 1 buffer

	vHelper::createImage(
		width, height,
		VK_FORMAT_R16G16B16A16_SFLOAT,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		&bufferImages.position,
		m_cmdBuf, m_queue, m_device, m_memProp);

	vHelper::createImage(
		width, height,
		VK_FORMAT_R16G16B16A16_SFLOAT,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		&bufferImages.normal,
		m_cmdBuf, m_queue, m_device, m_memProp);

	vHelper::createImage(
		width, height,
		VK_FORMAT_R16G16B16A16_SFLOAT,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		&bufferImages.color,
		m_cmdBuf, m_queue, m_device, m_memProp);

	vHelper::createImage(
		width, height,
		VK_FORMAT_D16_UNORM,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		&bufferImages.depth,
		m_cmdBuf, m_queue, m_device, m_memProp);

	vHelper::createImage(
		width, height,
		VK_FORMAT_R16G16B16A16_SFLOAT,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		&bufferImages.output,
		m_cmdBuf, m_queue, m_device, m_memProp);

	//create render pass
	VkResult res;

	/* Need attachments for render target and depth buffer */
	VkAttachmentDescription attachments[5];
	attachments[0].format = bufferImages.position.format;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	//clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachments[0].flags = 0;

	attachments[1].format = bufferImages.normal.format;
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	//clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachments[1].flags = 0;

	attachments[2].format = bufferImages.color.format;
	attachments[2].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	//clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[2].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachments[2].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachments[2].flags = 0;

	attachments[3].format = bufferImages.depth.format;
	attachments[3].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[3].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	//clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[3].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[3].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[3].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachments[3].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachments[3].flags = 0;

	attachments[4].format = bufferImages.output.format;
	attachments[4].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[4].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	//clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[4].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[4].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[4].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[4].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachments[4].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachments[4].flags = 0;

	VkAttachmentReference color_reference[3];
	color_reference[0].attachment = 0;
	color_reference[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	color_reference[1].attachment = 1;
	color_reference[1].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	color_reference[2].attachment = 2;
	color_reference[2].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference color_reference_full;
	color_reference_full.attachment = 4;
	color_reference_full.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_reference = {};
	depth_reference.attachment = 3;
	depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass[2];
	subpass[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass[0].flags = 0;
	subpass[0].inputAttachmentCount = 0;
	subpass[0].pInputAttachments = NULL;
	subpass[0].colorAttachmentCount = 3;
	subpass[0].pColorAttachments = color_reference;
	subpass[0].pResolveAttachments = NULL;
	subpass[0].pDepthStencilAttachment = &depth_reference;
	subpass[0].preserveAttachmentCount = 0;
	subpass[0].pPreserveAttachments = NULL;

	subpass[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass[1].flags = 0;
	subpass[1].inputAttachmentCount = 0;
	subpass[1].pInputAttachments = NULL;
	subpass[1].colorAttachmentCount = 1;
	subpass[1].pColorAttachments = &color_reference_full;
	subpass[1].pResolveAttachments = NULL;
	subpass[1].pDepthStencilAttachment = NULL;
	subpass[1].preserveAttachmentCount = 0;
	subpass[1].pPreserveAttachments = NULL;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = 0;
	dependency.dstSubpass = 1;
	dependency.dependencyFlags = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
	dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

	VkRenderPassCreateInfo rp_info = {};
	rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rp_info.pNext = NULL;
	rp_info.attachmentCount = 5;
	rp_info.pAttachments = attachments;
	rp_info.subpassCount = 2;
	rp_info.pSubpasses = subpass;
	rp_info.dependencyCount = 1;
	rp_info.pDependencies = &dependency;

	res = vkCreateRenderPass(m_device, &rp_info, NULL, &renderPass);
	assert(res == VK_SUCCESS);

	//Framebuffers

	VkImageView fb_attachments[5];
	fb_attachments[0] = bufferImages.position.view;
	fb_attachments[1] = bufferImages.normal.view;
	fb_attachments[2] = bufferImages.color.view;
	fb_attachments[3] = bufferImages.depth.view;
	fb_attachments[4] = bufferImages.output.view;

	VkFramebufferCreateInfo fb_info = {};
	fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fb_info.pNext = NULL;
	fb_info.renderPass = renderPass;
	fb_info.attachmentCount = 5;
	fb_info.pAttachments = fb_attachments;
	fb_info.width = width;
	fb_info.height = height;
	fb_info.layers = 1;

	uint32_t i;

	framebuffers.resize(1);

	res = vkCreateFramebuffer(m_device, &fb_info, NULL,
		&framebuffers[0]);
	assert(res == VK_SUCCESS);
}
