#include "VulkanRenderer.h"

/*

	TODO:
		1)	Create material class from pipeline & desc sets
		2)	Seperate render passes

*/

VulkanRenderer::VulkanRenderer()
{
}


VulkanRenderer::~VulkanRenderer()
{
}

void VulkanRenderer::init()
{
	//Create Instance
	m_instance = CreateInstance("Colored Cube");

	//Enumerate & Create device
	EnumeratePhysicalDevices(m_instance, m_GPUs, m_memoryProperties, m_GPUProperties);
	m_device = CreateDevice(m_GPUs, m_queueCount, m_queueProps, m_queueFamilyIndex);

	//Init command buffer
	m_cmdPool = CreateCommandPool(m_queueFamilyIndex, m_device);
	m_cmdBuffer = CreateCommandBuffer(m_cmdPool, m_device);

	BeginCommandBuffer(m_cmdBuffer);

	//Init swapChain
	m_surface = GetSurface(m_connection, m_window, m_instance);
	m_queueGraphicsPresentIndex = GetGraphicsPresentQueueIndex(m_queueCount, m_GPUs[0], m_queueProps, m_surface);
	m_swapChain = CreateSwapChain(
		m_GPUs[0], m_surface, m_colorImgFormat, width, height, m_device,
		m_swapChainImageCount, m_swapChainImgBuffer, m_swapChainImgBuffer2, m_cmdBuffer, m_queue, m_queueFamilyIndex);

	//Create DepthBuffer
	m_depthMap = CreateDepthMap(m_GPUs[0], m_device, width, height, m_memoryProperties, m_cmdBuffer, m_queue);

	//Create UniformBuffer

	//Descriptor & pipeline Layout
	//InitDescriptorPipelineLayout(m_descLayout, m_pipelineLayout, m_device);

	//RenderPass
	m_renderPass = CreateRenderPass(m_colorImgFormat, m_depthMap, m_device);

	//Shader. glsl to SPIR-V by glslang.

	//Create frame buffers
	m_framebuffers = CreateFrameBuffers(m_device, m_depthMap, m_swapChainImgBuffer.data(), m_swapChainImgBuffer2.data(), m_renderPass, width, height, m_swapChainImageCount);

	//Vertex buffer
	//Index buffer

	//Desc pool & set
	m_descPool = CreateDescriptorPool(m_device);

	//Pipeline

	initViewports(width, height, m_cmdBuffer, m_viewport);
	initScissors(width, height, m_cmdBuffer, m_scissor);

	//sampler to sampling from framebuffer attachments
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.pNext = NULL;
	samplerInfo.magFilter = VK_FILTER_NEAREST;
	samplerInfo.minFilter = VK_FILTER_NEAREST;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV = samplerInfo.addressModeU;
	samplerInfo.addressModeW = samplerInfo.addressModeU;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.maxAnisotropy = 0;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 1.0f;
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

	VkResult res = vkCreateSampler(m_device, &samplerInfo, NULL, &simpleSampler);
	assert(res == VK_SUCCESS);

	//Finish init
	EndCommandBuffer(m_cmdBuffer);
	QueueCommandBuffer(m_cmdBuffer, m_device, m_queue);

	m_prepared = true;
}

void VulkanRenderer::InitRenderChain(
	const void * vertexData, uint32_t vdataSize, uint32_t vdataStride,
	const void *  indexData, uint32_t idataSize, uint32_t idataStride)
{
	//we need: position, normal, color, combined(SSAO), blurX, blurY, DOF(swapChainImage) (6 new images total)
	//chain:	(buffer)			=>	position, normal, color
	//			pos,norm,col		=>	combined(SSAO)
	//			combined, norm		=>	blurX
	//			blurX, norm			=>	blurY
	//			blurY, position		=>	DOF(swapChain)

	//init images
	vHelper::createImage(
		width, height,
		VK_FORMAT_R16G16B16A16_SFLOAT,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		&imagePool.position,
		m_cmdBuffer, m_queue, m_device, m_memoryProperties);

	vHelper::createImage(
		width, height,
		VK_FORMAT_R16G16B16A16_SFLOAT,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		&imagePool.normal,
		m_cmdBuffer, m_queue, m_device, m_memoryProperties);

	vHelper::createImage(
		width, height,
		VK_FORMAT_R16G16B16A16_SFLOAT,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		&imagePool.color,
		m_cmdBuffer, m_queue, m_device, m_memoryProperties);

	vHelper::createImage(
		width, height,
		VK_FORMAT_R16G16B16A16_SFLOAT,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		&imagePool.combined_AO,
		m_cmdBuffer, m_queue, m_device, m_memoryProperties);

	vHelper::createImage(
		width, height,
		VK_FORMAT_R16G16B16A16_SFLOAT,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		&imagePool.blurX_AO,
		m_cmdBuffer, m_queue, m_device, m_memoryProperties);

	vHelper::createImage(
		width, height,
		VK_FORMAT_R16G16B16A16_SFLOAT,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		&imagePool.blurY_AO,
		m_cmdBuffer, m_queue, m_device, m_memoryProperties);

	//init units
	RenderUnit gBufferWorker;
	gBufferWorker.init(
		false, m_device, m_GPUs[0], m_memoryProperties,
		m_cmdBuffer, m_queue,
		width, height,
		{}, { imagePool.position, imagePool.normal, imagePool.color },
		m_descPool, simpleSampler, m_viBinding, m_viAttribs,
		vHelper::ReadFileString("mrt.vert").c_str(), vHelper::ReadFileString("mrt.frag").c_str(),
		true);
	unitChain.push_back(gBufferWorker);

	RenderUnit ssaoCombiner;
	ssaoCombiner.init(
		true, m_device, m_GPUs[0], m_memoryProperties,
		m_cmdBuffer, m_queue,
		width, height,
		{ imagePool.position, imagePool.normal, imagePool.color }, { imagePool.combined_AO },
		m_descPool, simpleSampler, m_viBinding, m_viAttribs,
		vHelper::ReadFileString("SQuad.vert").c_str(), vHelper::ReadFileString("ssao.frag").c_str(),
		false);
	unitChain.push_back(ssaoCombiner);

	RenderUnit blurX;
	blurX.init(
		true, m_device, m_GPUs[0], m_memoryProperties,
		m_cmdBuffer, m_queue,
		width, height,
		{ imagePool.combined_AO, imagePool.normal }, { imagePool.blurX_AO },
		m_descPool, simpleSampler, m_viBinding, m_viAttribs,
		vHelper::ReadFileString("SQuad.vert").c_str(), vHelper::ReadFileString("blurX.frag").c_str(),
		false);
	unitChain.push_back(blurX);

	RenderUnit blurY;
	blurY.init(
		true, m_device, m_GPUs[0], m_memoryProperties,
		m_cmdBuffer, m_queue,
		width, height,
		{ imagePool.blurX_AO, imagePool.normal }, { imagePool.blurY_AO },
		m_descPool, simpleSampler, m_viBinding, m_viAttribs,
		vHelper::ReadFileString("SQuad.vert").c_str(), vHelper::ReadFileString("blurY.frag").c_str(),
		false);
	unitChain.push_back(blurY);

	RenderUnit DOF;
	DOF.initAsLastUnit(
		true, m_device, m_GPUs[0], m_memoryProperties,
		m_cmdBuffer, m_queue,
		width, height,
		{ imagePool.blurY_AO, imagePool.position },
		m_swapChainImgBuffer.data(), m_swapChainImageCount, m_colorImgFormat,
		m_descPool, simpleSampler, m_viBinding, m_viAttribs,
		vHelper::ReadFileString("SQuad.vert").c_str(), vHelper::ReadFileString("DOF.frag").c_str(),
		false);
	unitChain.push_back(DOF);

	unitCount = unitChain.size();

	screenAlignedQuad.init(m_device, m_descPool, { gBufferWorker.layout.descLayout[0] }, m_memoryProperties);
	//screenAlignedQuad.initIA(m_device, m_descPool, layoutIA.descLayout, m_memoryProperties, simpleSampler, views, VK_IMAGE_LAYOUT_GENERAL);
	screenAlignedQuad.SetVertexBuffer(m_memoryProperties, vertexData, vdataSize, vdataStride, m_viBinding, m_viAttribs);
	screenAlignedQuad.SetIndexBuffer(m_memoryProperties, indexData, idataSize, idataStride);
	screenAlignedQuad.SetScreenQuad(m_memoryProperties);
}

