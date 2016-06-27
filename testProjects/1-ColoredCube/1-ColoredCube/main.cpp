#define WIN32_LEAN_AND_MEAN
#define VK_USE_PLATFORM_WIN32_KHR
#define NOMINMAX /* Don't let Windows define min() or max() */

/* Number of samples needs to be the same at image creation,      */
/* renderpass creation and pipeline creation.                     */
#define NUM_SAMPLES VK_SAMPLE_COUNT_1_BIT

/* Number of descriptor sets needs to be the same at alloc,       */
/* pipeline layout creation, and descriptor set layout creation   */
#define NUM_DESCRIPTOR_SETS 1

/* Number of samples needs to be the same at image creation,      */
/* renderpass creation and pipeline creation.                     */
#define NUM_SAMPLES VK_SAMPLE_COUNT_1_BIT

/* Number of viewports and number of scissors have to be the same */
/* at pipeline creation and in any call to set them dynamically   */
/* They also have to be the same as each other                    */
#define NUM_VIEWPORTS 1
#define NUM_SCISSORS NUM_VIEWPORTS

#define LENGTH 127

#define XYZ1(_x_, _y_, _z_) (_x_), (_y_), (_z_), 1.f

#define GLM_FORCE_RADIANS
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <cassert>
#include <vulkan/vulkan.h>
#include <vulkan/vk_sdk_platform.h>

#include "SPIRV/GlslangToSpv.h"

#include <Windows.h>

using namespace std;

/*
	Structs
*/

struct Vertex {
	float posX, posY, posZ, posW; // Position data
	float r, g, b, a;             // Color
};

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

typedef struct _uniform {
	VkBuffer buf;
	VkDeviceMemory mem;
	VkDescriptorBufferInfo buffer_info;
	VkMemoryRequirements mem_reqs;
} UniformBuffer;

typedef struct _vb {
	VkBuffer buf;
	VkDeviceMemory mem;
	VkDescriptorBufferInfo buffer_info;
	VkMemoryRequirements mem_reqs;
} VertexBuffer;

typedef struct _ib {
	VkBuffer buf;
	VkDeviceMemory mem;
	VkDescriptorBufferInfo buffer_info;
	VkMemoryRequirements mem_reqs;
} IndexBuffer;


/*
	Globals
*/

static const Vertex g_vb_solid_face_colors_Data[] = {
	{ XYZ1(-1, -1, -1), XYZ1(1.f, 0.f, 0.f) },
	{ XYZ1(1, -1, -1), XYZ1(1.f, 0.f, 0.f) },
	{ XYZ1(-1, 1, -1), XYZ1(1.f, 0.f, 0.f) },
	{ XYZ1(-1, 1, -1), XYZ1(1.f, 0.f, 0.f) },
	{ XYZ1(1, -1, -1), XYZ1(1.f, 0.f, 0.f) },
	{ XYZ1(1, 1, -1), XYZ1(1.f, 0.f, 0.f) },

	{ XYZ1(-1, -1, 1), XYZ1(0.f, 1.f, 0.f) },
	{ XYZ1(-1, 1, 1), XYZ1(0.f, 1.f, 0.f) },
	{ XYZ1(1, -1, 1), XYZ1(0.f, 1.f, 0.f) },
	{ XYZ1(1, -1, 1), XYZ1(0.f, 1.f, 0.f) },
	{ XYZ1(-1, 1, 1), XYZ1(0.f, 1.f, 0.f) },
	{ XYZ1(1, 1, 1), XYZ1(0.f, 1.f, 0.f) },

	{ XYZ1(1, 1, 1), XYZ1(0.f, 0.f, 1.f) },
	{ XYZ1(1, 1, -1), XYZ1(0.f, 0.f, 1.f) },
	{ XYZ1(1, -1, 1), XYZ1(0.f, 0.f, 1.f) },
	{ XYZ1(1, -1, 1), XYZ1(0.f, 0.f, 1.f) },
	{ XYZ1(1, 1, -1), XYZ1(0.f, 0.f, 1.f) },
	{ XYZ1(1, -1, -1), XYZ1(0.f, 0.f, 1.f) },

	{ XYZ1(-1, 1, 1), XYZ1(1.f, 1.f, 0.f) },
	{ XYZ1(-1, -1, 1), XYZ1(1.f, 1.f, 0.f) },
	{ XYZ1(-1, 1, -1), XYZ1(1.f, 1.f, 0.f) },
	{ XYZ1(-1, 1, -1), XYZ1(1.f, 1.f, 0.f) },
	{ XYZ1(-1, -1, 1), XYZ1(1.f, 1.f, 0.f) },
	{ XYZ1(-1, -1, -1), XYZ1(1.f, 1.f, 0.f) },

	{ XYZ1(1, 1, 1), XYZ1(1.f, 0.f, 1.f) },
	{ XYZ1(-1, 1, 1), XYZ1(1.f, 0.f, 1.f) },
	{ XYZ1(1, 1, -1), XYZ1(1.f, 0.f, 1.f) },
	{ XYZ1(1, 1, -1), XYZ1(1.f, 0.f, 1.f) },
	{ XYZ1(-1, 1, 1), XYZ1(1.f, 0.f, 1.f) },
	{ XYZ1(-1, 1, -1), XYZ1(1.f, 0.f, 1.f) },

	{ XYZ1(1, -1, 1), XYZ1(0.f, 1.f, 1.f) },
	{ XYZ1(1, -1, -1), XYZ1(0.f, 1.f, 1.f) },
	{ XYZ1(-1, -1, 1), XYZ1(0.f, 1.f, 1.f) },
	{ XYZ1(-1, -1, 1), XYZ1(0.f, 1.f, 1.f) },
	{ XYZ1(1, -1, -1), XYZ1(0.f, 1.f, 1.f) },
	{ XYZ1(-1, -1, -1), XYZ1(0.f, 1.f, 1.f) },
};

