#define WIN32_LEAN_AND_MEAN
#define VK_USE_PLATFORM_WIN32_KHR
#define NOMINMAX /* Don't let Windows define min() or max() */

/* Number of samples needs to be the same at image creation,      */
/* renderpass creation and pipeline creation.                     */
#define NUM_SAMPLES VK_SAMPLE_COUNT_1_BIT

#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <cassert>
#include <vulkan/vulkan.h>
#include <vulkan/vk_sdk_platform.h>

#include <Windows.h>

using namespace std;

/*
	Structs
*/

typedef struct _swap_chain_buffers {
	VkImage image;
	VkImageView view;
} SwapChainBuffer;

typedef struct _depth {
	VkFormat format;

	VkImage image;
	VkImageView view;
	VkDeviceMemory memory;
} DepthMap;


/*
	Globals
*/

VkInstance m_instance;
vector<VkPhysicalDevice> m_GPUs;

uint32_t m_queueCount;
uint32_t m_queueFamilyIndex;//index of the queue founded
vector<VkQueueFamilyProperties> m_queueProps;

VkPhysicalDeviceProperties m_GPUProperties;
VkPhysicalDeviceMemoryProperties m_memoryProperties;

VkDevice m_device;

VkCommandPool m_cmdPool;
VkCommandBuffer m_cmdBuffer;

VkSurfaceKHR m_surface;
uint32_t m_queueGraphicsPresentIndex;
VkQueue m_queue;

VkSwapchainKHR m_swapChain;
VkFormat m_colorImgFormat;
uint32_t m_swapChainImageCount;
vector<SwapChainBuffer> m_swapChainImgBuffer;

DepthMap m_depthMap;

///win32
HINSTANCE m_connection;
HWND m_window;

///const
const int fenceTimeout = 100000000;
const int width = 800;
const int height = 600;

/*
	Win32 window
*/
static void run()
{
	/* Placeholder for samples that want to show dynamic content */
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	case WM_PAINT:
		run();
		return 0;
	default:
		break;
	}

	return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

void InitWindow(int width, int height, string windowTitle)
{
	//char title[100];
	//sprintf(title, "%s", windowTitle.c_str());

	WNDCLASSEX win_class;
	assert(width > 0);
	assert(height > 0);

	m_connection = GetModuleHandle(NULL);

	// Initialize the window class structure:
	win_class.cbSize = sizeof(WNDCLASSEX);
	win_class.style = CS_HREDRAW | CS_VREDRAW;
	win_class.lpfnWndProc = WndProc;
	win_class.cbClsExtra = 0;
	win_class.cbWndExtra = 0;
	win_class.hInstance = m_connection; // hInstance
	win_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	win_class.hCursor = LoadCursor(NULL, IDC_ARROW);
	win_class.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	win_class.lpszMenuName = NULL;
	win_class.lpszClassName = windowTitle.c_str();
	win_class.hIconSm = LoadIcon(NULL, IDI_WINLOGO);
	// Register window class:
	if (!RegisterClassEx(&win_class)) {
		// It didn't work, so try to give a useful error:
		printf("Unexpected error trying to start the application!\n");
		fflush(stdout);
		exit(1);
	}
	// Create window with the registered class:
	RECT wr = { 0, 0, width, height };
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
	m_window = CreateWindowEx(0,
		windowTitle.c_str(),					// class name
		windowTitle.c_str(),					// app name
		WS_OVERLAPPEDWINDOW |					// window style
		WS_VISIBLE | WS_SYSMENU,
		100, 100,								// x/y coords
		wr.right - wr.left,						// width
		wr.bottom - wr.top,						// height
		NULL,									// handle to parent
		NULL,									// handle to menu
		m_connection,							// hInstance
		NULL);									// no extra parameters
	if (!m_window) {
		// It didn't work, so try to give a useful error:
		printf("Cannot create a window in which to draw!\n");
		fflush(stdout);
		exit(1);
	}
	//SetWindowLongPtr(m_window, GWLP_USERDATA, (LONG_PTR)&info);
}


