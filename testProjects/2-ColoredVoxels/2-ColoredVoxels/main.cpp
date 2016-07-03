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

#define LENGTH 255

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

#include "simpleTestPass.h"
#include "deferredTestPass.h"
#include "PostEffectTestDOFPass.h"

#include "VulkanRenderer.h"

#include "BlockGroup.h"
#include "BlockGroupToBufferWorker.h"

using namespace std;

/*
	descSet 支持Input attachment
	给framebuffer上装sampler
	试着搞一搞w
*/

/*
Globals
*/
Vertex quadVert[4] = {
	{ 1, 1, 0.5, 1, 0, 0, 0, 1},
	{-1, 1, 0.5, 0, 0, 0, 0, 1},
	{-1,-1, 0.5, 0, 1, 0, 0, 1},
	{ 1,-1, 0.5, 1, 1, 0, 0, 1}
};

uint32_t quadIndex[6] = {
	0, 1, 2, 2, 3, 0
};

bool useWireframe = true;

static Vertex surfaceData[(LENGTH + 1) * (LENGTH + 1)] = {};
static uint32_t indexData[(LENGTH * LENGTH) * 6] = {};

static const char *vertShader =
"#version 450\n"
"#extension GL_ARB_separate_shader_objects : enable\n"
"#extension GL_ARB_shading_language_420pack : enable\n"
"layout (std140, binding = 0) uniform bufferVals {\n"
"    mat4 model;\n"
"    mat4 view;\n"
"    mat4 proj;\n"
"} myBufferVals;\n"
"layout (location = 0) in vec3 pos;\n"
"layout (location = 1) in vec2 uv;\n"
"layout (location = 2) in vec3 normal;\n"
"layout (location = 3) in uint meta;\n"
"layout (location = 0) out vec3 worldPosition;\n"
"layout (location = 1) out float depth;\n"
"layout (location = 2) out vec3 worldNormal;\n"
"layout (location = 3) out vec4 outColor;\n"
"out gl_PerVertex { \n"
"    vec4 gl_Position;\n"
"};\n"
"void main() {\n"
"	worldPosition = (myBufferVals.model * vec4(pos, 1.0)).xyz;\n"
"	worldNormal = transpose(inverse(mat3(myBufferVals.model))) * normalize(normal);\n"
"   vec4 outPos = myBufferVals.proj * myBufferVals.view * myBufferVals.model * vec4(pos, 1.0);\n"
"   outColor = vec4(0.8, 1, 0.6, 1);\n"
"   depth = outPos.z / outPos.w;\n"
"   gl_Position = outPos;\n"
"}\n";

static const char *fragShader =
"#version 450\n"
"#extension GL_ARB_separate_shader_objects : enable\n"
"#extension GL_ARB_shading_language_420pack : enable\n"
"layout (location = 0) in vec3 worldPosition;\n"
"layout (location = 1) in float depth;\n"
"layout (location = 2) in vec3 worldNormal;\n"
"layout (location = 3) in vec4 color;\n"
"layout (location = 0) out vec4 outColor;\n"
"layout (location = 1) out vec4 outColor2;\n"
"layout (location = 2) out vec4 outColor3;\n"
"void main() {\n"
"   outColor = vec4(worldPosition, depth);\n"
"	outColor2 = vec4(worldNormal, 1.0);\n"
"   outColor3 = color;\n"
"}\n";

static const char *vertShaderScreenQuad =
"#version 450\n"
"#extension GL_ARB_separate_shader_objects : enable\n"
"#extension GL_ARB_shading_language_420pack : enable\n"
"layout (std140, binding = 0) uniform bufferVals {\n"
"    mat4 model;\n"
"    mat4 view;\n"
"    mat4 proj;\n"
"} myBufferVals;\n"
"layout (location = 0) in vec3 pos;\n"
"layout (location = 1) in vec2 uv;\n"
"layout (location = 2) in vec3 normal;\n"
"layout (location = 0) out vec2 outUV;\n"
"out gl_PerVertex { \n"
"    vec4 gl_Position;\n"
"};\n"
"void main() {\n"
"   outUV = vec2(uv.x, 1.0 - uv.y);\n"
"   gl_Position = vec4(pos, 1.0);\n"
"}\n";