bool useWireframe = false;

static Vertex surfaceData[(LENGTH + 1) * (LENGTH + 1)];
static uint32_t indexData[(LENGTH * LENGTH) * 6];

static const char *vertShader =
"#version 400\n"
"#extension GL_ARB_separate_shader_objects : enable\n"
"#extension GL_ARB_shading_language_420pack : enable\n"
"layout (std140, binding = 0) uniform bufferVals {\n"
"    mat4 mvp;\n"
"} myBufferVals;\n"
"layout (location = 0) in vec4 pos;\n"
"layout (location = 1) in vec4 inColor;\n"
"layout (location = 0) out vec4 outColor;\n"
"layout (location = 1) out vec3 position;\n"
"out gl_PerVertex { \n"
"    vec4 gl_Position;\n"
"};\n"
"void main() {\n"
"   outColor = inColor;\n"
"	position = (myBufferVals.mvp * pos).xyz;\n"
"   gl_Position = myBufferVals.mvp * pos;\n"
"}\n";

static const char *fragShader =
"#version 400\n"
"#extension GL_ARB_separate_shader_objects : enable\n"
"#extension GL_ARB_shading_language_420pack : enable\n"
"layout (location = 0) in vec4 color;\n"
"layout (location = 1) in vec3 position;\n"
"layout (location = 0) out vec4 outColor;\n"
"void main() {\n"
"	vec3 lightPos = vec3(1, 1, 0);\n"
"	vec3 camPos = vec3(0, 3, 10);\n"
"	vec3 toCam = camPos - position;\n"
"   //outColor = dot(lightPos, toCam) * color * 0.3 + color * 0.7;\n"
"   outColor = color;\n"
"}\n";

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
uint32_t m_currentBuffer;

DepthMap m_depthMap;

UniformBuffer m_uniform;

VkPipelineLayout m_pipelineLayout;
vector<VkDescriptorSetLayout> m_descLayout;
VkPipelineCache m_pipelineCache;
VkRenderPass m_renderPass;
VkPipeline m_pipeline;

VkPipelineShaderStageCreateInfo m_shaderStages[2];

VkFramebuffer *m_framebuffers;

VertexBuffer m_vertexBuffer;
IndexBuffer m_indexBuffer;
VkVertexInputBindingDescription m_viBinding;
VkVertexInputAttributeDescription m_viAttribs[2];

VkDescriptorPool m_descPool;
vector<VkDescriptorSet> m_descSet;

VkViewport m_viewport;
VkRect2D m_scissor;

bool m_prepared;

///win32
HINSTANCE m_connection;
HWND m_window;

///const
const int fenceTimeout = 100000000;
const int width = 768;
const int height = 768;

/*
	Win32 window
*/
static void run();

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	case WM_MOVE:
	case WM_MOVING:
	case WM_PAINT:
		run();
		return 0;
	case WM_ERASEBKGND:
		return 0; //This one is really hacking lol (may lead to some issue but who cares lol)
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

void initResources(TBuiltInResource &Resources) {
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

EShLanguage FindLanguage(const VkShaderStageFlagBits shader_type) {
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

bool GLSLtoSPV(const VkShaderStageFlagBits shader_type, const char *pshader,
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

		SetImageLayout(cmdBuf, queue, buffer[i].image, VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		color_image_view.image = buffer[i].image;

		res = vkCreateImageView(device, &color_image_view, NULL,
			&buffer[i].view);
		assert(res == VK_SUCCESS);
	}

	return swapChain;
}

DepthMap CreateDepthMap(
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

	return depth;
}

UniformBuffer CreateUniformBuffer(
	void *content, int size,
	VkDevice device, VkPhysicalDeviceMemoryProperties memoryProperties)
{
	UniformBuffer uniform_data;

	VkBufferCreateInfo buf_info = {};
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buf_info.pNext = NULL;
	buf_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	buf_info.size = size;
	buf_info.queueFamilyIndexCount = 0;
	buf_info.pQueueFamilyIndices = NULL;
	buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	buf_info.flags = 0;
	VkResult res = vkCreateBuffer(device, &buf_info, NULL, &uniform_data.buf);
	assert(res == VK_SUCCESS);

	VkMemoryRequirements mem_reqs;
	vkGetBufferMemoryRequirements(device, uniform_data.buf,
		&mem_reqs);

	uniform_data.mem_reqs = mem_reqs;

	VkMemoryAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.pNext = NULL;
	alloc_info.memoryTypeIndex = 0;

	alloc_info.allocationSize = mem_reqs.size;
	bool pass = MemoryTypeFromProperties(memoryProperties, mem_reqs.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&alloc_info.memoryTypeIndex);
	assert(pass && "No mappable, coherent memory");

	res = vkAllocateMemory(device, &alloc_info, NULL,
		&(uniform_data.mem));
	assert(res == VK_SUCCESS);

	uint8_t *pData;
	res = vkMapMemory(device, uniform_data.mem, 0, mem_reqs.size, 0,
		(void **)&pData);
	assert(res == VK_SUCCESS);

	memcpy(pData, content, size);

	vkUnmapMemory(device, uniform_data.mem);

	res = vkBindBufferMemory(device, uniform_data.buf,
		uniform_data.mem, 0);
	assert(res == VK_SUCCESS);

	uniform_data.buffer_info.buffer = uniform_data.buf;
	uniform_data.buffer_info.offset = 0;
	uniform_data.buffer_info.range = size;

	return uniform_data;
}

void InitDescriptorPipelineLayout(
	vector<VkDescriptorSetLayout> &desc_layout, VkPipelineLayout &pipeline_layout,
	VkDevice device)
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

	desc_layout.resize(NUM_DESCRIPTOR_SETS);
	res = vkCreateDescriptorSetLayout(device, &descriptor_layout, NULL,
		desc_layout.data());
	assert(res == VK_SUCCESS);

	/* Now use the descriptor layout to create a pipeline layout */
	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
	pPipelineLayoutCreateInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pPipelineLayoutCreateInfo.pNext = NULL;
	pPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pPipelineLayoutCreateInfo.pPushConstantRanges = NULL;
	pPipelineLayoutCreateInfo.setLayoutCount = NUM_DESCRIPTOR_SETS;
	pPipelineLayoutCreateInfo.pSetLayouts = desc_layout.data();

	res = vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, NULL,
		&pipeline_layout);
	assert(res == VK_SUCCESS);
}

