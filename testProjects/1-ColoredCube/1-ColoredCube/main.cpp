#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <cassert>
#include <vulkan/vulkan.h>
#include <vulkan/vk_sdk_platform.h>

using namespace std;

/*
	Globals
*/

VkInstance m_instance;
vector<VkPhysicalDevice> m_GPUs;

uint32_t m_queueCount;
vector<VkQueueFamilyProperties> m_queueProps;

VkDevice m_device;


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

	// initialize the VkInstanceCreateInfo structure
	VkInstanceCreateInfo inst_info = {};
	inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	inst_info.pNext = NULL;
	inst_info.flags = 0;
	inst_info.pApplicationInfo = &app_info;
	inst_info.enabledExtensionCount = 0;
	inst_info.ppEnabledExtensionNames = NULL;
	inst_info.enabledLayerCount = 0;
	inst_info.ppEnabledLayerNames = NULL;

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

void EnumeratePhysicalDevices(VkInstance &inst, vector<VkPhysicalDevice> &gpus)
{
	uint32_t gpu_count = 1;
	VkResult res = vkEnumeratePhysicalDevices(inst, &gpu_count, NULL);
	assert(gpu_count);
	gpus.resize(gpu_count);
	res = vkEnumeratePhysicalDevices(inst, &gpu_count, gpus.data());
	assert(!res && gpu_count >= 1);
}

VkDevice CreateDevice(vector<VkPhysicalDevice> &gpus, uint32_t &queueCount, vector<VkQueueFamilyProperties> &queueProps)
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
			queue_info.queueFamilyIndex = i;
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


/*
	Main
*/

int main(int argc, char *argv[])
{
	m_instance = CreateInstance("Colored Cube");

	EnumeratePhysicalDevices(m_instance, m_GPUs);
	m_device = CreateDevice(m_GPUs, m_queueCount, m_queueProps);

	//todo...
}