/*
	ToolBoxes
*/

void BeginCommandBuffer(VkCommandBuffer &cmdBuf)
{
	VkCommandBufferBeginInfo cmd_buf_info = {};
	cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmd_buf_info.pNext = NULL;
	cmd_buf_info.flags = 0;
	cmd_buf_info.pInheritanceInfo = NULL;

	VkResult res = vkBeginCommandBuffer(cmdBuf, &cmd_buf_info);
	assert(res == VK_SUCCESS);
}

void EndCommandBuffer(VkCommandBuffer &cmdBuf)
{
	VkResult res = vkEndCommandBuffer(cmdBuf);
	assert(res == VK_SUCCESS);
}

void QueueCommandBuffer(VkCommandBuffer &cmdBuf, VkDevice &device, VkQueue &queue)
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

bool MemoryTypeFromProperties(VkPhysicalDeviceMemoryProperties memory_properties, uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex)
{
	// Search memtypes to find first index with those properties
	for (uint32_t i = 0; i < 32; i++) {
		if ((typeBits & 1) == 1) {
			// Type is available, does it match user properties?
			if ((memory_properties.memoryTypes[i].propertyFlags &
				requirements_mask) == requirements_mask) {
				*typeIndex = i;
				return true;
			}
		}
		typeBits >>= 1;
	}
	// No memory types matched, return failure
	return false;
}

void SetImageLayout(VkCommandBuffer &cmdBuf, VkQueue &queue, VkImage image,
	VkImageAspectFlags aspectMask,
	VkImageLayout old_image_layout,
	VkImageLayout new_image_layout) {
	/* DEPENDS on info.cmd and info.queue initialized */

	assert(cmdBuf != VK_NULL_HANDLE);
	assert(queue != VK_NULL_HANDLE);

	VkImageMemoryBarrier image_memory_barrier = {};
	image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	image_memory_barrier.pNext = NULL;
	image_memory_barrier.srcAccessMask = 0;
	image_memory_barrier.dstAccessMask = 0;
	image_memory_barrier.oldLayout = old_image_layout;
	image_memory_barrier.newLayout = new_image_layout;
	image_memory_barrier.image = image;
	image_memory_barrier.subresourceRange.aspectMask = aspectMask;
	image_memory_barrier.subresourceRange.baseMipLevel = 0;
	image_memory_barrier.subresourceRange.levelCount = 1;
	image_memory_barrier.subresourceRange.layerCount = 1;

	if (old_image_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		image_memory_barrier.srcAccessMask =
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}

	if (new_image_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		/* Make sure anything that was copying from this image has completed */
		image_memory_barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	}

	if (new_image_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		/* Make sure any Copy or CPU writes to image are flushed */
		image_memory_barrier.srcAccessMask =
			VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	}

	if (new_image_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		image_memory_barrier.dstAccessMask =
			VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
	}

	if (new_image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		image_memory_barrier.dstAccessMask =
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
	}

	VkPipelineStageFlags src_stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkPipelineStageFlags dest_stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	vkCmdPipelineBarrier(cmdBuf, src_stages, dest_stages, 0, 0, NULL, 0, NULL,
		1, &image_memory_barrier);
}


/*
	Steps
*/

VkInstance CreateInstance(string instanceName)
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

void EnumeratePhysicalDevices(VkInstance &inst, vector<VkPhysicalDevice> &gpus, VkPhysicalDeviceMemoryProperties &memory_properties, VkPhysicalDeviceProperties &gpu_props)
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