const char * VulkanRenderer::GetRawTextFromFile(const char * fileName)
{
	return "";
}

/*
ToolBoxes
*/

void VulkanRenderer::initResources(TBuiltInResource &Resources) {
	Resources.maxLights = 32;
	Resources.maxClipPlanes = 6;
	Resources.maxTextureUnits = 32;
	Resources.maxTextureCoords = 32;
	Resources.maxVertexAttribs = 64;
	Resources.maxVertexUniformComponents = 4096;
	Resources.maxVaryingFloats = 64;
	Resources.maxVertexTextureImageUnits = 32;
	Resources.maxCombinedTextureImageUnits = 80;
	Resources.maxTextureImageUnits = 32;
	Resources.maxFragmentUniformComponents = 4096;
	Resources.maxDrawBuffers = 32;
	Resources.maxVertexUniformVectors = 128;
	Resources.maxVaryingVectors = 8;
	Resources.maxFragmentUniformVectors = 16;
	Resources.maxVertexOutputVectors = 16;
	Resources.maxFragmentInputVectors = 15;
	Resources.minProgramTexelOffset = -8;
	Resources.maxProgramTexelOffset = 7;
	Resources.maxClipDistances = 8;
	Resources.maxComputeWorkGroupCountX = 65535;
	Resources.maxComputeWorkGroupCountY = 65535;
	Resources.maxComputeWorkGroupCountZ = 65535;
	Resources.maxComputeWorkGroupSizeX = 1024;
	Resources.maxComputeWorkGroupSizeY = 1024;
	Resources.maxComputeWorkGroupSizeZ = 64;
	Resources.maxComputeUniformComponents = 1024;
	Resources.maxComputeTextureImageUnits = 16;
	Resources.maxComputeImageUniforms = 8;
	Resources.maxComputeAtomicCounters = 8;
	Resources.maxComputeAtomicCounterBuffers = 1;
	Resources.maxVaryingComponents = 60;
	Resources.maxVertexOutputComponents = 64;
	Resources.maxGeometryInputComponents = 64;
	Resources.maxGeometryOutputComponents = 128;
	Resources.maxFragmentInputComponents = 128;
	Resources.maxImageUnits = 8;
	Resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
	Resources.maxCombinedShaderOutputResources = 8;
	Resources.maxImageSamples = 0;
	Resources.maxVertexImageUniforms = 0;
	Resources.maxTessControlImageUniforms = 0;
	Resources.maxTessEvaluationImageUniforms = 0;
	Resources.maxGeometryImageUniforms = 0;
	Resources.maxFragmentImageUniforms = 8;
	Resources.maxCombinedImageUniforms = 8;
	Resources.maxGeometryTextureImageUnits = 16;
	Resources.maxGeometryOutputVertices = 256;
	Resources.maxGeometryTotalOutputComponents = 1024;
	Resources.maxGeometryUniformComponents = 1024;
	Resources.maxGeometryVaryingComponents = 64;
	Resources.maxTessControlInputComponents = 128;
	Resources.maxTessControlOutputComponents = 128;
	Resources.maxTessControlTextureImageUnits = 16;
	Resources.maxTessControlUniformComponents = 1024;
	Resources.maxTessControlTotalOutputComponents = 4096;
	Resources.maxTessEvaluationInputComponents = 128;
	Resources.maxTessEvaluationOutputComponents = 128;
	Resources.maxTessEvaluationTextureImageUnits = 16;
	Resources.maxTessEvaluationUniformComponents = 1024;
	Resources.maxTessPatchComponents = 120;
	Resources.maxPatchVertices = 32;
	Resources.maxTessGenLevel = 64;
	Resources.maxViewports = 16;
	Resources.maxVertexAtomicCounters = 0;
	Resources.maxTessControlAtomicCounters = 0;
	Resources.maxTessEvaluationAtomicCounters = 0;
	Resources.maxGeometryAtomicCounters = 0;
	Resources.maxFragmentAtomicCounters = 8;
	Resources.maxCombinedAtomicCounters = 8;
	Resources.maxAtomicCounterBindings = 1;
	Resources.maxVertexAtomicCounterBuffers = 0;
	Resources.maxTessControlAtomicCounterBuffers = 0;
	Resources.maxTessEvaluationAtomicCounterBuffers = 0;
	Resources.maxGeometryAtomicCounterBuffers = 0;
	Resources.maxFragmentAtomicCounterBuffers = 1;
	Resources.maxCombinedAtomicCounterBuffers = 1;
	Resources.maxAtomicCounterBufferSize = 16384;
	Resources.maxTransformFeedbackBuffers = 4;
	Resources.maxTransformFeedbackInterleavedComponents = 64;
	Resources.maxCullDistances = 8;
	Resources.maxCombinedClipAndCullDistances = 8;
	Resources.maxSamples = 4;
	Resources.limits.nonInductiveForLoops = 1;
	Resources.limits.whileLoops = 1;
	Resources.limits.doWhileLoops = 1;
	Resources.limits.generalUniformIndexing = 1;
	Resources.limits.generalAttributeMatrixVectorIndexing = 1;
	Resources.limits.generalVaryingIndexing = 1;
	Resources.limits.generalSamplerIndexing = 1;
	Resources.limits.generalVariableIndexing = 1;
	Resources.limits.generalConstantMatrixVectorIndexing = 1;
}

EShLanguage VulkanRenderer::FindLanguage(const VkShaderStageFlagBits shader_type) {
	switch (shader_type) {
	case VK_SHADER_STAGE_VERTEX_BIT:
		return EShLangVertex;

	case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
		return EShLangTessControl;

	case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
		return EShLangTessEvaluation;

	case VK_SHADER_STAGE_GEOMETRY_BIT:
		return EShLangGeometry;

	case VK_SHADER_STAGE_FRAGMENT_BIT:
		return EShLangFragment;

	case VK_SHADER_STAGE_COMPUTE_BIT:
		return EShLangCompute;

	default:
		return EShLangVertex;
	}
}

