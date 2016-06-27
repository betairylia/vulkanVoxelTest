#pragma once

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

#include "structs.h"
#include "vHelper.h"

#include "Pipeline.h"
#include "DescSet.h"
#include "Renderable.h"
#include "DescPipelineLayout.h"
#include "RenderPass.h"

using namespace std;

/*
	Class
*/

class VulkanRenderer
{
public:
	VulkanRenderer();
	~VulkanRenderer();

	void init();

	//Vulkan processes

	const char* GetRawTextFromFile(const char* fileName);
	void initResources(TBuiltInResource & Resources);

	EShLanguage FindLanguage(const VkShaderStageFlagBits shader_type);
	bool GLSLtoSPV(const VkShaderStageFlagBits shader_type, const char * pshader, std::vector<unsigned int>& spirv);

	void BeginCommandBuffer(VkCommandBuffer & cmdBuf);
	void EndCommandBuffer(VkCommandBuffer & cmdBuf);
	void QueueCommandBuffer(VkCommandBuffer & cmdBuf, VkDevice & device, VkQueue & queue);

	VkInstance CreateInstance(string instanceName);
	void EnumeratePhysicalDevices(VkInstance & inst, vector<VkPhysicalDevice>& gpus, VkPhysicalDeviceMemoryProperties & memory_properties, VkPhysicalDeviceProperties & gpu_props);
	VkDevice CreateDevice(vector<VkPhysicalDevice>& gpus, uint32_t & queueCount, vector<VkQueueFamilyProperties>& queueProps, uint32_t & familyIndex);
	VkCommandPool CreateCommandPool(const uint32_t queueFamilyIndex, VkDevice & device);
	VkCommandBuffer CreateCommandBuffer(VkCommandPool & cmdPool, VkDevice & device);
	VkSurfaceKHR GetSurface(HINSTANCE hinstance, HWND window, VkInstance & instance);
	uint32_t GetGraphicsPresentQueueIndex(const uint32_t queueCount, VkPhysicalDevice gpu, vector<VkQueueFamilyProperties> queueProps, VkSurfaceKHR surface);
	VkSwapchainKHR CreateSwapChain(VkPhysicalDevice gpu, VkSurfaceKHR surface, VkFormat & format, int width, int height, VkDevice device, uint32_t & swapChainImageCount, vector<SwapChainBuffer>& buffer, vector<SwapChainBuffer> &buffer2, VkCommandBuffer & cmdBuf, VkQueue & queue, const uint32_t queueFamilyIndex);
	DepthMap CreateDepthMap(VkPhysicalDevice gpu, VkDevice device, int width, int height, VkPhysicalDeviceMemoryProperties & memory_properties, VkCommandBuffer & cmdBuf, VkQueue & queue);
	VkRenderPass CreateRenderPass(VkFormat imgFormat, DepthMap & depth, VkDevice device);
	VkFramebuffer * CreateFrameBuffers(VkDevice device, DepthMap & depth, SwapChainBuffer * buffers, SwapChainBuffer *buffers2, VkRenderPass & render_pass, int width, int height, int swapchainImageCount);
	VkDescriptorPool CreateDescriptorPool(VkDevice device);
	void initViewports(int width, int height, VkCommandBuffer cmdBuf, VkViewport & viewport);
	void initScissors(int width, int height, VkCommandBuffer cmdBuf, VkRect2D & scissor);

	VkImage CreateEmptyImage(uint32_t width, uint32_t height, VkFormat format);
	
	void render(DescPipelineLayout layout, RenderPass pass, RenderPass passDOF, vector<Renderable> renderableList);

//private:
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
	vector<SwapChainBuffer> m_swapChainImgBuffer, m_swapChainImgBuffer2;
	uint32_t m_currentBuffer;

	DepthMap m_depthMap;

	VkRenderPass m_renderPass;

	VkFramebuffer *m_framebuffers;

	VkVertexInputBindingDescription m_viBinding;
	VkVertexInputAttributeDescription m_viAttribs[4];

	VkDescriptorPool m_descPool;

	VkViewport m_viewport;
	VkRect2D m_scissor;

	bool m_prepared;

	Renderable screenAlignedQuad;
	DescPipelineLayout layoutIA;
	VkSampler simpleSampler;

	///win32
	HINSTANCE m_connection;
	HWND m_window;

	int fenceTimeout = 100000000;
	int width = 1440;
	int height = 900;

	bool useWireframe;
};