VkDevice CreateDevice(
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

	VkDeviceCreateInfo device_info = {};
	device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_info.pNext = NULL;
	device_info.queueCreateInfoCount = 1;
	device_info.pQueueCreateInfos = &queue_info;
	device_info.enabledExtensionCount = 0;
	device_info.ppEnabledExtensionNames = NULL;
	device_info.enabledLayerCount = 0;
	device_info.ppEnabledLayerNames = NULL;
	device_info.pEnabledFeatures = NULL;

	VkDevice device;
	VkResult res =
		vkCreateDevice(gpus[0], &device_info, NULL, &device);
	assert(res == VK_SUCCESS);

	return device;
}

VkCommandPool CreateCommandPool(const uint32_t queueFamilyIndex, VkDevice &device)
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

VkCommandBuffer CreateCommandBuffer(VkCommandPool &cmdPool, VkDevice &device)
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

VkSurfaceKHR GetSurface(HINSTANCE hinstance, HWND window, VkInstance &instance)
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

uint32_t GetGraphicsPresentQueueIndex(
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

VkSwapchainKHR CreateSwapChain(
	VkPhysicalDevice gpu, VkSurfaceKHR surface, VkFormat &format,
	int width, int height,
	VkDevice device,
	uint32_t &swapChainImageCount, vector<SwapChainBuffer> &buffer,
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
	swap_chain.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
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

	//Create Images
	BeginCommandBuffer(cmdBuf);
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

		SetImageLayout(cmdBuf, queue, buffer[i].image, VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		color_image_view.image = buffer[i].image;

		res = vkCreateImageView(device, &color_image_view, NULL,
			&buffer[i].view);
		assert(res == VK_SUCCESS);
	}

	EndCommandBuffer(cmdBuf);
	QueueCommandBuffer(cmdBuf, device, queue);

	return swapChain;
}

DepthMap CreateDepthMap(
	VkPhysicalDevice gpu, VkDevice device, int width, int height,
	VkPhysicalDeviceMemoryProperties &memory_properties, VkCommandBuffer &cmdBuf,
	VkQueue &queue)
{
	VkResult res;
	DepthMap depth;

	BeginCommandBuffer(cmdBuf);

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
	view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
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
	bool pass = MemoryTypeFromProperties(memory_properties, mem_reqs.memoryTypeBits,
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
	SetImageLayout(cmdBuf, queue, depth.image, VK_IMAGE_ASPECT_DEPTH_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	/* Create image view */
	view_info.image = depth.image;
	res = vkCreateImageView(device, &view_info, NULL, &depth.view);
	assert(res == VK_SUCCESS);

	EndCommandBuffer(cmdBuf);
	QueueCommandBuffer(cmdBuf, device, queue);

	return depth;
}

/*
	Main
*/

int main(int argc, char *argv[])
{
	InitWindow(width, height, "ColoredCube");

	//Create Instance
	m_instance = CreateInstance("Colored Cube");

	//Enumerate & Create device
	EnumeratePhysicalDevices(m_instance, m_GPUs, m_memoryProperties, m_GPUProperties);
	m_device = CreateDevice(m_GPUs, m_queueCount, m_queueProps, m_queueFamilyIndex);

	//Init command buffer
	m_cmdPool = CreateCommandPool(m_queueFamilyIndex, m_device);
	m_cmdBuffer = CreateCommandBuffer(m_cmdPool, m_device);

	//Init swapChain
	m_surface = GetSurface(m_connection, m_window, m_instance);
	m_queueGraphicsPresentIndex = GetGraphicsPresentQueueIndex(m_queueCount, m_GPUs[0], m_queueProps, m_surface);
	m_swapChain = CreateSwapChain(
		m_GPUs[0], m_surface, m_colorImgFormat, width, height, m_device,
		m_swapChainImageCount, m_swapChainImgBuffer, m_cmdBuffer, m_queue, m_queueFamilyIndex);

	//Create DepthBuffer
	m_depthMap = CreateDepthMap(m_GPUs[0], m_device, width, height, m_memoryProperties, m_cmdBuffer, m_queue);

	//todo...
}