bool VulkanRenderer::GLSLtoSPV(const VkShaderStageFlagBits shader_type, const char *pshader,
	std::vector<unsigned int> &spirv) {

	EShLanguage stage = FindLanguage(shader_type);
	glslang::TShader shader(stage);
	glslang::TProgram program;
	const char *shaderStrings[1];
	TBuiltInResource Resources;
	initResources(Resources);

	// Enable SPIR-V and Vulkan rules when parsing GLSL
	EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

	shaderStrings[0] = pshader;
	shader.setStrings(shaderStrings, 1);

	if (!shader.parse(&Resources, 100, false, messages)) {
		puts(shader.getInfoLog());
		puts(shader.getInfoDebugLog());
		return false; // something didn't work
	}

	program.addShader(&shader);

	//
	// Program-level processing...
	//

	if (!program.link(messages)) {
		puts(shader.getInfoLog());
		puts(shader.getInfoDebugLog());
		fflush(stdout);
		return false;
	}

	glslang::GlslangToSpv(*program.getIntermediate(stage), spirv);
	return true;
}

void VulkanRenderer::BeginCommandBuffer(VkCommandBuffer &cmdBuf)
{
	VkCommandBufferBeginInfo cmd_buf_info = {};
	cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmd_buf_info.pNext = NULL;
	cmd_buf_info.flags = 0;
	cmd_buf_info.pInheritanceInfo = NULL;

	VkResult res = vkBeginCommandBuffer(cmdBuf, &cmd_buf_info);
	assert(res == VK_SUCCESS);
}

void VulkanRenderer::EndCommandBuffer(VkCommandBuffer &cmdBuf)
{
	VkResult res = vkEndCommandBuffer(cmdBuf);
	assert(res == VK_SUCCESS);
}