VkRenderPass CreateRenderPass(
	VkFormat imgFormat, DepthMap &depth, VkDevice device)
{
	VkRenderPass render_pass;

	VkResult res;
	/* Need attachments for render target and depth buffer */
	VkAttachmentDescription attachments[2];
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


	attachments[1].format = depth.format;
	attachments[1].samples = NUM_SAMPLES;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		//clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[1].initialLayout =
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachments[1].finalLayout =
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachments[1].flags = 0;

	VkAttachmentReference color_reference = {};
	color_reference.attachment = 0;
	color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_reference = {};
	depth_reference.attachment = 1;
	depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.flags = 0;
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = NULL;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_reference;
	subpass.pResolveAttachments = NULL;
	subpass.pDepthStencilAttachment = &depth_reference;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = NULL;

	VkRenderPassCreateInfo rp_info = {};
	rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rp_info.pNext = NULL;
	rp_info.attachmentCount = 2;
	rp_info.pAttachments = attachments;
	rp_info.subpassCount = 1;
	rp_info.pSubpasses = &subpass;
	rp_info.dependencyCount = 0;
	rp_info.pDependencies = NULL;

	res = vkCreateRenderPass(device, &rp_info, NULL, &render_pass);
	assert(res == VK_SUCCESS);

	return render_pass;
}

void InitShaderStageCreateInfo(
	const char *vertShaderText, const char *fragShaderText,
	VkPipelineShaderStageCreateInfo &vert, VkPipelineShaderStageCreateInfo &frag,
	VkDevice device)
{
	VkResult res;
	bool retVal;

	// If no shaders were submitted, just return
	if (!(vertShaderText || fragShaderText))
		return;

	glslang::InitializeProcess();
	VkShaderModuleCreateInfo moduleCreateInfo;

	if (vertShaderText) {
		std::vector<unsigned int> vtx_spv;
		vert.sType =
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vert.pNext = NULL;
		vert.pSpecializationInfo = NULL;
		vert.flags = 0;
		vert.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vert.pName = "main";

		retVal = GLSLtoSPV(VK_SHADER_STAGE_VERTEX_BIT, vertShaderText, vtx_spv);
		assert(retVal);

		moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		moduleCreateInfo.pNext = NULL;
		moduleCreateInfo.flags = 0;
		moduleCreateInfo.codeSize = vtx_spv.size() * sizeof(unsigned int);
		moduleCreateInfo.pCode = vtx_spv.data();
		res = vkCreateShaderModule(device, &moduleCreateInfo, NULL,
			&vert.module);
		assert(res == VK_SUCCESS);
	}

	if (fragShaderText) {
		std::vector<unsigned int> frag_spv;
		frag.sType =
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		frag.pNext = NULL;
		frag.pSpecializationInfo = NULL;
		frag.flags = 0;
		frag.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		frag.pName = "main";

		retVal =
			GLSLtoSPV(VK_SHADER_STAGE_FRAGMENT_BIT, fragShaderText, frag_spv);
		assert(retVal);

		moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		moduleCreateInfo.pNext = NULL;
		moduleCreateInfo.flags = 0;
		moduleCreateInfo.codeSize = frag_spv.size() * sizeof(unsigned int);
		moduleCreateInfo.pCode = frag_spv.data();
		res = vkCreateShaderModule(device, &moduleCreateInfo, NULL,
			&frag.module);
		assert(res == VK_SUCCESS);
	}

	glslang::FinalizeProcess();
}

