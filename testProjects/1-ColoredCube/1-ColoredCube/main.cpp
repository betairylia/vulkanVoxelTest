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
	Globals
*/

VkInstance m_instance;
vector<VkPhysicalDevice> m_GPUs;

uint32_t m_queueCount;
vector<VkQueueFamilyProperties> m_queueProps;

VkDevice m_device;

///win32
HINSTANCE m_connection;
HWND m_window;

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

void initWindow(int width, int height, string windowTitle)
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
	initWindow(800, 600, "ColoredCube");

	m_instance = CreateInstance("Colored Cube");

	EnumeratePhysicalDevices(m_instance, m_GPUs);
	m_device = CreateDevice(m_GPUs, m_queueCount, m_queueProps);

	//todo...
}