void VulkanRenderer::QueueCommandBuffer(VkCommandBuffer &cmdBuf, VkDevice &device, VkQueue &queue)
{
	VkResult res;

	/* Queue the command buffer for execution */
	const VkCommandBuffer cmd_bufs[] = { cmdBuf };
	VkFenceCreateInfo fenceInfo;
	VkFence drawFence;
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.pNext = NULL;
	fenceInfo.flags = 0;
	vkCreateFence(device, &fenceInfo, NULL, &drawFence);

	VkPipelineStageFlags pipe_stage_flags =
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	VkSubmitInfo submit_info[1] = {};
	submit_info[0].pNext = NULL;
	submit_info[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info[0].waitSemaphoreCount = 0;
	submit_info[0].pWaitSemaphores = NULL;
	submit_info[0].pWaitDstStageMask = &pipe_stage_flags;
	submit_info[0].commandBufferCount = 1;
	submit_info[0].pCommandBuffers = cmd_bufs;
	submit_info[0].signalSemaphoreCount = 0;
	submit_info[0].pSignalSemaphores = NULL;

	res = vkQueueSubmit(queue, 1, submit_info, drawFence);
	assert(res == VK_SUCCESS);

	do {
		res =
			vkWaitForFences(device, 1, &drawFence, VK_TRUE, fenceTimeout);
	} while (res == VK_TIMEOUT);
	assert(res == VK_SUCCESS);

	vkDestroyFence(device, drawFence, NULL);
}

/*
Steps
*/

VkInstance VulkanRenderer::CreateInstance(string instanceName)
{
	VkApplicationInfo app_info = {};

	// initialize the VkApplicationInfo structure
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pNext = NULL;
	app_info.pApplicationName = instanceName.c_str();
	app_info.applicationVersion = 1;
	app_info.pEngineName = instanceName.c_str();
	app_info.engineVersion = 1;
	app_info.apiVersion = VK_API_VERSION_1_0;

	vector<char*> enabledInstanceExtensions;
	enabledInstanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	enabledInstanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#ifdef VALIDATE_VULKAN
	enabledInstanceExtensions.push_back("VK_EXT_debug_report");
#endif

	vector<char*> enabledInstanceLayers;
#ifdef VALIDATE_VULKAN
	enabledInstanceLayers.push_back("VK_LAYER_LUNARG_standard_validation");
#endif

	// initialize the VkInstanceCreateInfo structure
	VkInstanceCreateInfo inst_info = {};
	inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	inst_info.pNext = NULL;
	inst_info.flags = 0;
	inst_info.pApplicationInfo = &app_info;
	inst_info.enabledExtensionCount = (uint32_t)enabledInstanceExtensions.size();
	inst_info.ppEnabledExtensionNames = enabledInstanceExtensions.data();
	inst_info.enabledLayerCount = (uint32_t)enabledInstanceLayers.size();
	inst_info.ppEnabledLayerNames = enabledInstanceLayers.data();

	//We need to request the extensions necessary to create a surface!

	VkInstance inst;
	VkResult res;

	res = vkCreateInstance(&inst_info, NULL, &inst);
	if (res == VK_ERROR_INCOMPATIBLE_DRIVER) {
		std::cout << "cannot find a compatible Vulkan ICD\n";
		exit(-1);
	}
	else if (res) {
		std::cout << "unknown error\n";
		exit(-1);
	}

	return inst;
}

void VulkanRenderer::EnumeratePhysicalDevices(VkInstance &inst, vector<VkPhysicalDevice> &gpus, VkPhysicalDeviceMemoryProperties &memory_properties, VkPhysicalDeviceProperties &gpu_props)
{
	uint32_t gpu_count = 1;
	VkResult res = vkEnumeratePhysicalDevices(inst, &gpu_count, NULL);
	assert(gpu_count);
	gpus.resize(gpu_count);
	res = vkEnumeratePhysicalDevices(inst, &gpu_count, gpus.data());
	assert(!res && gpu_count >= 1);

	vkGetPhysicalDeviceMemoryProperties(gpus[0], &memory_properties);
	vkGetPhysicalDeviceProperties(gpus[0], &gpu_props);
}

VkDevice VulkanRenderer::CreateDevice(
	vector<VkPhysicalDevice> &gpus, uint32_t &queueCount,
	vector<VkQueueFamilyProperties> &queueProps, uint32_t &familyIndex)
{
	VkDeviceQueueCreateInfo queue_info = {};

	vkGetPhysicalDeviceQueueFamilyProperties(gpus[0], &queueCount, NULL);
	assert(queueCount >= 1);

	queueProps.resize(queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(gpus[0], &queueCount,
		queueProps.data());
	assert(queueCount >= 1);

	bool found = false;
	for (unsigned int i = 0; i < queueCount; i++) {
		if (queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			familyIndex = queue_info.queueFamilyIndex = i;
			found = true;
			break;
		}
	}
	assert(found);
	assert(queueCount >= 1);

	float queue_priorities[1] = { 0.0 };
	queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_info.pNext = NULL;
	queue_info.queueCount = 1;
	queue_info.pQueuePriorities = queue_priorities;

	vector<char*> enabledDeviceExtensions;
	enabledDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
#ifdef VALIDATE_VULKAN
	enabledInstanceExtensions.push_back("VK_EXT_debug_report");
#endif

	vector<char*> enabledDeviceLayers;
#ifdef VALIDATE_VULKAN
	enabledInstanceLayers.push_back("VK_LAYER_LUNARG_standard_validation");
#endif

	VkDeviceCreateInfo device_info = {};
	device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_info.pNext = NULL;
	device_info.queueCreateInfoCount = 1;
	device_info.pQueueCreateInfos = &queue_info;
	device_info.enabledExtensionCount = (uint32_t)enabledDeviceExtensions.size();
	device_info.ppEnabledExtensionNames = enabledDeviceExtensions.data();
	device_info.enabledLayerCount = (uint32_t)enabledDeviceLayers.size();
	device_info.ppEnabledLayerNames = enabledDeviceLayers.data();
	device_info.pEnabledFeatures = NULL;

	VkDevice device;
	VkResult res =
		vkCreateDevice(gpus[0], &device_info, NULL, &device);
	assert(res == VK_SUCCESS);

	return device;
}

VkCommandPool VulkanRenderer::CreateCommandPool(const uint32_t queueFamilyIndex, VkDevice &device)
{
	VkCommandPool cmdPool;

	VkCommandPoolCreateInfo cmd_pool_info = {};
	cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmd_pool_info.pNext = NULL;
	cmd_pool_info.queueFamilyIndex = queueFamilyIndex;

	//TODO: Maybe we need modify flags here.
	cmd_pool_info.flags = 0;

	VkResult res =
		vkCreateCommandPool(device, &cmd_pool_info, NULL, &cmdPool);
	assert(res == VK_SUCCESS);

	return cmdPool;
}

VkCommandBuffer VulkanRenderer::CreateCommandBuffer(VkCommandPool &cmdPool, VkDevice &device)
{
	VkCommandBuffer cmdBuf;

	VkCommandBufferAllocateInfo cmd = {};
	cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmd.pNext = NULL;
	cmd.commandPool = cmdPool;
	cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmd.commandBufferCount = 1;

	VkResult res = vkAllocateCommandBuffers(device, &cmd, &cmdBuf);
	assert(res == VK_SUCCESS);

	return cmdBuf;
}

VkSurfaceKHR VulkanRenderer::GetSurface(HINSTANCE hinstance, HWND window, VkInstance &instance)
{
	//WIN32 Only
	VkSurfaceKHR surface = 0;

	VkWin32SurfaceCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext = NULL;
	createInfo.hinstance = hinstance;
	createInfo.hwnd = window;

	VkResult res = vkCreateWin32SurfaceKHR(instance, &createInfo, NULL, &surface);
	assert(res == VK_SUCCESS);

	return surface;
}

uint32_t VulkanRenderer::GetGraphicsPresentQueueIndex(
	const uint32_t queueCount, VkPhysicalDevice gpu,
	vector<VkQueueFamilyProperties> queueProps, VkSurfaceKHR surface)
{
	VkBool32 *supportsPresent =
		(VkBool32 *)malloc(queueCount * sizeof(VkBool32));
	for (uint32_t i = 0; i < queueCount; i++) {
		vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, surface,
			&supportsPresent[i]);
	}

	// Search for a graphics queue and a present queue in the array of queue
	// families, try to find one that supports both
	uint32_t graphicsQueueNodeIndex = UINT32_MAX;
	for (uint32_t i = 0; i < queueCount; i++) {
		if ((queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
			if (supportsPresent[i] == VK_TRUE) {
				graphicsQueueNodeIndex = i;
				break;
			}
		}
	}
	free(supportsPresent);

	// Generate error if could not find a queue that supports both a graphics
	// and present
	if (graphicsQueueNodeIndex == UINT32_MAX) {
		std::cout << "Could not find a queue that supports both graphics and "
			"present\n";
		exit(-1);
	}

	return graphicsQueueNodeIndex;
}

VkSwapchainKHR VulkanRenderer::CreateSwapChain(
	VkPhysicalDevice gpu, VkSurfaceKHR surface, VkFormat &format,
	int width, int height,
	VkDevice device,
	uint32_t &swapChainImageCount, vector<SwapChainBuffer> &buffer, vector<SwapChainBuffer> &buffer2,
	VkCommandBuffer &cmdBuf, VkQueue &queue,
	const uint32_t queueFamilyIndex)
{
	// Get the list of VkFormats that are supported:
	uint32_t formatCount;
	VkResult res =
		vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, NULL);
	assert(res == VK_SUCCESS);

	VkSurfaceFormatKHR *surfFormats =
		(VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
	res = vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface,
		&formatCount, surfFormats);
	assert(res == VK_SUCCESS);

	// If the format list includes just one entry of VK_FORMAT_UNDEFINED,
	// the surface has no preferred format.  Otherwise, at least one
	// supported format will be returned.
	if (formatCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED) {
		format = VK_FORMAT_B8G8R8A8_UNORM;
	}
	else {
		assert(formatCount >= 1);
		format = surfFormats[0].format;
	}
	free(surfFormats);

	VkSurfaceCapabilitiesKHR surfCapabilities;

	res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface,
		&surfCapabilities);
	assert(res == VK_SUCCESS);

	uint32_t presentModeCount;
	res = vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface,
		&presentModeCount, NULL);
	assert(res == VK_SUCCESS);

	VkPresentModeKHR *presentModes =
		(VkPresentModeKHR *)malloc(presentModeCount * sizeof(VkPresentModeKHR));

	res = vkGetPhysicalDeviceSurfacePresentModesKHR(
		gpu, surface, &presentModeCount, presentModes);
	assert(res == VK_SUCCESS);

	VkExtent2D swapChainExtent;
	// width and height are either both -1, or both not -1.
	if (surfCapabilities.currentExtent.width == (uint32_t)-1) {
		// If the surface size is undefined, the size is set to
		// the size of the images requested.
		swapChainExtent.width = width;
		swapChainExtent.height = height;
	}
	else {
		// If the surface size is defined, the swap chain size must match
		swapChainExtent = surfCapabilities.currentExtent;
	}

	// If mailbox mode is available, use it, as is the lowest-latency non-
	// tearing mode.  If not, try IMMEDIATE which will usually be available,
	// and is fastest (though it tears).  If not, fall back to FIFO which is
	// always available.
	VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
	for (size_t i = 0; i < presentModeCount; i++) {
		if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
			swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
		if ((swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) &&
			(presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)) {
			swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
	}

	// Determine the number of VkImage's to use in the swap chain (we desire to
	// own only 1 image at a time, besides the images being displayed and
	// queued for display):
	uint32_t desiredNumberOfSwapChainImages =
		surfCapabilities.minImageCount + 1;
	if ((surfCapabilities.maxImageCount > 0) &&
		(desiredNumberOfSwapChainImages > surfCapabilities.maxImageCount)) {
		// Application must settle for fewer images than desired:
		desiredNumberOfSwapChainImages = surfCapabilities.maxImageCount;
	}

	VkSurfaceTransformFlagBitsKHR preTransform;
	if (surfCapabilities.supportedTransforms &
		VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else {
		preTransform = surfCapabilities.currentTransform;
	}

	VkSwapchainKHR swapChain;
	VkSwapchainCreateInfoKHR swap_chain = {};
	swap_chain.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swap_chain.pNext = NULL;
	swap_chain.surface = surface;
	swap_chain.minImageCount = desiredNumberOfSwapChainImages;
	swap_chain.imageFormat = format;
	swap_chain.imageExtent.width = swapChainExtent.width;
	swap_chain.imageExtent.height = swapChainExtent.height;
	swap_chain.preTransform = preTransform;
	swap_chain.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swap_chain.imageArrayLayers = 1;
	swap_chain.presentMode = swapchainPresentMode;
	swap_chain.oldSwapchain = NULL;
	swap_chain.clipped = true;
	swap_chain.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	swap_chain.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	swap_chain.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swap_chain.queueFamilyIndexCount = 0;
	swap_chain.pQueueFamilyIndices = NULL;

	res =
		vkCreateSwapchainKHR(device, &swap_chain, NULL, &swapChain);
	assert(res == VK_SUCCESS);

	res = vkGetSwapchainImagesKHR(device, swapChain,
		&swapChainImageCount, NULL);
	assert(res == VK_SUCCESS);

	VkImage *swapchainImages =
		(VkImage *)malloc(swapChainImageCount * sizeof(VkImage));
	assert(swapchainImages);
	res = vkGetSwapchainImagesKHR(device, swapChain,
		&swapChainImageCount, swapchainImages);
	assert(res == VK_SUCCESS);

	buffer.resize(swapChainImageCount);
	buffer2.resize(swapChainImageCount);//here

	//Create Images
	vkGetDeviceQueue(device, queueFamilyIndex, 0,
		&queue);

	for (uint32_t i = 0; i < swapChainImageCount; i++) {
		VkImageViewCreateInfo color_image_view = {};
		color_image_view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		color_image_view.pNext = NULL;
		color_image_view.format = format;
		color_image_view.components.r = VK_COMPONENT_SWIZZLE_R;
		color_image_view.components.g = VK_COMPONENT_SWIZZLE_G;
		color_image_view.components.b = VK_COMPONENT_SWIZZLE_B;
		color_image_view.components.a = VK_COMPONENT_SWIZZLE_A;
		color_image_view.subresourceRange.aspectMask =
			VK_IMAGE_ASPECT_COLOR_BIT;
		color_image_view.subresourceRange.baseMipLevel = 0;
		color_image_view.subresourceRange.levelCount = 1;
		color_image_view.subresourceRange.baseArrayLayer = 0;
		color_image_view.subresourceRange.layerCount = 1;
		color_image_view.viewType = VK_IMAGE_VIEW_TYPE_2D;
		color_image_view.flags = 0;

		buffer[i].image = swapchainImages[i];

		vHelper::SetImageLayout(cmdBuf, queue, buffer[i].image, VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		color_image_view.image = buffer[i].image;

		res = vkCreateImageView(device, &color_image_view, NULL,
			&buffer[i].view);
		assert(res == VK_SUCCESS);

		//buffer2
		buffer2[i].image = CreateEmptyImage(width, height, format);

		/*SetImageLayout(cmdBuf, queue, buffer2[i].image, VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);*/

		color_image_view.image = buffer2[i].image;

		res = vkCreateImageView(device, &color_image_view, NULL,
			&buffer2[i].view);
		assert(res == VK_SUCCESS);
	}

	return swapChain;
}

DepthMap VulkanRenderer::CreateDepthMap(
	VkPhysicalDevice gpu, VkDevice device, int width, int height,
	VkPhysicalDeviceMemoryProperties &memory_properties, VkCommandBuffer &cmdBuf,
	VkQueue &queue)
{
	VkResult res;
	DepthMap depth;

	VkImageCreateInfo image_info = {};
	const VkFormat depth_format = VK_FORMAT_D16_UNORM;
	VkFormatProperties props;
	vkGetPhysicalDeviceFormatProperties(gpu, depth_format, &props);
	if (props.linearTilingFeatures &
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
		image_info.tiling = VK_IMAGE_TILING_LINEAR;
	}
	else if (props.optimalTilingFeatures &
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
		image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	}
	else {
		/* Try other depth formats? */
		std::cout << "VK_FORMAT_D16_UNORM Unsupported.\n";
		exit(-1);
	}

	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.pNext = NULL;
	image_info.imageType = VK_IMAGE_TYPE_2D;
	image_info.format = depth_format;
	image_info.extent.width = width;
	image_info.extent.height = height;
	image_info.extent.depth = 1;
	image_info.mipLevels = 1;
	image_info.arrayLayers = 1;
	image_info.samples = NUM_SAMPLES;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	image_info.queueFamilyIndexCount = 0;
	image_info.pQueueFamilyIndices = NULL;
	image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_info.flags = 0;

	VkMemoryAllocateInfo mem_alloc = {};
	mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mem_alloc.pNext = NULL;
	mem_alloc.allocationSize = 0;
	mem_alloc.memoryTypeIndex = 0;

	VkImageViewCreateInfo view_info = {};
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.pNext = NULL;
	view_info.image = VK_NULL_HANDLE;
	view_info.format = depth_format;
	view_info.components.r = VK_COMPONENT_SWIZZLE_R;
	view_info.components.g = VK_COMPONENT_SWIZZLE_G;
	view_info.components.b = VK_COMPONENT_SWIZZLE_B;
	view_info.components.a = VK_COMPONENT_SWIZZLE_A;
	view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.levelCount = 1;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = 1;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.flags = 0;

	VkMemoryRequirements mem_reqs;

	depth.format = depth_format;

	/* Create image */
	res = vkCreateImage(device, &image_info, NULL, &depth.image);
	assert(res == VK_SUCCESS);

	vkGetImageMemoryRequirements(device, depth.image, &mem_reqs);

	mem_alloc.allocationSize = mem_reqs.size;
	/* Use the memory properties to determine the type of memory required */
	bool pass = vHelper::MemoryTypeFromProperties(memory_properties, mem_reqs.memoryTypeBits,
		0, /* No Requirements */
		&mem_alloc.memoryTypeIndex);
	assert(pass);

	/* Allocate memory */
	res = vkAllocateMemory(device, &mem_alloc, NULL, &depth.memory);
	assert(res == VK_SUCCESS);

	/* Bind memory */
	res = vkBindImageMemory(device, depth.image, depth.memory, 0);
	assert(res == VK_SUCCESS);

	/* Set the image layout to depth stencil optimal */
	vHelper::SetImageLayout(cmdBuf, queue, depth.image, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	/* Create image view */
	view_info.image = depth.image;
	res = vkCreateImageView(device, &view_info, NULL, &depth.view);
	assert(res == VK_SUCCESS);

	return depth;
}

VkRenderPass VulkanRenderer::CreateRenderPass(
	VkFormat imgFormat, DepthMap &depth, VkDevice device)
{
	VkRenderPass render_pass;

	VkResult res;
	/* Need attachments for render target and depth buffer */
	VkAttachmentDescription attachments[3];
	attachments[0].format = imgFormat;
	attachments[0].samples = NUM_SAMPLES;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	//clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachments[0].flags = 0;

	attachments[1].format = imgFormat;
	attachments[1].samples = NUM_SAMPLES;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	//clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachments[1].flags = 0;

	attachments[2].format = depth.format;
	attachments[2].samples = NUM_SAMPLES;
	attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	//clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[2].initialLayout =
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachments[2].finalLayout =
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachments[2].flags = 0;

	VkAttachmentReference color_reference[2];
	color_reference[0].attachment = 0;
	color_reference[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	color_reference[1].attachment = 1;
	color_reference[1].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_reference = {};
	depth_reference.attachment = 2;
	depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.flags = 0;
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = NULL;
	subpass.colorAttachmentCount = 2;
	subpass.pColorAttachments = color_reference;
	subpass.pResolveAttachments = NULL;
	subpass.pDepthStencilAttachment = &depth_reference;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = NULL;

	VkRenderPassCreateInfo rp_info = {};
	rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rp_info.pNext = NULL;
	rp_info.attachmentCount = 3;
	rp_info.pAttachments = attachments;
	rp_info.subpassCount = 1;
	rp_info.pSubpasses = &subpass;
	rp_info.dependencyCount = 0;
	rp_info.pDependencies = NULL;

	res = vkCreateRenderPass(device, &rp_info, NULL, &render_pass);
	assert(res == VK_SUCCESS);

	return render_pass;
}

VkFramebuffer* VulkanRenderer::CreateFrameBuffers(
	VkDevice device,
	DepthMap &depth, SwapChainBuffer *buffers, SwapChainBuffer *buffers2,
	VkRenderPass &render_pass, int width, int height,
	int swapchainImageCount)
{
	VkFramebuffer *framebuffers;

	VkResult res;
	VkImageView attachments[3];
	attachments[2] = depth.view;

	VkFramebufferCreateInfo fb_info = {};
	fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fb_info.pNext = NULL;
	fb_info.renderPass = render_pass;
	fb_info.attachmentCount = 3;
	fb_info.pAttachments = attachments;
	fb_info.width = width;
	fb_info.height = height;
	fb_info.layers = 1;

	uint32_t i;

	framebuffers = (VkFramebuffer *)malloc(swapchainImageCount *
		sizeof(VkFramebuffer));

	for (i = 0; i < swapchainImageCount; i++) {
		attachments[0] = buffers[i].view;
		attachments[1] = buffers2[i].view;
		res = vkCreateFramebuffer(device, &fb_info, NULL,
			&framebuffers[i]);
		assert(res == VK_SUCCESS);
	}

	return framebuffers;
}

VkDescriptorPool VulkanRenderer::CreateDescriptorPool(VkDevice device)
{
	VkDescriptorPool descriptorPool;

	VkResult res;
	VkDescriptorPoolSize type_count[2];
	type_count[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	type_count[0].descriptorCount = 1;

	type_count[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	type_count[1].descriptorCount = 5;

	VkDescriptorPoolCreateInfo descriptor_pool = {};
	descriptor_pool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptor_pool.pNext = NULL;
	descriptor_pool.maxSets = 16;
	descriptor_pool.poolSizeCount = 2;
	descriptor_pool.pPoolSizes = type_count;

	res = vkCreateDescriptorPool(device, &descriptor_pool, NULL,
		&descriptorPool);
	assert(res == VK_SUCCESS);

	return descriptorPool;
}

void VulkanRenderer::initViewports(int width, int height, VkCommandBuffer cmdBuf, VkViewport &viewport)
{
	viewport.height = (float)height;
	viewport.width = (float)width;
	viewport.minDepth = (float)0.0f;
	viewport.maxDepth = (float)1.0f;
	viewport.x = 0;
	viewport.y = 0;
	vkCmdSetViewport(cmdBuf, 0, NUM_VIEWPORTS, &viewport);
}

void VulkanRenderer::initScissors(int width, int height, VkCommandBuffer cmdBuf, VkRect2D &scissor)
{
	scissor.extent.width = width;
	scissor.extent.height = height;
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	vkCmdSetScissor(cmdBuf, 0, NUM_SCISSORS, &scissor);
}

VkImage VulkanRenderer::CreateEmptyImage(uint32_t width, uint32_t height, VkFormat format)
{
	VkFormatProperties formatProps;
	vkGetPhysicalDeviceFormatProperties(m_GPUs[0], VK_FORMAT_R8G8B8A8_UNORM,
		&formatProps);

	/* See if we can use a linear tiled image for a texture, if not, we will
	* need a staging image for the texture data */
	bool needStaging = (!(formatProps.linearTilingFeatures &
		VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))
		? true
		: false;

	VkImageCreateInfo image_create_info = {};
	image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_create_info.pNext = NULL;
	image_create_info.imageType = VK_IMAGE_TYPE_2D;
	image_create_info.format = format;
	image_create_info.extent.width = width;
	image_create_info.extent.height = height;
	image_create_info.extent.depth = 1;
	image_create_info.mipLevels = 1;
	image_create_info.arrayLayers = 1;
	image_create_info.samples = NUM_SAMPLES;
	image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;// | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	image_create_info.queueFamilyIndexCount = 0;
	image_create_info.pQueueFamilyIndices = NULL;
	image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_create_info.flags = 0;

	VkMemoryAllocateInfo mem_alloc = {};
	mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mem_alloc.pNext = NULL;
	mem_alloc.allocationSize = 0;
	mem_alloc.memoryTypeIndex = 0;

	VkImage mappableImage;
	VkDeviceMemory mappableMemory;

	VkMemoryRequirements mem_reqs;

	/* Create a mappable image.  It will be the texture if linear images are ok
	* to be textures or it will be the staging image if they are not. */
	VkResult res = vkCreateImage(m_device, &image_create_info, NULL, &mappableImage);
	assert(res == VK_SUCCESS);

	vkGetImageMemoryRequirements(m_device, mappableImage, &mem_reqs);
	assert(res == VK_SUCCESS);

	mem_alloc.allocationSize = mem_reqs.size;

	/* Find the memory type that is host mappable */
	bool pass = vHelper::MemoryTypeFromProperties(m_memoryProperties, mem_reqs.memoryTypeBits,
		0,
		&mem_alloc.memoryTypeIndex);
	assert(pass && "No mappable, coherent memory");

	/* allocate memory */
	res = vkAllocateMemory(m_device, &mem_alloc, NULL, &(mappableMemory));
	assert(res == VK_SUCCESS);

	/* bind memory */
	res = vkBindImageMemory(m_device, mappableImage, mappableMemory, 0);
	assert(res == VK_SUCCESS);

	vHelper::SetImageLayout(m_cmdBuffer, m_queue, mappableImage, VK_IMAGE_ASPECT_COLOR_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	return mappableImage;
}

void VulkanRenderer::render(
	DescPipelineLayout layout,
	RenderPass pass, RenderPass passDOF, vector<Renderable> renderableList)
{
	BeginCommandBuffer(m_cmdBuffer);

	VkResult res;

	VkClearValue clear_values[4];
	clear_values[0].color.float32[0] = 0.1f;
	clear_values[0].color.float32[1] = 0.1f;
	clear_values[0].color.float32[2] = 0.1f;
	clear_values[0].color.float32[3] = 1.0f;
	clear_values[1].color.float32[0] = 0.1f;
	clear_values[1].color.float32[1] = 0.1f;
	clear_values[1].color.float32[2] = 0.1f;
	clear_values[1].color.float32[3] = 1.0f;
	clear_values[2].color.float32[0] = 0.1f;
	clear_values[2].color.float32[1] = 0.1f;
	clear_values[2].color.float32[2] = 0.1f;
	clear_values[2].color.float32[3] = 1.0f;
	clear_values[3].depthStencil.depth = 1.0f;
	clear_values[3].depthStencil.stencil = 0;
	clear_values[4].color.float32[0] = 0.1f;
	clear_values[4].color.float32[1] = 0.1f;
	clear_values[4].color.float32[2] = 0.1f;
	clear_values[4].color.float32[3] = 1.0f;

	VkSemaphore presentCompleteSemaphore;
	VkSemaphoreCreateInfo presentCompleteSemaphoreCreateInfo;
	presentCompleteSemaphoreCreateInfo.sType =
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	presentCompleteSemaphoreCreateInfo.pNext = NULL;
	presentCompleteSemaphoreCreateInfo.flags = 0;

	res = vkCreateSemaphore(m_device, &presentCompleteSemaphoreCreateInfo,
		NULL, &presentCompleteSemaphore);
	assert(res == VK_SUCCESS);

	// Get the index of the next available swapchain image:
	res = vkAcquireNextImageKHR(m_device, m_swapChain, UINT64_MAX,
		presentCompleteSemaphore, VK_NULL_HANDLE,
		&m_currentBuffer);
	// TODO: Deal with the VK_SUBOPTIMAL_KHR and VK_ERROR_OUT_OF_DATE_KHR
	// return codes
	assert(res == VK_SUCCESS);

	vHelper::SetImageLayout(m_cmdBuffer, m_queue, m_swapChainImgBuffer[m_currentBuffer].image,
		VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED,//¥Ê“…
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	//vHelper::SetImageLayout(m_cmdBuffer, m_queue, m_swapChainImgBuffer2[m_currentBuffer].image,
	//	VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
	//	VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	VkRenderPassBeginInfo rp_begin;
	rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rp_begin.pNext = NULL;
	rp_begin.renderPass = pass.renderPass;
	rp_begin.framebuffer = pass.framebuffers[0];
	rp_begin.renderArea.offset.x = 0;
	rp_begin.renderArea.offset.y = 0;
	rp_begin.renderArea.extent.width = width;
	rp_begin.renderArea.extent.height = height;
	rp_begin.clearValueCount = 5;
	rp_begin.pClearValues = clear_values;

	vkCmdBeginRenderPass(m_cmdBuffer, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

	initViewports(width, height, m_cmdBuffer, m_viewport);
	initScissors(width, height, m_cmdBuffer, m_scissor);

	int i = 0;
	vkCmdBindPipeline(m_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pass.pipelines[0].pipelineObj);

	const VkDeviceSize offsets[1] = { 0 };

	for (int i = 0; i < renderableList.size(); i++)
	{
		vkCmdBindDescriptorSets(m_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			layout.pipelineLayout, 0, renderableList[i].descSet.descSetCount,
			renderableList[i].descSet.data(), 0, NULL);

		vkCmdBindVertexBuffers(m_cmdBuffer, 0, 1, &renderableList[i].vertexBuffer.buf, offsets);
		vkCmdBindIndexBuffer(m_cmdBuffer, renderableList[i].indexBuffer.buf, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(m_cmdBuffer, renderableList[i].indicesCount, 1, 0, 0, 0);
	}

	vkCmdNextSubpass(m_cmdBuffer, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(m_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pass.pipelines[1].pipelineObj);

	//draw a screen-aligned quad
	vkCmdBindDescriptorSets(m_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		layoutIA.pipelineLayout, 0, screenAlignedQuad.descSet.descSetCount,
		screenAlignedQuad.descSet.data(), 0, NULL);

	vkCmdBindVertexBuffers(m_cmdBuffer, 0, 1, &screenAlignedQuad.vertexBuffer.buf, offsets);
	vkCmdBindIndexBuffer(m_cmdBuffer, screenAlignedQuad.indexBuffer.buf, 0, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(m_cmdBuffer, 6, 1, 0, 0, 0);

	vkCmdEndRenderPass(m_cmdBuffer);

	rp_begin.renderPass = passDOF.renderPass;
	rp_begin.framebuffer = passDOF.framebuffers[m_currentBuffer];
	rp_begin.clearValueCount = 1;
	
	vkCmdBeginRenderPass(m_cmdBuffer, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(m_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, passDOF.pipelines[0].pipelineObj);
	vkCmdBindDescriptorSets(m_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		layoutIA.pipelineLayout, 0, screenAlignedQuad.descSet.descSetCount,
		screenAlignedQuad.descSet.data(), 0, NULL);

	vkCmdBindVertexBuffers(m_cmdBuffer, 0, 1, &screenAlignedQuad.vertexBuffer.buf, offsets);
	vkCmdBindIndexBuffer(m_cmdBuffer, screenAlignedQuad.indexBuffer.buf, 0, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(m_cmdBuffer, 6, 1, 0, 0, 0);

	vkCmdEndRenderPass(m_cmdBuffer);

	VkImageMemoryBarrier prePresentBarrier = {};
	prePresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	prePresentBarrier.pNext = NULL;
	prePresentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	prePresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	prePresentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	prePresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	prePresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	prePresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	prePresentBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	prePresentBarrier.subresourceRange.baseMipLevel = 0;
	prePresentBarrier.subresourceRange.levelCount = 1;
	prePresentBarrier.subresourceRange.baseArrayLayer = 0;
	prePresentBarrier.subresourceRange.layerCount = 1;
	prePresentBarrier.image = m_swapChainImgBuffer[m_currentBuffer].image;
	vkCmdPipelineBarrier(m_cmdBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL, 0,
		NULL, 1, &prePresentBarrier);

	res = vkEndCommandBuffer(m_cmdBuffer);
	const VkCommandBuffer cmd_bufs[] = { m_cmdBuffer };
	VkFenceCreateInfo fenceInfo;
	VkFence drawFence;
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.pNext = NULL;
	fenceInfo.flags = 0;
	vkCreateFence(m_device, &fenceInfo, NULL, &drawFence);

	VkPipelineStageFlags pipe_stage_flags =
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	VkSubmitInfo submit_info[1] = {};
	submit_info[0].pNext = NULL;
	submit_info[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info[0].waitSemaphoreCount = 1;
	submit_info[0].pWaitSemaphores = &presentCompleteSemaphore;
	submit_info[0].pWaitDstStageMask = &pipe_stage_flags;
	submit_info[0].commandBufferCount = 1;
	submit_info[0].pCommandBuffers = cmd_bufs;
	submit_info[0].signalSemaphoreCount = 0;
	submit_info[0].pSignalSemaphores = NULL;

	/* Queue the command buffer for execution */
	res = vkQueueSubmit(m_queue, 1, submit_info, drawFence);
	assert(res == VK_SUCCESS);

	/* Now present the image in the window */

	VkPresentInfoKHR present;
	present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present.pNext = NULL;
	present.swapchainCount = 1;
	present.pSwapchains = &m_swapChain;
	present.pImageIndices = &m_currentBuffer;
	present.pWaitSemaphores = NULL;
	present.waitSemaphoreCount = 0;
	present.pResults = NULL;

	/* Make sure command buffer is finished before presenting */
	do {
		res =
			vkWaitForFences(m_device, 1, &drawFence, VK_TRUE, fenceTimeout);
	} while (res == VK_TIMEOUT);

	assert(res == VK_SUCCESS);
	res = vkQueuePresentKHR(m_queue, &present);
	assert(res == VK_SUCCESS);
}

void VulkanRenderer::renderByChain(vector<Renderable> renderableList)
{
	BeginCommandBuffer(m_cmdBuffer);

	VkResult res;
	int i, j;

	VkSemaphore presentCompleteSemaphore;
	VkSemaphoreCreateInfo presentCompleteSemaphoreCreateInfo;
	presentCompleteSemaphoreCreateInfo.sType =
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	presentCompleteSemaphoreCreateInfo.pNext = NULL;
	presentCompleteSemaphoreCreateInfo.flags = 0;

	res = vkCreateSemaphore(m_device, &presentCompleteSemaphoreCreateInfo,
		NULL, &presentCompleteSemaphore);
	assert(res == VK_SUCCESS);

	// Get the index of the next available swapchain image:
	res = vkAcquireNextImageKHR(m_device, m_swapChain, UINT64_MAX,
		presentCompleteSemaphore, VK_NULL_HANDLE,
		&m_currentBuffer);
	// TODO: Deal with the VK_SUBOPTIMAL_KHR and VK_ERROR_OUT_OF_DATE_KHR
	// return codes
	assert(res == VK_SUCCESS);

	vHelper::SetImageLayout(m_cmdBuffer, m_queue, m_swapChainImgBuffer[m_currentBuffer].image,
		VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED,//¥Ê“…
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	VkRenderPassBeginInfo rp_begin;
	rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rp_begin.pNext = NULL;
	rp_begin.renderArea.offset.x = 0;
	rp_begin.renderArea.offset.y = 0;
	rp_begin.renderArea.extent.width = width;
	rp_begin.renderArea.extent.height = height;

	for (i = 0;i < unitCount; i++)
	{
		VkClearValue clear_values[10];

		for (j = 0;j < unitChain[i].oCount; j++)
		{
			clear_values[j].color.float32[0] = 0.1f;
			clear_values[j].color.float32[1] = 0.1f;
			clear_values[j].color.float32[2] = 0.1f;
			clear_values[j].color.float32[3] = 1.0f;
		}

		if (unitChain[i].hasDepth)
		{
			clear_values[unitChain[i].oCount].depthStencil.depth = 1.0f;
			clear_values[unitChain[i].oCount].depthStencil.stencil = 0;
		}

		rp_begin.pClearValues = clear_values;
		rp_begin.clearValueCount = unitChain[i].clearCount;

		rp_begin.renderPass = unitChain[i].renderPass;
		
		if (unitChain[i].asLast == true)
		{
			rp_begin.framebuffer = unitChain[i].framebuffers[m_currentBuffer];
		}
		else
		{
			rp_begin.framebuffer = unitChain[i].framebuffers[0];
		}

		vkCmdBeginRenderPass(m_cmdBuffer, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(m_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, unitChain[i].pipeline.pipelineObj);

		vkCmdBindDescriptorSets(m_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			unitChain[i].layout.pipelineLayout, 2, unitChain[i].descSet.descSetCount,
			unitChain[i].descSet.data(), 0, NULL);

		vkCmdBindDescriptorSets(m_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			unitChain[i].layout.pipelineLayout, 1, screenAlignedQuad.descSet.descSetCount,
			screenAlignedQuad.descSet.data(), 0, NULL);

		vkCmdBindDescriptorSets(m_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			unitChain[i].layout.pipelineLayout, 0, screenAlignedQuad.descSet.descSetCount,
			screenAlignedQuad.descSet.data(), 0, NULL);

		initViewports(width, height, m_cmdBuffer, m_viewport);
		initScissors(width, height, m_cmdBuffer, m_scissor);

		const VkDeviceSize offsets[1] = { 0 };

		//if (unitChain[i].drawScreenQuad == true)
		//{
		//	//draw a screen-aligned quad
		//	vkCmdBindDescriptorSets(m_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		//		unitChain[i].layout.pipelineLayout, 0, screenAlignedQuad.descSet.descSetCount,
		//		screenAlignedQuad.descSet.data(), 0, NULL);

		//	vkCmdBindVertexBuffers(m_cmdBuffer, 0, 1, &screenAlignedQuad.vertexBuffer.buf, offsets);
		//	vkCmdBindIndexBuffer(m_cmdBuffer, screenAlignedQuad.indexBuffer.buf, 0, VK_INDEX_TYPE_UINT32);
		//	vkCmdDrawIndexed(m_cmdBuffer, 6, 1, 0, 0, 0);
		//}
		//else
		//{
		//	for (j = 0; j < renderableList.size(); j++)
		//	{
		//		vkCmdBindDescriptorSets(m_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		//			unitChain[i].layout.pipelineLayout, 0, renderableList[i].descSet.descSetCount,
		//			renderableList[i].descSet.data(), 0, NULL);

		//		vkCmdBindVertexBuffers(m_cmdBuffer, 0, 1, &renderableList[i].vertexBuffer.buf, offsets);
		//		vkCmdBindIndexBuffer(m_cmdBuffer, renderableList[i].indexBuffer.buf, 0, VK_INDEX_TYPE_UINT32);
		//		vkCmdDrawIndexed(m_cmdBuffer, renderableList[i].indicesCount, 1, 0, 0, 0);
		//	}
		//}

		vkCmdEndRenderPass(m_cmdBuffer);
	}

	VkImageMemoryBarrier prePresentBarrier = {};
	prePresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	prePresentBarrier.pNext = NULL;
	prePresentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	prePresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	prePresentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	prePresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	prePresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	prePresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	prePresentBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	prePresentBarrier.subresourceRange.baseMipLevel = 0;
	prePresentBarrier.subresourceRange.levelCount = 1;
	prePresentBarrier.subresourceRange.baseArrayLayer = 0;
	prePresentBarrier.subresourceRange.layerCount = 1;
	prePresentBarrier.image = m_swapChainImgBuffer[m_currentBuffer].image;
	vkCmdPipelineBarrier(m_cmdBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL, 0,
		NULL, 1, &prePresentBarrier);

	res = vkEndCommandBuffer(m_cmdBuffer);
	const VkCommandBuffer cmd_bufs[] = { m_cmdBuffer };
	VkFenceCreateInfo fenceInfo;
	VkFence drawFence;
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.pNext = NULL;
	fenceInfo.flags = 0;
	vkCreateFence(m_device, &fenceInfo, NULL, &drawFence);

	VkPipelineStageFlags pipe_stage_flags =
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	VkSubmitInfo submit_info[1] = {};
	submit_info[0].pNext = NULL;
	submit_info[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info[0].waitSemaphoreCount = 1;
	submit_info[0].pWaitSemaphores = &presentCompleteSemaphore;
	submit_info[0].pWaitDstStageMask = &pipe_stage_flags;
	submit_info[0].commandBufferCount = 1;
	submit_info[0].pCommandBuffers = cmd_bufs;
	submit_info[0].signalSemaphoreCount = 0;
	submit_info[0].pSignalSemaphores = NULL;

	/* Queue the command buffer for execution */
	res = vkQueueSubmit(m_queue, 1, submit_info, drawFence);
	assert(res == VK_SUCCESS);

	/* Now present the image in the window */

	VkPresentInfoKHR present;
	present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present.pNext = NULL;
	present.swapchainCount = 1;
	present.pSwapchains = &m_swapChain;
	present.pImageIndices = &m_currentBuffer;
	present.pWaitSemaphores = NULL;
	present.waitSemaphoreCount = 0;
	present.pResults = NULL;

	/* Make sure command buffer is finished before presenting */
	do {
		res =
			vkWaitForFences(m_device, 1, &drawFence, VK_TRUE, fenceTimeout);
	} while (res == VK_TIMEOUT);

	assert(res == VK_SUCCESS);
	res = vkQueuePresentKHR(m_queue, &present);
	assert(res == VK_SUCCESS);
}