//SSAO
static const char *fragShaderScreenQuad1 =
"#version 450\n"
"#extension GL_ARB_separate_shader_objects : enable\n"
"#extension GL_ARB_shading_language_420pack : enable\n"
"layout (binding = 1) uniform sampler2D samplerPosition;\n"
"layout (binding = 2) uniform sampler2D samplerNormal;\n"
"layout (binding = 3) uniform sampler2D samplerColor;\n"
"layout (location = 0) in vec2 uv;\n"
"layout (location = 0) out vec4 outColor;\n"
"int sampleCount = 8;\n"
"int selAngle[8] = int[](2, 4, 3, 7, 6, 5, 0, 1);\n"
"int selDist[8]  = int[](3, 2, 4, 7, 1, 6, 5, 0);\n"
"float fSelAngle = 6.2831853 / sampleCount;\n"
"float fSelDist  = 1.0 / sampleCount;\n"
"float fScaler = 0.3;\n"
"float rand(vec2 co){\n"
"    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);\n"
"}\n"
"void main() {\n"
"    vec2 vSampleVector;\n"
"    vec3 vPos = texture(samplerPosition, uv).xyz;\n"
"    vec3 vNormal = texture(samplerNormal, uv).xyz;\n"
"    float occ = 0.0;\n"
"    for(int i = 0; i < sampleCount; ++i)\n"
"    {\n"
"        int n = selAngle[i];\n"
"        int m = selDist[i];\n"
"        vSampleVector = vec2(cos(fSelAngle * n), sin(fSelAngle * n)) * (fSelDist * (m + 1)) * fScaler / vPos.z;\n"
"        float rAngle = 3.14159265 * rand(vPos.yx);\n"
"        vSampleVector = mat2(cos(rAngle), -sin(rAngle), sin(rAngle), cos(rAngle)) * vSampleVector;\n"
"        float dDist = length(vPos - texture(samplerPosition, uv + vSampleVector).xyz);\n"
"        float fNorm = max(0.0, dot(vNormal, texture(samplerNormal, uv + vSampleVector).xyz) - 0.1);\n"
"        float fDist = 1.0 / (1.0 + dDist * dDist);"
"        occ += fNorm * fDist;\n"
"    }\n"
"    occ = occ / sampleCount;\n"
"    //outColor = vec4(texture(samplerColor, uv).rgb * occ, 1.0);\n"
"    outColor = vec4(occ, occ, occ, 1.0);\n"
"}\n";

//SSSAO
static const char *fragShaderScreenQuad =
"#version 450\n"
"#extension GL_ARB_separate_shader_objects : enable\n"
"#extension GL_ARB_shading_language_420pack : enable\n"
"layout (binding = 1) uniform sampler2D samplerPosition;\n"
"layout (binding = 2) uniform sampler2D samplerNormal;\n"
"layout (binding = 3) uniform sampler2D samplerColor;\n"
"layout (location = 0) in vec2 uv;\n"
"layout (location = 0) out vec4 outColor;\n"
"int sampleCount = 8;\n"
"int selAngle[8] = int[](2, 4, 3, 7, 6, 5, 0, 1);\n"
"int selDist[8]  = int[](3, 2, 4, 7, 1, 6, 5, 0);\n"
"float fSelAngle = 6.2831853 / sampleCount;\n"
"float fSelDist  = 1.0 / sampleCount;\n"
"float fScaler = 0.8;\n"
"float fRadius = 3.0;\n"
"float rand(vec2 co){\n"
"    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);\n"
"}\n"
"void main() {\n"
"    vec2 vSampleVector;\n"
"    vec3 vPos = texture(samplerPosition, uv).xyz;\n"
"    vec3 vNormal = texture(samplerNormal, uv).xyz;\n"
"    float occ = 0.0;\n"
"    for(int i = 0; i < sampleCount; ++i)\n"
"    {\n"
"        int n = selAngle[i];\n"
"        int m = selDist[i];\n"
"        vSampleVector = vec2(cos(fSelAngle * n), sin(fSelAngle * n)) * (fSelDist * (m + 1)) * fScaler / vPos.z;\n"
"        float rAngle = 3.14159265 * rand(vPos.yx);\n"
"        vSampleVector = mat2(cos(rAngle), -sin(rAngle), sin(rAngle), cos(rAngle)) * vSampleVector;\n"
"        vec3 sPos = texture(samplerPosition, uv + vSampleVector).rgb;\n"
"        vec3 sdPos = sPos - vPos;\n"
"        float fAbsV = dot(sdPos, sdPos);\n"
"        float fRes = 0;\n"
"        fRes = max(0.0, 1.0 - sqrt(fAbsV) / fRadius) * max(0.0, dot(sdPos / sqrt(fAbsV), vNormal) - 0.01);\n"
"        occ += fScaler * fRes;\n"
"    }\n"
"    occ = 1-(occ / sampleCount);\n"
"    //outColor = vec4(texture(samplerColor, uv).rgb * occ, 1.0);\n"
"    outColor = vec4(occ, occ, occ, 1.0);\n"
"}\n";