VkFramebuffer* CreateFrameBuffers(
	VkDevice device,
	DepthMap &depth, SwapChainBuffer *buffers,
	VkRenderPass &render_pass, int width, int height,
	int swapchainImageCount)
{
	VkFramebuffer *framebuffers;

	VkResult res;
	VkImageView attachments[2];
	attachments[1] = depth.view;

	VkFramebufferCreateInfo fb_info = {};
	fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fb_info.pNext = NULL;
	fb_info.renderPass = render_pass;
	fb_info.attachmentCount = 2;
	fb_info.pAttachments = attachments;
	fb_info.width = width;
	fb_info.height = height;
	fb_info.layers = 1;

	uint32_t i;

	framebuffers = (VkFramebuffer *)malloc(swapchainImageCount *
		sizeof(VkFramebuffer));

	for (i = 0; i < swapchainImageCount; i++) {
		attachments[0] = buffers[i].view;
		res = vkCreateFramebuffer(device, &fb_info, NULL,
			&framebuffers[i]);
		assert(res == VK_SUCCESS);
	}

	return framebuffers;
}

VertexBuffer CreateVertexBuffer(
	VkDevice device, VkPhysicalDeviceMemoryProperties memoryProp,
	const void *vertexData, uint32_t dataSize, uint32_t dataStride,
	VkVertexInputBindingDescription &vi_binding, VkVertexInputAttributeDescription* vi_attribs)
{
	VertexBuffer vertex_buffer;

	VkResult res;
	bool pass;

	VkBufferCreateInfo buf_info = {};
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buf_info.pNext = NULL;
	buf_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	buf_info.size = dataSize;
	buf_info.queueFamilyIndexCount = 0;
	buf_info.pQueueFamilyIndices = NULL;
	buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	buf_info.flags = 0;
	res = vkCreateBuffer(device, &buf_info, NULL, &vertex_buffer.buf);
	assert(res == VK_SUCCESS);

	VkMemoryRequirements mem_reqs;
	vkGetBufferMemoryRequirements(device, vertex_buffer.buf,
		&mem_reqs);

	vertex_buffer.mem_reqs = mem_reqs;

	VkMemoryAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.pNext = NULL;
	alloc_info.memoryTypeIndex = 0;

	alloc_info.allocationSize = mem_reqs.size;
	pass = MemoryTypeFromProperties(memoryProp, mem_reqs.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&alloc_info.memoryTypeIndex);
	assert(pass && "No mappable, coherent memory");

	res = vkAllocateMemory(device, &alloc_info, NULL,
		&(vertex_buffer.mem));
	assert(res == VK_SUCCESS);
	vertex_buffer.buffer_info.range = mem_reqs.size;
	vertex_buffer.buffer_info.offset = 0;

	uint8_t *pData;
	res = vkMapMemory(device, vertex_buffer.mem, 0, mem_reqs.size, 0,
		(void **)&pData);
	assert(res == VK_SUCCESS);

	memcpy(pData, vertexData, dataSize);

	vkUnmapMemory(device, vertex_buffer.mem);

	res = vkBindBufferMemory(device, vertex_buffer.buf,
		vertex_buffer.mem, 0);
	assert(res == VK_SUCCESS);

	vi_binding.binding = 0;
	vi_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	vi_binding.stride = dataStride;

	vi_attribs[0].binding = 0;
	vi_attribs[0].location = 0;
	vi_attribs[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	vi_attribs[0].offset = 0;
	vi_attribs[1].binding = 0;
	vi_attribs[1].location = 1;
	vi_attribs[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	vi_attribs[1].offset = 4 * sizeof(float);

	return vertex_buffer;
}

IndexBuffer CreateIndexBuffer(
	VkDevice device, VkPhysicalDeviceMemoryProperties memoryProp,
	const void *vertexData, uint32_t dataSize, uint32_t dataStride)
{
	IndexBuffer index_buffer;

	VkResult res;
	bool pass;

	VkBufferCreateInfo buf_info = {};
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buf_info.pNext = NULL;
	buf_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	buf_info.size = dataSize;
	buf_info.queueFamilyIndexCount = 0;
	buf_info.pQueueFamilyIndices = NULL;
	buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	buf_info.flags = 0;
	res = vkCreateBuffer(device, &buf_info, NULL, &index_buffer.buf);
	assert(res == VK_SUCCESS);

	VkMemoryRequirements mem_reqs;
	vkGetBufferMemoryRequirements(device, index_buffer.buf,
		&mem_reqs);

	index_buffer.mem_reqs = mem_reqs;

	VkMemoryAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.pNext = NULL;
	alloc_info.memoryTypeIndex = 0;

	alloc_info.allocationSize = mem_reqs.size;
	pass = MemoryTypeFromProperties(memoryProp, mem_reqs.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&alloc_info.memoryTypeIndex);
	assert(pass && "No mappable, coherent memory");

	res = vkAllocateMemory(device, &alloc_info, NULL,
		&(index_buffer.mem));
	assert(res == VK_SUCCESS);
	index_buffer.buffer_info.range = mem_reqs.size;
	index_buffer.buffer_info.offset = 0;

	uint8_t *pData;
	res = vkMapMemory(device, index_buffer.mem, 0, mem_reqs.size, 0,
		(void **)&pData);
	assert(res == VK_SUCCESS);

	memcpy(pData, vertexData, dataSize);

	vkUnmapMemory(device, index_buffer.mem);

	res = vkBindBufferMemory(device, index_buffer.buf,
		index_buffer.mem, 0);
	assert(res == VK_SUCCESS);

	return index_buffer;
}

VkDescriptorPool CreateDescriptorPool(VkDevice device)
{
	VkDescriptorPool descriptorPool;

	VkResult res;
	VkDescriptorPoolSize type_count[2];
	type_count[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	type_count[0].descriptorCount = 1;

	VkDescriptorPoolCreateInfo descriptor_pool = {};
	descriptor_pool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptor_pool.pNext = NULL;
	descriptor_pool.maxSets = 1;
	descriptor_pool.poolSizeCount = 1;
	descriptor_pool.pPoolSizes = type_count;

	res = vkCreateDescriptorPool(device, &descriptor_pool, NULL,
		&descriptorPool);
	assert(res == VK_SUCCESS);

	return descriptorPool;
}

void CreateDescriptorSet(
	VkDevice device, VkDescriptorPool &desc_pool, vector<VkDescriptorSetLayout> &desc_layout, 
	vector<VkDescriptorSet> &desc_set, UniformBuffer &uniform_data)
{
	VkResult res;

	VkDescriptorSetAllocateInfo alloc_info[1];
	alloc_info[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info[0].pNext = NULL;
	alloc_info[0].descriptorPool = desc_pool;
	alloc_info[0].descriptorSetCount = NUM_DESCRIPTOR_SETS;
	alloc_info[0].pSetLayouts = desc_layout.data();

	desc_set.resize(NUM_DESCRIPTOR_SETS);
	res =
		vkAllocateDescriptorSets(device, alloc_info, desc_set.data());
	assert(res == VK_SUCCESS);

	VkWriteDescriptorSet writes[2];

	writes[0] = {};
	writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[0].pNext = NULL;
	writes[0].dstSet = desc_set[0];
	writes[0].descriptorCount = 1;
	writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writes[0].pBufferInfo = &uniform_data.buffer_info;
	writes[0].dstArrayElement = 0;
	writes[0].dstBinding = 0;

	vkUpdateDescriptorSets(device, 1, writes, 0, NULL);
}

void CreatePipeline(
	VkDevice device, VkPipelineCache &pipelineCache, VkPipeline &pipelineObj,
	VkVertexInputBindingDescription &vi_binding, VkVertexInputAttributeDescription* vi_attribs,
	VkPipelineLayout pipeline_layout, VkPipelineShaderStageCreateInfo *shaderStages,
	VkRenderPass render_pass)
{
	VkResult res;

	VkPipelineCacheCreateInfo pipelineCachei;
	pipelineCachei.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	pipelineCachei.pNext = NULL;
	pipelineCachei.initialDataSize = 0;
	pipelineCachei.pInitialData = NULL;
	pipelineCachei.flags = 0;
	res = vkCreatePipelineCache(device, &pipelineCachei, NULL,
		&pipelineCache);
	assert(res == VK_SUCCESS);

	//Dynamic state ?
	VkDynamicState dynamicStateEnables[VK_DYNAMIC_STATE_RANGE_SIZE];
	VkPipelineDynamicStateCreateInfo dynamicState = {};
	memset(dynamicStateEnables, 0, sizeof dynamicStateEnables);
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pNext = NULL;
	dynamicState.pDynamicStates = dynamicStateEnables;
	dynamicState.dynamicStateCount = 0;

	//Vertex buffer format ?
	VkPipelineVertexInputStateCreateInfo vi;
	memset(&vi, 0, sizeof(vi));
	vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vi.pNext = NULL;
	vi.flags = 0;
	vi.vertexBindingDescriptionCount = 1;
	vi.pVertexBindingDescriptions = &vi_binding;
	vi.vertexAttributeDescriptionCount = 2;
	vi.pVertexAttributeDescriptions = vi_attribs;

	//Input assamble£¨TRIANGLE_LIST£©
	VkPipelineInputAssemblyStateCreateInfo ia;
	ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	ia.pNext = NULL;
	ia.flags = 0;
	ia.primitiveRestartEnable = VK_FALSE;
	ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	//Rasterization ( polygonMode = Fill, cullMode = BackCulling, frontFace = Clockwise, depth etc. )
	VkPipelineRasterizationStateCreateInfo rs;
	rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rs.pNext = NULL;
	rs.flags = 0;
	rs.polygonMode = useWireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
	rs.cullMode = VK_CULL_MODE_NONE;
	rs.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rs.depthClampEnable = VK_TRUE;
	rs.rasterizerDiscardEnable = VK_FALSE;
	rs.depthBiasEnable = VK_FALSE;
	rs.depthBiasConstantFactor = 0;
	rs.depthBiasClamp = 0;
	rs.depthBiasSlopeFactor = 0;
	rs.lineWidth = 1.0f;

	//Colorblending ( DISABLED [ Add, rgb = 0 + 0, a = 0 + 0 ] ) [[[What is cb.blendConstants?]]]
	VkPipelineColorBlendStateCreateInfo cb;
	cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	cb.flags = 0;
	cb.pNext = NULL;
	VkPipelineColorBlendAttachmentState att_state[1];
	att_state[0].colorWriteMask = 0xf;
	att_state[0].blendEnable = VK_FALSE;
	att_state[0].alphaBlendOp = VK_BLEND_OP_ADD;
	att_state[0].colorBlendOp = VK_BLEND_OP_ADD;
	att_state[0].srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	att_state[0].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	att_state[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	att_state[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	cb.attachmentCount = 1;
	cb.pAttachments = att_state;
	cb.logicOpEnable = VK_FALSE;
	cb.logicOp = VK_LOGIC_OP_NO_OP;
	cb.blendConstants[0] = 1.0f;
	cb.blendConstants[1] = 1.0f;
	cb.blendConstants[2] = 1.0f;
	cb.blendConstants[3] = 1.0f;

	//Viewport
	VkPipelineViewportStateCreateInfo vp = {};
	vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vp.pNext = NULL;
	vp.flags = 0;

	vp.viewportCount = NUM_VIEWPORTS;
	dynamicStateEnables[dynamicState.dynamicStateCount++] =
		VK_DYNAMIC_STATE_VIEWPORT;
	vp.scissorCount = NUM_SCISSORS;
	dynamicStateEnables[dynamicState.dynamicStateCount++] =
		VK_DYNAMIC_STATE_SCISSOR;
	vp.pScissors = NULL;
	vp.pViewports = NULL;

	//Depth & Stencil ( depthTestMode: <= )
	VkPipelineDepthStencilStateCreateInfo ds;
	ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	ds.pNext = NULL;
	ds.flags = 0;
	ds.depthTestEnable = true;
	ds.depthWriteEnable = true;
	ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	ds.depthBoundsTestEnable = VK_FALSE;
	ds.stencilTestEnable = VK_FALSE;
	ds.back.failOp = VK_STENCIL_OP_KEEP;
	ds.back.passOp = VK_STENCIL_OP_KEEP;
	ds.back.compareOp = VK_COMPARE_OP_ALWAYS;
	ds.back.compareMask = 0;
	ds.back.reference = 0;
	ds.back.depthFailOp = VK_STENCIL_OP_KEEP;
	ds.back.writeMask = 0;
	ds.minDepthBounds = 0;
	ds.maxDepthBounds = 0;
	ds.stencilTestEnable = VK_FALSE;
	ds.front = ds.back;

	//MSAA?
	VkPipelineMultisampleStateCreateInfo ms;
	ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	ms.pNext = NULL;
	ms.flags = 0;
	ms.pSampleMask = NULL;
	ms.rasterizationSamples = NUM_SAMPLES;
	ms.sampleShadingEnable = VK_FALSE;
	ms.alphaToCoverageEnable = VK_FALSE;
	ms.alphaToOneEnable = VK_FALSE;
	ms.minSampleShading = 0.0;

	//pipeline!
	VkGraphicsPipelineCreateInfo pipeline;
	pipeline.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline.pNext = NULL;
	pipeline.layout = pipeline_layout;
	pipeline.basePipelineHandle = VK_NULL_HANDLE;
	pipeline.basePipelineIndex = 0;
	pipeline.flags = 0;
	pipeline.pVertexInputState = &vi;
	pipeline.pInputAssemblyState = &ia;
	pipeline.pRasterizationState = &rs;
	pipeline.pColorBlendState = &cb;
	pipeline.pTessellationState = NULL;
	pipeline.pMultisampleState = &ms;
	pipeline.pDynamicState = &dynamicState;
	pipeline.pViewportState = &vp;
	pipeline.pDepthStencilState = &ds;
	pipeline.pStages = shaderStages;
	pipeline.stageCount = 2;
	pipeline.renderPass = render_pass;
	pipeline.subpass = 0;

	res = vkCreateGraphicsPipelines(device, pipelineCache, 1,
		&pipeline, NULL, &pipelineObj);
	assert(res == VK_SUCCESS);
}

void initViewports(int width, int height, VkCommandBuffer cmdBuf, VkViewport &viewport) 
{
	viewport.height = (float)height;
	viewport.width = (float)width;
	viewport.minDepth = (float)0.0f;
	viewport.maxDepth = (float)1.0f;
	viewport.x = 0;
	viewport.y = 0;
	vkCmdSetViewport(cmdBuf, 0, NUM_VIEWPORTS, &viewport);
}

void initScissors(int width, int height, VkCommandBuffer cmdBuf, VkRect2D &scissor) 
{
	scissor.extent.width = width;
	scissor.extent.height = height;
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	vkCmdSetScissor(cmdBuf, 0, NUM_SCISSORS, &scissor);
}

void render()
{
	BeginCommandBuffer(m_cmdBuffer);

	//DrawCube!
	VkResult res;

	VkClearValue clear_values[2];
	clear_values[0].color.float32[0] = 0.1f;
	clear_values[0].color.float32[1] = 0.1f;
	clear_values[0].color.float32[2] = 0.1f;
	clear_values[0].color.float32[3] = 1.0f;
	clear_values[1].depthStencil.depth = 1.0f;
	clear_values[1].depthStencil.stencil = 0;

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

	SetImageLayout(m_cmdBuffer, m_queue, m_swapChainImgBuffer[m_currentBuffer].image,
		VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	VkRenderPassBeginInfo rp_begin;
	rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rp_begin.pNext = NULL;
	rp_begin.renderPass = m_renderPass;
	rp_begin.framebuffer = m_framebuffers[m_currentBuffer];
	rp_begin.renderArea.offset.x = 0;
	rp_begin.renderArea.offset.y = 0;
	rp_begin.renderArea.extent.width = width;
	rp_begin.renderArea.extent.height = height;
	rp_begin.clearValueCount = 2;
	rp_begin.pClearValues = clear_values;

	vkCmdBeginRenderPass(m_cmdBuffer, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(m_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
	vkCmdBindDescriptorSets(m_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		m_pipelineLayout, 0, NUM_DESCRIPTOR_SETS,
		m_descSet.data(), 0, NULL);

	//const VkDeviceSize offsets[1] = { 0 };
	//vkCmdBindVertexBuffers(m_cmdBuffer, 0, 1, &m_vertexBuffer.buf, offsets);
	
	initViewports(width, height, m_cmdBuffer, m_viewport);
	initScissors(width, height, m_cmdBuffer, m_scissor);

	vkCmdDraw(m_cmdBuffer, 12 * 3, 1, 0, 0);
	//vkCmdDrawIndexed(m_cmdBuffer, (LENGTH * LENGTH) * 6, 1, 0, 0, 0);

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

void GenerateNewBuffers(int frame)
{
	int i, j;
	for (i = 0;i < (LENGTH+1); i++)
	{
		for (j = 0;j < (LENGTH+1);j++)
		{
			surfaceData[i * (LENGTH + 1) + j] = { XYZ1((float)i / (LENGTH) * 5, (float)sin((float)i / (LENGTH) * 15 + (float)frame / 200.0), (float)j / (LENGTH) * 5), XYZ1(1.f, 1.f, 1.f) };
		}
	}

	for (i = 0;i < LENGTH;i++)
	{
		for (j = 0;j < LENGTH;j++)
		{

			/*

			2*(l+1)		2*(l+1)+1	...
			l+1			l+1+1		...
			0			1			...

			*/

			indexData[(i*LENGTH + j) * 6 + 0] = (i + 0) * (LENGTH + 1) + (j + 0);
			indexData[(i*LENGTH + j) * 6 + 1] = (i + 1) * (LENGTH + 1) + (j + 0);
			indexData[(i*LENGTH + j) * 6 + 2] = (i + 1) * (LENGTH + 1) + (j + 1);

			indexData[(i*LENGTH + j) * 6 + 3] = (i + 0) * (LENGTH + 1) + (j + 0);
			indexData[(i*LENGTH + j) * 6 + 4] = (i + 1) * (LENGTH + 1) + (j + 1);
			indexData[(i*LENGTH + j) * 6 + 5] = (i + 0) * (LENGTH + 1) + (j + 1);
		}
	}
}

void update()
{
	static int frame = 0;

	double fac = (double)frame / 600.0f;

	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
	glm::mat4 View = glm::lookAt(
		glm::vec3(10 * glm::sin(fac), 6 * glm::cos(fac / 2.5), 10 * glm::cos(fac)), // Camera is at (0,3,10), in World Space
		glm::vec3(0, 0, 0),  // and looks at the origin
		glm::vec3(0, -1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
		);
	glm::mat4 Model = glm::mat4(1.0f);
	// Vulkan clip space has inverted Y and half Z.
	glm::mat4 Clip = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f,
		0.0f, 0.0f, 0.5f, 1.0f);

	glm::mat4 MVP = Clip * Projection * View * Model;

	uint8_t *pData;
	VkResult res = vkMapMemory(m_device, m_uniform.mem, 0, m_uniform.mem_reqs.size, 0, (void **)&pData);
	assert(res == VK_SUCCESS);

	memcpy(pData, (const void*)&MVP, sizeof(MVP));

	vkUnmapMemory(m_device, m_uniform.mem);

	GenerateNewBuffers(frame);

	//uint8_t *pData;

	//VkResult res = vkMapMemory(m_device, m_vertexBuffer.mem, 0, m_vertexBuffer.mem_reqs.size, 0,
	//	(void **)&pData);
	//assert(res == VK_SUCCESS);

	//memcpy(pData, (const void*)&surfaceData, sizeof(surfaceData));

	//vkUnmapMemory(m_device, m_vertexBuffer.mem);

	//No need to care index buffer cuz we actully don't change it

	frame++;
	return;
}

static void run()
{
	if (!m_prepared) return;

	vkDeviceWaitIdle(m_device);
	
	update();
	render();

	vkDeviceWaitIdle(m_device);
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

	BeginCommandBuffer(m_cmdBuffer);

	//Init swapChain
	m_surface = GetSurface(m_connection, m_window, m_instance);
	m_queueGraphicsPresentIndex = GetGraphicsPresentQueueIndex(m_queueCount, m_GPUs[0], m_queueProps, m_surface);
	m_swapChain = CreateSwapChain(
		m_GPUs[0], m_surface, m_colorImgFormat, width, height, m_device,
		m_swapChainImageCount, m_swapChainImgBuffer, m_cmdBuffer, m_queue, m_queueFamilyIndex);

	//Create DepthBuffer
	m_depthMap = CreateDepthMap(m_GPUs[0], m_device, width, height, m_memoryProperties, m_cmdBuffer, m_queue);

	//Create UniformBuffer
	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
	glm::mat4 View = glm::lookAt(
		glm::vec3(2.5, 3, 10), // Camera is at (2.5,3,10), in World Space
		glm::vec3(2.5, 0, 0),  // and looks at the origin
		glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
		);
	glm::mat4 Model = glm::mat4(1.0f);
	// Vulkan clip space has inverted Y and half Z.
	glm::mat4 Clip = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f,
		0.0f, 0.0f, 0.5f, 1.0f);

	glm::mat4 MVP = Clip * Projection * View * Model;

	m_uniform = CreateUniformBuffer(&MVP, sizeof(MVP), m_device, m_memoryProperties);

	//Descriptor & pipeline Layout
	InitDescriptorPipelineLayout(m_descLayout, m_pipelineLayout, m_device);

	//RenderPass
	m_renderPass = CreateRenderPass(m_colorImgFormat, m_depthMap, m_device);

	//Shader. glsl to SPIR-V by glslang.
	InitShaderStageCreateInfo(vertShader, fragShader, m_shaderStages[0], m_shaderStages[1], m_device);

	//Create frame buffers
	m_framebuffers = CreateFrameBuffers(m_device, m_depthMap, m_swapChainImgBuffer.data(), m_renderPass, width, height, m_swapChainImageCount);

	//Vertex buffer
	GenerateNewBuffers(0);
	//m_vertexBuffer = CreateVertexBuffer(
	//	m_device, m_memoryProperties, &surfaceData,
	//	sizeof(surfaceData), sizeof(surfaceData[0]), m_viBinding, m_viAttribs);

	m_vertexBuffer = CreateVertexBuffer(
		m_device, m_memoryProperties, &g_vb_solid_face_colors_Data,
		sizeof(g_vb_solid_face_colors_Data), sizeof(g_vb_solid_face_colors_Data[0]), m_viBinding, m_viAttribs);

	//Index buffer
	/*m_indexBuffer = CreateIndexBuffer(
		m_device, m_memoryProperties, &indexData,
		sizeof(indexData), sizeof(indexData[0]));*/

	//Desc pool & set
	m_descPool = CreateDescriptorPool(m_device);
	CreateDescriptorSet(m_device, m_descPool, m_descLayout, m_descSet, m_uniform);

	//Pipeline
	CreatePipeline(m_device, m_pipelineCache, m_pipeline, m_viBinding, m_viAttribs, m_pipelineLayout, m_shaderStages, m_renderPass);

	/**/vkCmdBindPipeline(m_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
	/**/vkCmdBindDescriptorSets(m_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		m_pipelineLayout, 0, NUM_DESCRIPTOR_SETS,
		m_descSet.data(), 0, NULL);

	/**/const VkDeviceSize offsets[1] = { 0 };
	/**/vkCmdBindVertexBuffers(m_cmdBuffer, 0, 1, &m_vertexBuffer.buf, offsets);
	///**/vkCmdBindIndexBuffer(m_cmdBuffer, m_indexBuffer.buf, 0, VK_INDEX_TYPE_UINT32);

	/**/initViewports(width, height, m_cmdBuffer, m_viewport);
	/**/initScissors(width, height, m_cmdBuffer, m_scissor);

	//Finish init
	EndCommandBuffer(m_cmdBuffer);
	QueueCommandBuffer(m_cmdBuffer, m_device, m_queue);

	m_prepared = true;

	MSG msg;

	bool done = false;
	while (!done)
	{
		PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
		if (msg.message == WM_QUIT) // check for a quit message
		{
			done = true; // if found, quit app
		}
		else {
			/* Translate and dispatch to event queue*/
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		RedrawWindow(m_window, NULL, NULL, RDW_INTERNALPAINT);
	}

	//cleanUp
	uint32_t i;

	m_prepared = false;

	for (i = 0;i < m_swapChainImageCount;i++)
	{
		vkDestroyFramebuffer(m_device, m_framebuffers[i], NULL);
	}
	free(m_framebuffers);

	vkDestroyDescriptorPool(m_device, m_descPool, NULL);

	vkDestroyPipeline(m_device, m_pipeline, NULL);
	vkDestroyPipelineCache(m_device, m_pipelineCache, NULL);
	vkDestroyRenderPass(m_device, m_renderPass, NULL);
	vkDestroyPipelineLayout(m_device, m_pipelineLayout, NULL);

	for (i = 0;i < m_descLayout.size();i++)
	{
		vkDestroyDescriptorSetLayout(m_device, m_descLayout[i], NULL);
	}

	/*
		for (i = 0; i < DEMO_TEXTURE_COUNT; i++) {
		vkDestroyImageView(m_device, demo->textures[i].view, NULL);
		vkDestroyImage(m_device, demo->textures[i].image, NULL);
		vkFreeMemory(m_device, demo->textures[i].mem, NULL);
		vkDestroySampler(m_device, demo->textures[i].sampler, NULL);
		}
	*/
	vkDestroySwapchainKHR(m_device, m_swapChain, NULL);

	vkDestroyImageView(m_device, m_depthMap.view, NULL);
	vkDestroyImage(m_device, m_depthMap.image, NULL);
	vkFreeMemory(m_device, m_depthMap.memory, NULL);

	vkDestroyBuffer(m_device, m_uniform.buf, NULL);
	vkFreeMemory(m_device, m_uniform.mem, NULL);

	for (i = 0; i < m_swapChainImageCount; i++) {
		vkDestroyImageView(m_device, m_swapChainImgBuffer[i].view, NULL);
	}
	vkFreeCommandBuffers(m_device, m_cmdPool, 1,
		&m_cmdBuffer);

	m_swapChainImgBuffer.clear();

	m_queueProps.clear();

	vkDestroyCommandPool(m_device, m_cmdPool, NULL);
	vkDestroyDevice(m_device, NULL);

	vkDestroySurfaceKHR(m_instance, m_surface, NULL);
	vkDestroyInstance(m_instance, NULL);
}
