#pragma once


#define GLFW_INCLUDE_VULKAN
#include <glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtx/rotate_vector.hpp>
#include <GLM/gtc/quaternion.hpp>
#include <GLM/gtx/quaternion.hpp>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <array>
#include <optional>
#include <set>

#include "GLFW_Window.h"
#include "VulkanObject.h"
#include "VulkanEngine.h"

#include "Lighting.h"



/*! Uniform Buffer Object struct
	Holds the model, view and projection matrix
*/
struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 lightRot;
	glm::mat4 lightSpace;
	glm::mat4 lightViewProj;

	//Lighting
	glm::vec4 AmbientColour;
	glm::vec4 DirectionalColour;
};
struct OffScreenUniformBufferObject {
	glm::mat4 depthMVP;
};
struct GBufferUniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};




/*! Vulkan App
	Handling all vulkan code for displaying a simple pyrimid 
*/
class VulkanApp {
	struct FrameBufferAttachment {
		VkImage image;
		VkDeviceMemory mem;
		VkImageView view;
		VkFormat format;
	};
	struct OffscreenPass {
		int32_t width, height;
		VkFramebuffer frameBuffer;
		FrameBufferAttachment depth;
		VkRenderPass renderPass;
		VkSampler depthSampler;
		VkDescriptorImageInfo descriptor;
	} offscreenPass;


	struct GFrameBuffer {
		VkFramebuffer frameBuffer;
		FrameBufferAttachment position, normal, albedo;
		FrameBufferAttachment depth;
		VkRenderPass renderPass;
	} offScreenFrameBuf;
	// One sampler for the frame buffer color attachments
	VkSampler colourSampler;
private:

	/*! Queue Family Indices Struct
		Holds the graphics and present families data
	*/
	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		//Check if a complete index
		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	/*! Swap Chain Support Details Struct
		Holds the formats and present modes for the swap chain
	*/
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	/*! The GLFW Window used for drawing and call backs*/
	GLFW_Window *window;
	/*! Vulkan instance for accessing the vulkan api with the correct settings */
	VkInstance instance;
	/*! Vulkan Debug Messenger for printing out errors from he validation layers */
	VkDebugUtilsMessengerEXT debugMessenger;
	/*! The physical device (gpu) used for rendering */
	VkPhysicalDevice physicalDevice;
	/*! The logical device for interfacing with the physical hardware */
	VkDevice device;
	/*! The graphics queue used for rending a single object with the provided vertex and fragment shaders */
	VkQueue graphicsQueue;

	/*! The vulkan surface we can render too */
	VkSurfaceKHR surface;
	/*! Handle for accessing the presentation queue */
	VkQueue presentQueue;

	/*! The swap chain that stores the framebuffers we will render too */
	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages; //Vector of each image we will render
	VkFormat swapChainImageFormat; //Image formatting
	VkExtent2D swapChainExtent; //Resolution
	std::vector<VkImageView> swapChainImageViews; //Descriptors as to how to view the images in the swap chain
	std::vector<VkFramebuffer> swapChainFramebuffers; //The frame buffer objects for displaying the images

	
	/*! Shader modules for the Vertex and Fragment stages */
	VkShaderModule vertShaderModule;
	VkShaderModule fragShaderModule;


	/*! The standard validation layer */
	const std::vector<const char*> validationLayers = {
		"VK_LAYER_LUNARG_standard_validation"
	};
	/*! List of needed device extentions */
	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	/*! The render pass contain the information about the frame buffer attachments we use while rendering*/
	VkRenderPass renderPass;
	VkPipelineLayout pipelineLayout; //The pipeline layout

	/*! Graphics pipeline that contains the sequence of opertations used to render vertex information to the screen */
	VkPipeline GBufferGraphicsPipeline; 
	VkPipeline graphicsPipeline;

	/*! The command pool that holds all the command buffers we will use for each frame */
	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers; //List of the command buffers, each containing the infomation of the commands to be carried out each frame (e.g. drawing, memory transfer etc)
	std::vector<VkFence> inFlightFences; //Fences used to halt the command buffers from executing until the previos frame has completed
	std::vector<VkSemaphore> imageAvailableSemaphores; //List of semaphores to signel if an image is available to render too (GPU Syncing)
	std::vector<VkSemaphore> renderFinishedSemaphores; //List of semaphores to signel when the image is finished and can be presented (GPU Syncing)

	//Number of frames we can have being held at one time
	const int MAX_FRAMES_IN_FLIGHT = 2;
	size_t currentFrame = 0;

	
	//Disable validation layers in release mode
	#ifdef NDEBUG
		const bool enableValidationLayers = false;
	#else
		const bool enableValidationLayers = true;
	#endif

	//Custom Stuff

		VulkanEngine* m_Engine;


public:
	VulkanApp() {};
	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

	bool framebufferResized = false;

private:
	const void initWindow();	
	const void initVulkan();
	const void mainLoop();
	const void cleanup();

	const void createInstance();
	bool checkValidationLayerSupport();
	std::vector<const char*> getRequiredExtensions();
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

	void setupDebugMessenger();
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger);

	void DestroyDebugUtilsMessengerEXT(VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

	void pickPhysicalDevice();
	bool isDeviceSuitable(VkPhysicalDevice device);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

	void createLogicalDevice();

	void createSurface();

	bool checkDeviceExtensionSupport(VkPhysicalDevice device);

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	void createSwapChain();

	void createImageViews();

	void createGraphicsPipeline();
	VkShaderModule createShaderModule(const std::vector<char>& code);

	void createRenderPass();

	void createFramebuffers();

	void createCommandPool();
	void createCommandBuffers();

	void drawFrame();

	void createSyncObjects();

	void recreateSwapChain();
	void cleanupSwapChain();

	

	//Uniform layouts
	VkDescriptorSetLayout descriptorSetLayout;
	void createDescriptorSetLayout();

	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	

	void createUniformBuffers();
	void updateUniformBuffer(uint32_t currentImage, unsigned int objectIndex);

	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;
	VkDescriptorSet finalRSet;

	void createDescriptorPool();
	void createDescriptorSets();

	//Depth Buffering
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	void createDepthResources();
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	VkFormat findDepthFormat();

	//Custom

	std::vector<VulkanObject*> m_Objects;

	VkViewport viewport;

	//Shadows
	void prepareOffscreenRenderpass();
	void prepareOffscreenFramebuffer();
	VkPipeline offscreenPipeline;
	VkPipelineLayout offscreenPipelineLayout; //The pipeline layout
	std::vector<VkBuffer> offscreenUniforms;
	std::vector<VkDeviceMemory> offscreenMemorys;
	std::vector<VkDescriptorSet> offscreenDescSets;
	std::vector<OffScreenUniformBufferObject> offscreenUBOs;

	//MSAA
	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;

	VkSampleCountFlagBits getMaxUsableSampleCount();
	void createColorResources();

	//GBuffer
	void CreateGAttachment(
		VkFormat format,
		VkImageUsageFlagBits usage,
		FrameBufferAttachment *attachment);

	void prepareGOffscreenFramebuffer();
	void CleanGBuffer();
	void UpdateGBufferSets();
	VkBuffer GBUniform;
	VkDeviceMemory GBUniformMemory;
	GBufferUniformBufferObject GBubo;
	
};