//DOF
static const char *fragShaderTestDOF =
"#version 450\n"
"#extension GL_ARB_separate_shader_objects : enable\n"
"#extension GL_ARB_shading_language_420pack : enable\n"
"layout (binding = 1) uniform sampler2D samplerPosition;\n"
"layout (binding = 2) uniform sampler2D samplerNormal;\n"
"layout (binding = 3) uniform sampler2D samplerColor;\n"
"layout (binding = 4) uniform sampler2D samplerResult;\n"
"layout (location = 0) in vec2 uv;\n"
"layout (location = 0) out vec4 outColor;\n"
"vec2 invRes = vec2(1.0 / 140.0, 1.0 / 90.0);\n"
"mat3 kernel9 = mat3(0.0751, 0.1238, 0.0751, 0.1238, 0.2042, 0.1238, 0.0751, 0.1238, 0.0751);\n"
"void main() {\n"
"    int i, j;\n"
"    vec4 sum = vec4(0.0, 0.0, 0.0, 0.0);vec2 nUV = vec2(0.0, 0.0);vec3 vNormal = texture(samplerNormal, uv).rgb;\n"
"    for(i=-1;i<=1;++i){for(j=-1;j<=1;++j){nUV = uv + vec2(i, j) * invRes; if(dot(vNormal, texture(samplerNormal, nUV).rgb) > 0.9) sum += (texture(samplerResult,nUV)*kernel9[i+1][j+1]);}}\n"
"    outColor = texture(samplerColor, uv) * sum;\n"
"    //outColor = sum;\n"
"    //outColor = texture(samplerResult, uv);\n"
"}\n";

bool m_prepared;

///win32
HINSTANCE m_connection;
HWND m_window;

///const
const int fenceTimeout = 100000000;
const int width = 1440;
const int height = 900;

VulkanRenderer renderer;
Pipeline pipelineGBuffer, pipelineRendering, pipelineDOF;
DescSet descSet;
DescPipelineLayout layout;
vector<Renderable> renderables;
//SimpleTestPass pass;
DeferredTestPass pass;
PostEffectTestDOFPass passDOF;

VkImageView views[4];

BlockGroup blockGroup;
BlockGroupToBufferWorker worker;

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
		300, 60,								// x/y coords
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

void render()
{
	//renderer.render(layout, pass, passDOF, renderables);
	renderer.renderByChain(renderables);
}

void GenerateNewBuffers(int frame)
{
	int i, j;
	for (i = 0;i < (LENGTH + 1); i++)
	{
		for (j = 0;j < (LENGTH + 1);j++)
		{
			surfaceData[i * (LENGTH + 1) + j] = { XYZ1((float)i / (LENGTH)* 5, (float)sin((float)i / (LENGTH)* 15 + (float)frame / 200.0), (float)j / (LENGTH)* 5), XYZ1(1.f, 1.f, 1.f) };
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

LARGE_INTEGER tPrev, tNow, tC;

void update()
{
	static double fps = 0;
	static int frame = 0;

	//double fac = (double)frame / 600.0f;

	//glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
	//glm::mat4 View = glm::lookAt(
	//	glm::vec3(10 * glm::sin(fac), 6 * glm::cos(fac / 2.5), 10 * glm::cos(fac)), // Camera is at (0,3,10), in World Space
	//	glm::vec3(0, 0, 0),  // and looks at the origin
	//	glm::vec3(0, -1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	//	);
	//glm::mat4 Model = glm::mat4(1.0f);
	//// Vulkan clip space has inverted Y and half Z.
	//glm::mat4 Clip = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
	//	0.0f, -1.0f, 0.0f, 0.0f,
	//	0.0f, 0.0f, 0.5f, 0.0f,
	//	0.0f, 0.0f, 0.5f, 1.0f);

	//glm::mat4 MVP = Clip * Projection * View * Model;

	//uint8_t *pData;
	//VkResult res = vkMapMemory(m_device, m_uniform.mem, 0, m_uniform.mem_reqs.size, 0, (void **)&pData);
	//assert(res == VK_SUCCESS);

	//memcpy(pData, (const void*)&MVP, sizeof(MVP));

	//vkUnmapMemory(m_device, m_uniform.mem);
/*
	GenerateNewBuffers(frame);

	uint8_t *pData;

	VkResult res = vkMapMemory(renderer.m_device, renderables[0].vertexBuffer.mem, 0, renderables[0].vertexBuffer.mem_reqs.size, 0,
		(void **)&pData);
	assert(res == VK_SUCCESS);

	memcpy(pData, (const void*)&surfaceData, sizeof(surfaceData));

	vkUnmapMemory(renderer.m_device, renderables[0].vertexBuffer.mem);
*/

	if (frame % 25 == 0)
	{
		blockGroup.generateTestChunk((double)frame / 200);
		worker.workUpdate(renderer.m_device, renderables[0], blockGroup);

		blockGroup.generateTestChunk((double)frame / 400);
		worker.workUpdate(renderer.m_device, renderables[1], blockGroup);
	}

	renderables[0].UpdatePosition(   5 +   cos((double)frame / 300)  * 5, -20, -40);
	renderables[1].UpdatePosition( -37 + (-cos((double)frame / 300)) * 5, -20, -40);

	QueryPerformanceCounter(&tNow);
	fps = 1.000 / ((tNow.QuadPart - tPrev.QuadPart) * 1.0 / tC.QuadPart);
	QueryPerformanceCounter(&tPrev);

	if (frame % 300 == 0)
	{
		printf("fps = %lf\n", fps);
	}

	frame++;
	return;
}

static void run()
{
	if (!m_prepared) return;

	vkDeviceWaitIdle(renderer.m_device);

	update();
	render();

	VkResult res = vkDeviceWaitIdle(renderer.m_device);
	assert(res == VK_SUCCESS);
}

/*
Main
*/

int main(int argc, char *argv[])
{
	QueryPerformanceFrequency(&tC);

	InitWindow(width, height, "ColoredCube");

	renderer.m_connection = m_connection;
	renderer.m_window = m_window;

	renderer.init();

	renderer.BeginCommandBuffer(renderer.m_cmdBuffer);

	renderer.InitRenderChain(
		quadVert, sizeof(quadVert), sizeof(quadVert[0]),
		quadIndex, sizeof(quadIndex), sizeof(quadIndex[0]));

	//GenerateNewBuffers(0);

	layout.m_device = renderer.m_device;
	layout.init();

	//renderer.layoutIA.m_device = renderer.m_device;
	//renderer.layoutIA.initIA();

	Renderable r;

	r.init(renderer.m_device, renderer.m_descPool, layout.descLayout, renderer.m_memoryProperties);
	r.UpdatePosition(-16, -20, -70);

	r.SetVertexBuffer(renderer.m_memoryProperties, surfaceData, sizeof(surfaceData), sizeof(surfaceData[0]), renderer.m_viBinding, renderer.m_viAttribs);
	r.SetIndexBuffer(renderer.m_memoryProperties, indexData, sizeof(indexData), sizeof(indexData[0]));

	renderables.push_back(r);

	Renderable r2;

	r2.init(renderer.m_device, renderer.m_descPool, layout.descLayout, renderer.m_memoryProperties);
	r2.UpdatePosition(16, -20, -70);

	r2.SetVertexBuffer(renderer.m_memoryProperties, surfaceData, sizeof(surfaceData), sizeof(surfaceData[0]), renderer.m_viBinding, renderer.m_viAttribs);
	r2.SetIndexBuffer(renderer.m_memoryProperties, indexData, sizeof(indexData), sizeof(indexData[0]));

	renderables.push_back(r2);

	blockGroup.generateTestChunk();
	worker.workUpdate(renderer.m_device, renderables[0], blockGroup);
	worker.workUpdate(renderer.m_device, renderables[1], blockGroup);

	//pass.SetBase(renderer.m_device, renderer.m_GPUs[0], renderer.m_memoryProperties, renderer.m_cmdBuffer, renderer.m_queue, width, height);
	//pass.initPass();

	//passDOF.SetBase(renderer.m_device, renderer.m_GPUs[0], renderer.m_memoryProperties, renderer.m_cmdBuffer, renderer.m_queue, width, height);
	//passDOF.initPass(renderer.m_swapChainImgBuffer.data(), renderer.m_swapChainImageCount, renderer.m_colorImgFormat);

	////Screen quad
	//views[0] = pass.bufferImages.position.view;
	//views[1] = pass.bufferImages.normal.view;
	//views[2] = pass.bufferImages.color.view;
	//views[3] = pass.bufferImages.output.view;
	//renderer.screenAlignedQuad.initIA(renderer.m_device, renderer.m_descPool, renderer.layoutIA.descLayout, renderer.m_memoryProperties, renderer.simpleSampler, views, VK_IMAGE_LAYOUT_GENERAL);
	//renderer.screenAlignedQuad.SetVertexBuffer(renderer.m_memoryProperties, quadVert, sizeof(quadVert), sizeof(quadVert[0]), renderer.m_viBinding, renderer.m_viAttribs);
	//renderer.screenAlignedQuad.SetIndexBuffer(renderer.m_memoryProperties, quadIndex, sizeof(quadIndex), sizeof(quadIndex[0]));
	//renderer.screenAlignedQuad.SetScreenQuad(renderer.m_memoryProperties);

	////Pipeline A
	//pipelineGBuffer.m_device = renderer.m_device;
	//pipelineGBuffer.InitShader(vertShader, fragShader);
	//pipelineGBuffer.initPipeline(renderer.m_viBinding, renderer.m_viAttribs, layout.pipelineLayout, pipelineGBuffer.info, pass.renderPass, 3, 0);

	////Pipeline B
	//pipelineRendering.m_device = renderer.m_device;
	//pipelineRendering.InitShader(vertShaderScreenQuad, fragShaderScreenQuad);
	//pipelineRendering.initPipeline(renderer.m_viBinding, renderer.m_viAttribs, renderer.layoutIA.pipelineLayout, pipelineRendering.info, pass.renderPass, 1, 1);
	//
	//pass.subPassCount = 2;
	//pass.pipelines = (Pipeline*)malloc(sizeof(Pipeline) * 2);
	//
	//pass.pipelines[0] = pipelineGBuffer;
	//pass.pipelines[1] = pipelineRendering;

	////Pipeline DOF
	//pipelineDOF.m_device = renderer.m_device;
	//pipelineDOF.InitShader(vertShaderScreenQuad, fragShaderTestDOF);
	//pipelineDOF.initPipeline(renderer.m_viBinding, renderer.m_viAttribs, renderer.layoutIA.pipelineLayout, pipelineDOF.info, passDOF.renderPass, 1, 0);

	//passDOF.subPassCount = 1;
	//passDOF.pipelines = (Pipeline*)malloc(sizeof(Pipeline));

	//passDOF.pipelines[0] = pipelineDOF;

	//descSet.m_device = renderer.m_device;
	//descSet.setDescPool(renderer.m_descPool);
	//descSet.CreateDescriptorSet(layout.descLayout, renderables[0].uniformBuffer);

	renderer.EndCommandBuffer(renderer.m_cmdBuffer);
	renderer.QueueCommandBuffer(renderer.m_cmdBuffer, renderer.m_device, renderer.m_queue);

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
	/*uint32_t i;

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
	*//*
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
	vkDestroyInstance(m_instance, NULL);*/
}
