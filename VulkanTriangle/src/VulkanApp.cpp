#include <VulkanApp.h>

static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {

	//Get the glfw window pointer cast to the vulkan app class, enable resizing
	auto app = reinterpret_cast<VulkanApp*>(glfwGetWindowUserPointer(window));
	app->framebufferResized = true;
}


static std::vector<char> readFile(const std::string& filename) {

	//Open a file stream with the binary setting
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) { //Check if file opened
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg(); //Get file size
	std::vector<char> buffer(fileSize); //Allocate enough memory

	file.seekg(0); //Go to the start of the files
	file.read(buffer.data(), fileSize); //Read all data into the buffer

	file.close(); //close file stream

	return buffer; //Return character buffer
}

const void VulkanApp::initWindow()
{
	window = new GLFW_Window(1280, 720, "Subsurface Scattering"); //Open the GLFW window with a given size and name

	glfwSetWindowUserPointer(window->Window(), this); //Set the window pointer to this class (VulkanApp)
	glfwSetFramebufferSizeCallback(window->Window(), framebufferResizeCallback); //Set resize call back to given function
}

const void VulkanApp::initVulkan() {

	createInstance();
	setupDebugMessenger();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createSwapChain();
	createImageViews();
	createRenderPass();
	createCommandPool();
	createColorResources();
	CreateSSFrameBuffer();
	prepareGOffscreenFramebuffer();
	createDescriptorSetLayout();
	prepareOffscreenFramebuffer();
	createGraphicsPipeline();
	


	//Creaate Objects after setting up required components


	

	m_Objects.push_back(new VulkanObject(m_Engine, device, graphicsQueue, commandPool, "models/plane.obj", "textures/Background.png", "textures/white.png", "textures/white.png"));
	m_Objects[0]->SetPos(glm::vec3(0.0f, -2.5f, -25));
	m_Objects[0]->SetRot(glm::vec3(0, 0.0f, 0));
	m_Objects[0]->SetScale(glm::vec3(24.0f, 13.5f, 1.5f));
	m_Objects[0]->SetLit(false);

	m_Objects.push_back(new VulkanObject(m_Engine, device, graphicsQueue, commandPool, "models/headLow.obj", "textures/headC.jpg", "textures/headN.jpg", "textures/headS.jpg"));
	m_Objects[1]->SetPos(glm::vec3(0.0f, -0.135, 0));
	m_Objects[1]->SetRot(glm::vec3(0, 0.0f, 0));
	m_Objects[1]->SetScale(glm::vec3(0.175f, 0.175f, 0.175f));

	

	m_Objects.push_back(new VulkanObject(m_Engine, device, graphicsQueue, commandPool, "models/Light.obj", "textures/white.png", "textures/handN.png", "textures/handS.png"));
	m_Objects[2]->SetPos(glm::vec3(0.0f, -0.15f, 0));
	m_Objects[2]->SetRot(glm::vec3(0, 0.0f, 0));
	m_Objects[2]->SetScale(glm::vec3(0.02f, 0.02f, 0.02f));
	m_Objects[2]->SetLit(false);

	

	
	createDepthResources();
	createFramebuffers();
	
	createUniformBuffers();
	createDescriptorPool();
	createDescriptorSets();
	createCommandBuffers();
	createSyncObjects();

}

const void VulkanApp::createInstance()
{

	//Check if validation layers are supported, if not throw an error
	if (enableValidationLayers && !checkValidationLayerSupport()) {
		throw std::runtime_error("validation layers requested, but not available!");
	}

	//Set up app info, this is optional but useful
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Subsurface Scattering";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	//Create instance info, used for getting GLFW extentions
	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;

	//Get required  instance extention
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	//Pass extention count and names to info
	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;

	createInfo.enabledLayerCount = 0;

	//If using validation layers pass in validation data to info
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	//Get required extention
	auto extensionsReq = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensionsReq.size()); //Set data
	createInfo.ppEnabledExtensionNames = extensionsReq.data();

	//Create instance using set data/info
	VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);

	//Check if the instance was created if note throw error
	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}



	uint32_t extensionCount = 0;
	//Get extention proerties count
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> extensions(extensionCount);
	//Use count to actaully get the data
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());


	//Print out extention info to console
	std::cout << "available extensions:" << std::endl;

	for (const auto& extension : extensions) {
		std::cout << "\t" << extension.extensionName << std::endl;
	}

	m_Engine = new VulkanEngine(physicalDevice, device);
}

bool VulkanApp::checkValidationLayerSupport()
{

	uint32_t layerCount;
	//Get layer count
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	//Allocate memory 
	std::vector<VkLayerProperties> availableLayers(layerCount);
	//Get player properties
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	//Go through each layer and check if it is supported return false is a required layer is not supported
	for (const char* layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

const void VulkanApp::mainLoop() {
	//while window should not close keep looping
	while (!window->ShouldClose())
	{
		//Update window
		window->UpdateWindow();
		//Draw frame
		drawFrame();
	}
	//Wait for last frame to be processed before ending
	vkDeviceWaitIdle(device);
}

void VulkanApp::drawFrame() {
	
	//Wait for current frame to be processed before drawing a new one (stop memory leak)
	vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());

	//Get next image to render too, if failed recreate swap chain and wait till next frame
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(device, swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	for (unsigned int j = 0; j < m_Objects.size(); j++)
	{
		//Update shader buffers
		updateUniformBuffer(imageIndex, j);
	}
	framecount++;
	//Set up submit info
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	//Pass in sync data (semiphores)
	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	//Pass in command buffer data
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

	//Reset wait fence 
	vkResetFences(device, 1, &inFlightFences[currentFrame]);


	//Submit data, graphics queue
	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}


	//Set up presentation settings
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = &imageIndex;

	//Add info to queue
	result = vkQueuePresentKHR(presentQueue, &presentInfo);

	//Check if not valid or if the framebuffer has been resized, is not recreate swap chain
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
		framebufferResized = false;
		recreateSwapChain();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}

	//Update frame count
	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}



const void VulkanApp::cleanup() {

	//Clean up memory from swap chain
	cleanupSwapChain();

	//Clean up descipter pool memory
	vkDestroyDescriptorPool(device, descriptorPool, nullptr);

	//Clean up layout memory
	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
	//Clean up shader buffers
	for (size_t i = 0; i < swapChainImages.size(); i++) {
		for (unsigned int j = 0; j < m_Objects.size(); j++)
		{
			unsigned int index = m_Objects.size() * i + j;
			vkDestroyBuffer(device, uniformBuffers[index], nullptr);
			vkFreeMemory(device, uniformBuffersMemory[index], nullptr);
		}
	}
	for (unsigned int j = 0; j < m_Objects.size(); j++)
	{
		vkDestroyBuffer(device, offscreenUniforms[j], nullptr);
		vkFreeMemory(device, offscreenMemorys[j], nullptr);
		
	}
	vkDestroyImage(device, offscreenPass.depth.image, nullptr);
	vkDestroySampler(device, offscreenPass.depthSampler, nullptr);

	delete m_Objects[0];
	delete m_Objects[1];
	delete m_Objects[2];
	

	vkDestroyImageView(device, offscreenPass.depth.view, nullptr);
	vkFreeMemory(device, offscreenPass.depth.mem, nullptr);
	vkDestroyFramebuffer(device, offscreenPass.frameBuffer, nullptr);
	
	vkDestroyRenderPass(device, offscreenPass.renderPass, nullptr); //Clean up render pass data
	//Clean up semaphore/sync objects
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(device, inFlightFences[i], nullptr);
	}

	//clean up command pools
	vkDestroyCommandPool(device, commandPool, nullptr);

	//Clean up GBuffer
	CleanGBuffer();
	vkDestroyBuffer(device, GBUniform, nullptr);
	vkFreeMemory(device, GBUniformMemory, nullptr);


	//Clean up sss
	subsurfaceManager.CleanUp(device);

	//Clean up device
	vkDestroyDevice(device, nullptr);


	//Clean up debugging
	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}

	//Clean up instance.surface
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);

	//Clean up glfw window
	delete window;
}

std::vector<const char*> VulkanApp::getRequiredExtensions() {

	//Get extention count
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	//Allocate vector memory
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	//Add extention names too vector
	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	//Return vector
	return extensions;
}


VKAPI_ATTR VkBool32 VKAPI_CALL VulkanApp::debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	//Return error message from validation layer
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

void VulkanApp::setupDebugMessenger() {
	
	//Ignore validation if disabled
	if (!enableValidationLayers) return;

	//Set debugging flags
	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback; //Set call back function

	//Create debug messenger, error is the creation fails
	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}

}

VkResult VulkanApp::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	

	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void VulkanApp::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	
	//Get destroy proxy function for cleaning up the debug messenger
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

void VulkanApp::pickPhysicalDevice() {

	//Get physical device count
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	//If 0 error, no vulkan enabled GPUs!
	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	//Allocate enough memory to store device data and pass data into the vector
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	//Loop through devices and look for a suitable device, just use first device found for now
	for (const auto& device : devices) {
		if (isDeviceSuitable(device)) {
			physicalDevice = device;
			msaaSamples =  getMaxUsableSampleCount(); //  VK_SAMPLE_COUNT_1_BIT;//
			break;
		}
	}

	//If no divices are suitable, throw and error
	if (physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}

	//Print device count
	std::cout << "Count: " << deviceCount << " " << physicalDevice << std::endl;
}

bool VulkanApp::isDeviceSuitable(VkPhysicalDevice device) {

	//Check which queue families are support to check that all available commands are available
	QueueFamilyIndices indices = findQueueFamilies(device);

	//Check tha all extentions needed are supported
	bool extensionsSupported = checkDeviceExtensionSupport(device);

	//Make sure the swap chain is supported on the device
	bool swapChainAdequate = false;
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	//Only return true if all conditions are met
	return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

bool VulkanApp::checkDeviceExtensionSupport(VkPhysicalDevice device) {

	//Get extention count
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	//Allocate vector memory and get extention data
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	//Get a vector of all required extentions
	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	//Go through and remove extention names in teh vector
	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}
	
	//If all the devices names are erased then all are supported, return true
	return requiredExtensions.empty();
}

VulkanApp::QueueFamilyIndices VulkanApp::findQueueFamilies(VkPhysicalDevice device) {
	QueueFamilyIndices indices;

	//Get queue count
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	//Allocate memory and store family queue data
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	//Loop through all retreived queues
	int i = 0;
	for (const auto& queueFamily : queueFamilies) {

		//Check for graphics flag and update graphics family
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}
		//Check if the support is present
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

		//IF support is found and queue count is greater than 0 update presentFamily
		if (queueFamily.queueCount > 0 && presentSupport) {
			indices.presentFamily = i;
		}

		//If all data is complete break the loop
		if (indices.isComplete()) {
			break;
		}

		i++;
	}

	return indices;
}

void VulkanApp::createLogicalDevice() {

	//Get queue family data
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

	//Allocate memory and set the graphics and present data
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	//Loop through each family and set up queue info and add them to the vector
	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}



	VkPhysicalDeviceFeatures deviceFeatures = {};

	deviceFeatures.wideLines = VK_TRUE;

	//Set up logical device info
	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	//Pass queue data
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	//Set Enabled devices features
	createInfo.pEnabledFeatures = &deviceFeatures;

	//Set extentions
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();


	//Set up validation layers for logical device if enabled
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	//Create deivce and check to make sure it was created, otherwise throw and error
	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

void VulkanApp::createSurface() {

	//Create window rendering surface using vulkan instance infomation 
	if (glfwCreateWindowSurface(instance, window->Window(), nullptr, &surface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
	}
}

VulkanApp::SwapChainSupportDetails VulkanApp::querySwapChainSupport(VkPhysicalDevice device) {
	
	//Query the the basic surface capabilities
	SwapChainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	//Get the supported surface formats
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	//Query the supported presentation modes
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}

	//Return swap chain details
	return details;
}

VkSurfaceFormatKHR VulkanApp::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	
	//In the case that the surface has no prefered formats, just choose one to use
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
		return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	//Otherwise we need to check if the preferred combinations is available
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM  && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR VulkanApp::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes) {

	//Set best mode to VK_PRESENT_MODE_FIFO_KHR as it is garenteed to be available
	VkPresentModeKHR bestMode = VK_PRESENT_MODE_IMMEDIATE_KHR;// VK_PRESENT_MODE_FIFO_KHR;

	//Loop through and check for VK_PRESENT_MODE_MAILBOX_KHR for triple buffering
	//for (const auto& availablePresentMode : availablePresentModes) {
	//	if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
	//		return availablePresentMode;
	//	}//Otherwise if not supported, try to get the basic mode for displaying images straight away
	//	else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
	//		bestMode = availablePresentMode;
	//	}
	//}

	return bestMode;
}

VkExtent2D VulkanApp::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
	
	//If the current resolution is max, keep the current resolution
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else { //Otherwise query the framebuffer size from glfw to return the window resolution
		int width, height;
		glfwGetFramebufferSize(window->Window(), &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		return actualExtent;
	}
}

void VulkanApp::createSwapChain() {

	//Check for swap chain support data
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

	//Set up the formats from the swap chain details
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	//Request more than the minimum images to make sure we dont stall waiting for another image to render too
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

	//Make sure we don't exceed the maximum image count
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	//Set up swap chain info struct
	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface; //Give it the rendering surface

	createInfo.minImageCount = imageCount; //Swapchain image count
	createInfo.imageFormat = surfaceFormat.format; 
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent; //Resolution
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	//Get and set up the queue families supported on the selected physical device
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	//USe current transform, we don't want to transform the resulting images at all
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //Set up alpha blending
	createInfo.presentMode = presentMode; //Pass in resent mode
	createInfo.clipped = VK_TRUE; //Clip pixels that are obscured

	//For now only ever create one swap chain
	createInfo.oldSwapchain = VK_NULL_HANDLE;


	//Create swap chain and error check
	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	//Set up memory for the swap chain images and store the inital data
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
}

void VulkanApp::createImageViews() {
	
	//Allocate enough memory for each image
	swapChainImageViews.resize(swapChainImages.size());

	//For each image set up the info struct and create the image view
	for (uint32_t i = 0; i < swapChainImages.size(); i++) {
		swapChainImageViews[i] = m_Engine->createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	}

}

void VulkanApp::createGraphicsPipeline() {

	//Read in shader files
	//Base mesh and Shell rendering
	auto vertShaderCode = readFile("shaders/GBVert.spv");
	auto fragShaderCode = readFile("shaders/GBFrag.spv");


	//Set up shader modules for both vertex and fragment shaders
	VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

	//Set up vertex info
	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; //Vertex stage
	vertShaderStageInfo.module = vertShaderModule; //Vertex shader
	vertShaderStageInfo.pName = "main"; //Main function as entry point

	//Set up fragment info
	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT; //Fragment stage
	fragShaderStageInfo.module = fragShaderModule; //Fragment shader
	fragShaderStageInfo.pName = "main"; //Main function as entry point

	//List shader stage info in order
	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	//Set up vertex input pipline info
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	//Get descriptions
	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription; //Give binding desc
	vertexInputInfo.pVertexAttributeDescriptions = &attributeDescriptions[0]; //Give attribute data

	//Assembly state info (rendering type)
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; //Rendering using triangle lists
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	//Set up viewport
	viewport = {};
	viewport.x = 0.0f; //No offset
	viewport.y = 0.0f;//No offset
	//Resolution
	viewport.width = (float)swapChainExtent.width; 
	viewport.height = (float)swapChainExtent.height;
	//Depth buffer range
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	//No scissor ( dont want to cut any of the image out)
	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChainExtent;

	//Viewport info
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	//Rasterizer info
	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE; //Dont clamp depth
	rasterizer.rasterizerDiscardEnable = VK_FALSE; //Dont discard (wont draw to framebuffer otherwise)
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL; //We want to draw full polygons not lines or points
	rasterizer.lineWidth = 1.0f; 
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; //Cull back faces
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; //Set clockwise 
	rasterizer.depthBiasEnable = VK_FALSE;

	//Default multisamplign settings 
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = msaaSamples;

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional

	//Set up colour blending settings (Default blending settings)
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 3;
	std::array<VkPipelineColorBlendAttachmentState, 3> blendAttachmentStates = { colorBlendAttachment ,colorBlendAttachment ,colorBlendAttachment };
	colorBlending.pAttachments = blendAttachmentStates.data();
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	//Layout info (mainly default)
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

	std::vector<VkDynamicState> dynamicStateEnables = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR,
			VK_DYNAMIC_STATE_LINE_WIDTH
	};
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pDynamicStates = dynamicStateEnables.data();
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
	dynamicState.flags = 0;

	//Create layout and error check
	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}
	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &offscreenPipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create offscreen pipeline layout!");
	}

	//Set up graphics pipline info
	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2; //3 shader stages
	//Pass in all data set up before this
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = offScreenFrameBuf.renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; //Only going to be using one pipline for now, dont need to refrence the base
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pDynamicState = &dynamicState;

	uint32_t enablePCF = 1;
	VkSpecializationMapEntry specializationMapEntry{};
	specializationMapEntry.constantID = 0;
	specializationMapEntry.offset = 0;
	specializationMapEntry.size = sizeof(uint32_t);
	VkSpecializationInfo specializationInfo{};
	specializationInfo.mapEntryCount = 1;
	specializationInfo.pMapEntries = &specializationMapEntry;
	specializationInfo.dataSize = sizeof(uint32_t);
	specializationInfo.pData = &enablePCF;
	shaderStages[1].pSpecializationInfo = &specializationInfo;

	//Create pipeline and error check
	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &GBufferGraphicsPipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}
	vkDestroyShaderModule(device, fragShaderModule, nullptr);
	vkDestroyShaderModule(device, vertShaderModule, nullptr);
	pipelineInfo.renderPass = subsurfaceManager.SSRenderPass;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;

	auto vertShaderCodeR = readFile("shaders/vertR.spv");
	auto fragShaderCodeR = readFile("shaders/fragR.spv");
	//Set up shader modules for both vertex and fragment shaders
	vertShaderModule = createShaderModule(vertShaderCodeR);
	fragShaderModule = createShaderModule(fragShaderCodeR);

	//Set up vertex info
	vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; //Vertex stage
	vertShaderStageInfo.module = vertShaderModule; //Vertex shader
	vertShaderStageInfo.pName = "main"; //Main function as entry point

	//Set up fragment info
	fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT; //Fragment stage
	fragShaderStageInfo.module = fragShaderModule; //Fragment shader
	fragShaderStageInfo.pName = "main"; //Main function as entry point

	shaderStages[0] = vertShaderStageInfo;
	shaderStages[1] = fragShaderStageInfo;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	//Create pipeline and error check
	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}
	pipelineInfo.renderPass = renderPass;
	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &subsurfaceManager.SSGraphicsPipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}
	//Destroy shader modules now we have finished with them
	vkDestroyShaderModule(device, fragShaderModule, nullptr);
	vkDestroyShaderModule(device, vertShaderModule, nullptr);







	auto vertShaderCodeOff = readFile("shaders/vertOff.spv");
	auto fragShaderCodeOff = readFile("shaders/fragOff.spv");
	//Set up shader modules for both vertex and fragment shaders
	vertShaderModule = createShaderModule(vertShaderCodeOff);
	fragShaderModule = createShaderModule(fragShaderCodeOff);

	//Set up vertex info
	vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; //Vertex stage
	vertShaderStageInfo.module = vertShaderModule; //Vertex shader
	vertShaderStageInfo.pName = "main"; //Main function as entry point

	//Set up fragment info
	fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT; //Fragment stage
	fragShaderStageInfo.module = fragShaderModule; //Fragment shader
	fragShaderStageInfo.pName = "main"; //Main function as entry point

	shaderStages[0] = vertShaderStageInfo;
	shaderStages[1] = fragShaderStageInfo;

	pipelineInfo.stageCount = 1;
	// No blend attachment states (no color attachments used)
	colorBlending.attachmentCount = 0;
	// Cull front faces
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	// Enable depth bias
	rasterizer.depthBiasEnable = VK_TRUE;
	// Add depth bias to dynamic state, so we can change it at runtime
	dynamicStateEnables.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
	dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pDynamicStates = dynamicStateEnables.data();
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
	dynamicState.flags = 0;

	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;


	pipelineInfo.layout = offscreenPipelineLayout;
	pipelineInfo.renderPass = offscreenPass.renderPass;
	vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &offscreenPipeline);
	vkDestroyShaderModule(device, fragShaderModule, nullptr);
	vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

VkShaderModule VulkanApp::createShaderModule(const std::vector<char>& code) {

	//Set up up shader module info
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size(); //Pass in shader size
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data()); //Pass in shader code

	//Create shader module and error check
	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

void VulkanApp::createRenderPass() {
	

	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; //Colour attachment must match swap chain format
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; //Clear after frame
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; //Store frame-buffer after render so we can use it later
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; //Not using stencil buffer for this
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; //Not using stencil buffer for this
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;  //Ignore previos layout
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; //Use image in the swap buffer

	VkAttachmentDescription colorAttachmentResolve = {};
	colorAttachmentResolve.format = swapChainImageFormat;
	colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentResolveRef = {};
	colorAttachmentResolveRef.attachment = 2;
	colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = findDepthFormat();
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	//Refrence to the attachment for the sub passes
	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	//Sub pass info
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; //Tell the pass that this is a graphics pass, not a compute pass
	subpass.colorAttachmentCount = 1; //Just one attachment for colour rendering
	subpass.pColorAttachments = &colorAttachmentRef; //Pass reference
	subpass.pDepthStencilAttachment = &depthAttachmentRef;
	//subpass.pResolveAttachments = &colorAttachmentResolveRef;

	std::array<VkSubpassDependency, 2> dependencies;

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;


	std::array<VkSubpassDescription, 1> subpasses = { subpass };
	std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment};//, colorAttachmentResolve };
	//Set up the final render pass info passing in the above data
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = static_cast<uint32_t>(subpasses.size());
	renderPassInfo.pSubpasses = subpasses.data();
	renderPassInfo.dependencyCount = dependencies.size();
	renderPassInfo.pDependencies = dependencies.data();

	//Create render pass and error check
	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}

void VulkanApp::createFramebuffers() {

	//Allocate memory
	swapChainFramebuffers.resize(swapChainImageViews.size());

	//For each swap chain view create a frame buffer
	for (size_t i = 0; i < swapChainImageViews.size(); i++) {
		std::array<VkImageView, 2> attachments = {
				swapChainImageViews[i],
				depthImageView,
				
		};

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass; //Pass in the render pass
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());; //Two attachments in the swap chain (include depth)
		framebufferInfo.pAttachments = attachments.data(); //pass in the swap chain image view as an attachment along with depth/stencil
		framebufferInfo.width = swapChainExtent.width; //Resolution
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1; //Just 1 layer

		//Create frame buffer and error check
		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void VulkanApp::createCommandPool() {

	//Get the queue family so we can referance the graphics family
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value(); //Pass in the graphics family value

	//Create command pool and error check
	if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}
}

void VulkanApp::createCommandBuffers() {
	
	//Allocate memory
	commandBuffers.resize(swapChainFramebuffers.size());

	//Set up command buffer info
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool; //Pass in the command pool to be part of
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)commandBuffers.size(); //Number of command buffers

	//Allocate command buffers
	if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}

	//For each command buffer set up info and bind the required data
	for (size_t i = 0; i < commandBuffers.size(); i++) {
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}


		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0].color = { 0.2f, 0.2f, 0.2f, 1.0f };//Set clear colour
		clearValues[1].depthStencil = { 1.0f, 0 };  

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass; //Pass renderpass
		renderPassInfo.framebuffer = swapChainFramebuffers[i]; //Pass frame buffer
		renderPassInfo.renderArea.offset = { 0, 0 }; //No offset
		renderPassInfo.renderArea.extent = swapChainExtent; //Set resolution
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size()); //Clear value to 1
		renderPassInfo.pClearValues = clearValues.data(); //Pass in clear colour




		//Begin render pass so we can bind
		


		

		//Set up and bind in vertex infomation
		std::vector<VkBuffer> vertexBuffers;
		for (unsigned int v = 0; v < m_Objects.size(); v++)
		{
			vertexBuffers.push_back(m_Objects[v]->GetVertexBuffer());
		}

		//VkBuffer vertexBuffers[] = { m_Objects[j]->GetVertexBuffer() };
		VkDeviceSize offsets[] = { 0 };
		
		clearValues[0].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = offscreenPass.renderPass;
		renderPassBeginInfo.framebuffer = offscreenPass.frameBuffer;
		renderPassBeginInfo.renderArea.extent.width = offscreenPass.width;
		renderPassBeginInfo.renderArea.extent.height = offscreenPass.height;
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		for (unsigned int j = 0; j < m_Objects.size(); j++)
		{
			//Get depth texture
			if(j < 2 || j > 2)
			{
				
				VkViewport viewportoff = {};
				viewportoff.x = 0.0f; //No offset
				viewportoff.y = 0.0f;//No offset
				//Resolution
				viewportoff.width = 2048;
				viewportoff.height = 2048;
				//Depth buffer range
				viewportoff.minDepth = 0.0f;
				viewportoff.maxDepth = 1.0f;

				vkCmdSetViewport(commandBuffers[i], 0, 1, &viewportoff);

				VkRect2D scissor{};
				scissor.extent.width = 2048;
				scissor.extent.height = 2048;
				scissor.offset.x = 0;
				scissor.offset.y = 0;
				vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

				vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, offscreenPipeline);
				vkCmdSetDepthBias(
					commandBuffers[i],
					1.25f,
					0.0f,
					1.75f);


				vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, offscreenPipelineLayout, 0, 1, &offscreenDescSets[j], 0, NULL);

				vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &m_Objects[j]->GetVertexBuffer(), offsets);
				vkCmdBindIndexBuffer(commandBuffers[i], m_Objects[j]->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
				vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(m_Objects[j]->GetIndices().size()), 1, 0, 0, 0);

				
			}
		}
		vkCmdEndRenderPass(commandBuffers[i]);
		std::array<VkClearValue, 4> clearValuesG;
		clearValuesG[0].color = clearValuesG[1].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
		clearValuesG[2].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
		clearValuesG[3].depthStencil = { 1.0f, 0 };
		renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = offScreenFrameBuf.renderPass;
		renderPassBeginInfo.framebuffer = offScreenFrameBuf.frameBuffer;
		renderPassBeginInfo.renderArea.extent = swapChainExtent;
		renderPassBeginInfo.clearValueCount = 4;
		renderPassBeginInfo.pClearValues = clearValuesG.data();

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		for (unsigned int j = 0; j < m_Objects.size(); j++)
		{
			//Draw Scene
			{
				
				vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &m_Objects[j]->GetVertexBuffer(), offsets);
				unsigned int index = m_Objects.size() * i + j;
				std::cout << index << std::endl;

				//Set up dynamic viewport
				vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);

				VkRect2D scissor{};
				scissor.extent.width = swapChainExtent.width;
				scissor.extent.height = swapChainExtent.height;
				scissor.offset.x = 0;
				scissor.offset.y = 0;
				vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

				//Bind index buffer
				vkCmdBindIndexBuffer(commandBuffers[i], m_Objects[j]->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);


				//Bind the graphics pipeline
				vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, GBufferGraphicsPipeline);

				////Set the descipter to graphics
				vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[index], 0, nullptr);

				////Call the draw command
				vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(m_Objects[j]->GetIndices().size()), 1, 0, 0, 0);
				//vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
				
			}

		}
		vkCmdEndRenderPass(commandBuffers[i]);

		std::array<VkClearValue, 1> clearValuesD;
		clearValuesD[0].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
		//clearValuesD[1].depthStencil = { 1.0f, 0 };
		renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = subsurfaceManager.SSRenderPass;
		renderPassBeginInfo.framebuffer = subsurfaceManager.SSFrameBuffer;
		renderPassBeginInfo.renderArea.extent = swapChainExtent;
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = clearValuesD.data();

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		{
			vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &m_Objects[0]->GetVertexBuffer(), offsets);

			//Set up dynamic viewport
			vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);

			VkRect2D scissor{};
			scissor.extent.width = swapChainExtent.width;
			scissor.extent.height = swapChainExtent.height;
			scissor.offset.x = 0;
			scissor.offset.y = 0;
			vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

			//Bind index buffer
			vkCmdBindIndexBuffer(commandBuffers[i], m_Objects[0]->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);


			//Bind the graphics pipeline
			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

			////Set the descipter to graphics
			vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &finalRSet, 0, nullptr);

			////Call the draw command
			vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(m_Objects[0]->GetIndices().size()), 1, 0, 0, 0);

		}
		vkCmdEndRenderPass(commandBuffers[i]);

		std::array<VkClearValue, 2> clearValuesS;
		clearValuesS[0].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
		clearValuesS[1].depthStencil = { 1.0f, 0 };
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.framebuffer = swapChainFramebuffers[i];
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues = clearValuesS.data();
		vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		{
			vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &m_Objects[0]->GetVertexBuffer(), offsets);

			//Set up dynamic viewport
			vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);

			VkRect2D scissor{};
			scissor.extent.width = swapChainExtent.width;
			scissor.extent.height = swapChainExtent.height;
			scissor.offset.x = 0;
			scissor.offset.y = 0;
			vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

			//Bind index buffer
			vkCmdBindIndexBuffer(commandBuffers[i], m_Objects[0]->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);


			//Bind the graphics pipeline
			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, subsurfaceManager.SSGraphicsPipeline);

			////Set the descipter to graphics
			vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &subsurfaceManager.finalSSet, 0, nullptr);

			////Call the draw command
			vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(m_Objects[0]->GetIndices().size()), 1, 0, 0, 0);

		}
		vkCmdEndRenderPass(commandBuffers[i]);


		//End pass
		//vkCmdEndRenderPass(commandBuffers[i]);
		
		//Check the command has ended and error check
		if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}
}

void VulkanApp::createSyncObjects()
{

	//Allocate memory for all sync objects
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	//Set up semaphore info
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;


	//Set up fence info
	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;


	//For each frame in flight (buffering) clear availabe and finsihed semiforces as well as wait fences
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
			//Throw error if any creation fails
			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}
}

void VulkanApp::recreateSwapChain() {
	
	//Get the new width and height if either are set to 0
	int width = 0, height = 0;
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(window->Window(), &width, &height);
		window->setSize(glm::vec2(width, height));
		glfwWaitEvents();
	}
	//Wait for end of frame
	vkDeviceWaitIdle(device);

	//Delete current swapchain data
	cleanupSwapChain();
	CleanGBuffer();


	//Create all parts of the swap chain
	createSwapChain();
	
	createImageViews();
	createRenderPass();
	createColorResources();
	CreateSSFrameBuffer();
	prepareGOffscreenFramebuffer();
	createGraphicsPipeline();
	
	createDepthResources();
	createFramebuffers();
	
	UpdateGBufferSets();

	createCommandBuffers();
}

void VulkanApp::cleanupSwapChain() {

	

	vkDestroyImageView(device, depthImageView, nullptr);
	vkDestroyImage(device, depthImage, nullptr);
	vkFreeMemory(device, depthImageMemory, nullptr);

	//Destroy all frame buffers
	for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
		vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
	}
	//free memory from command buffers
	vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

	//Destroy graphics pipline and layout
	vkDestroyPipeline(device, GBufferGraphicsPipeline, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

	vkDestroyPipeline(device, offscreenPipeline, nullptr);
	vkDestroyPipelineLayout(device, offscreenPipelineLayout, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr); //Clean up render pass data

	//Clean up SSS
	subsurfaceManager.CleanUpBuffer(device);
	//Destroy all image views
	for (size_t i = 0; i < swapChainImageViews.size(); i++) {
		vkDestroyImageView(device, swapChainImageViews[i], nullptr);
	}
	
	//Destroy the actual swapchain object
	vkDestroySwapchainKHR(device, swapChain, nullptr);
}




void VulkanApp::createDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.pImmutableSamplers = nullptr;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding depthSamplerLayoutBinding = {};
	depthSamplerLayoutBinding.binding = 2;
	depthSamplerLayoutBinding.descriptorCount = 1;
	depthSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	depthSamplerLayoutBinding.pImmutableSamplers = nullptr;
	depthSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding normalSamplerLayoutBinding = {};
	normalSamplerLayoutBinding.binding = 3;
	normalSamplerLayoutBinding.descriptorCount = 1;
	normalSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	normalSamplerLayoutBinding.pImmutableSamplers = nullptr;
	normalSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding specSamplerLayoutBinding = {};
	specSamplerLayoutBinding.binding = 4;
	specSamplerLayoutBinding.descriptorCount = 1;
	specSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	specSamplerLayoutBinding.pImmutableSamplers = nullptr;
	specSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	

	std::array<VkDescriptorSetLayoutBinding, 5> bindings = { uboLayoutBinding, samplerLayoutBinding, depthSamplerLayoutBinding, normalSamplerLayoutBinding, specSamplerLayoutBinding };// guboLayoutBinding

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}

}

void VulkanApp::createUniformBuffers()
{
	//Get size of buffer
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	unsigned int size = 0;
	for (unsigned int i = 0; i < m_Objects.size(); i++)
	{
		size += swapChainImages.size();
	}

	//Allocate memory
	uniformBuffers.resize(size);
	uniformBuffersMemory.resize(size);

	//Create a uniform buffer for each of the swap chain images
	for (size_t i = 0; i < size; i++) {
		m_Engine->createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
	}
	offscreenUniforms.resize(m_Objects.size());
	offscreenMemorys.resize(m_Objects.size());
	for (size_t i = 0; i < m_Objects.size(); i++) {
		m_Engine->createBuffer(sizeof(OffScreenUniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, offscreenUniforms[i], offscreenMemorys[i]);
	}

	m_Engine->createBuffer(sizeof(GBufferUniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, GBUniform, GBUniformMemory);
	m_Engine->createBuffer(sizeof(GBufferUniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, subsurfaceManager.SSUniform, subsurfaceManager.SSUniformMemory);
}

void VulkanApp::updateUniformBuffer(uint32_t currentImage, unsigned int objectIndex)
{
	unsigned int index = m_Objects.size() * currentImage + objectIndex;

	
	//Time at start of frame
	static auto startTime = std::chrono::high_resolution_clock::now();

	//Current time
	auto currentTime = std::chrono::high_resolution_clock::now();
	//Delta Time
	
	
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
	//std::cout << time - realTime << '\r' << std::endl;// std::flush;
	timercount += time - realTime;
	realTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	

	if (timercount >= 1)
	{
		timercount = 0;
		std::cout << framecount << '\r' << std::endl;
		framecount = 0;
	}

	glm::vec3 lightPos = glm::vec3(-0.0f, 0.1f, -0.75f) *glm::mat3(glm::rotate(time * glm::radians(45.0f), glm::vec3(0, 1, 0)));
	if (objectIndex == 2)
	{
		glm::vec3 newPos = lightPos;
		glm::vec3 vDir = glm::normalize(-newPos);
		newPos += vDir * glm::vec3(0.55f);
		m_Objects[objectIndex]->SetPos(newPos);
	}

	// Matrix from light's point of view
	glm::mat4 depthProjectionMatrix = glm::perspective(glm::radians(45.0f), 1.0f / 1.0f, 0.01f, 150.0f);
	depthProjectionMatrix[1][1] *= -1;
	glm::mat4 depthViewMatrix = glm::lookAt(lightPos, glm::vec3(0.0f, 0.015f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 depthModelMatrix = glm::mat4(1); 
	depthModelMatrix = glm::translate(glm::mat4(1.0f), m_Objects[objectIndex]->GetPos()) * glm::rotate(depthModelMatrix, time * glm::radians(m_Objects[objectIndex]->GetRot().y), glm::vec3(0, 1, 0)) * glm::scale(glm::mat4(1.0f), m_Objects[objectIndex]->GetScale());

	//Set up the uniform model matrix (rotation and translation and scale)
	UniformBufferObject ubo = {};
	//glm::mat4 model = glm::translate(glm::mat4(1.0f), m_Objects[objectIndex]->GetPos()) * glm::rotate(ubo.model, time * glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 10.0f, 10.0f));
	ubo.model = glm::mat4(1);
	ubo.model = glm::translate(glm::mat4(1.0f), m_Objects[objectIndex]->GetPos()) * glm::rotate(ubo.model, time * glm::radians(m_Objects[objectIndex]->GetRot().y), glm::vec3(0, 1, 0)) * glm::scale(glm::mat4(1.0f), m_Objects[objectIndex]->GetScale());
	
	
	//View matrix using look at
	ubo.view = glm::lookAt(glm::vec3(0.0f, 0.1f, 0.55f), glm::vec3(0.0f, 0.015f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	//Projection / Perspective matrix
	glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)swapChainExtent.width / (float)swapChainExtent.height, 0.01f, 100.0f);
	proj[1][1] *= -1;
	ubo.proj = proj;
	ubo.lightRot = glm::rotate(glm::mat4(1), time * glm::radians(45.0f), glm::vec3(0, 1, 0));
	ubo.lightSpace = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;
	ubo.lightViewProj = depthProjectionMatrix * depthViewMatrix;

	ubo.AmbientColour = Lighting::AmbientColour;
	ubo.AmbientColour.w = m_Objects[objectIndex]->Lit();
	ubo.DirectionalColour = Lighting::LightColour;

	//Map memory to a CPU side pointer, copy over data then unmap from cpu side
	void* data;
	vkMapMemory(device, uniformBuffersMemory[index], 0, sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(device, uniformBuffersMemory[index]);

	
	//Depth MVP
	offscreenUBOs[objectIndex].depthMVP = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;

	void* offdata;
	vkMapMemory(device, offscreenMemorys[objectIndex], 0, sizeof(OffScreenUniformBufferObject), 0, &offdata);
	memcpy(offdata, &offscreenUBOs[objectIndex], sizeof(OffScreenUniformBufferObject));
	vkUnmapMemory(device, offscreenMemorys[objectIndex]);
	
	//SSSS first pass
	GBubo = {};
	float width = swapChainExtent.width;
	float height = swapChainExtent.height;
	glm::mat4 finalM = (glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0)) * glm::rotate(glm::radians(0.0f), glm::vec3(0, 1, 0)) * glm::scale(glm::mat4(1.0f), glm::vec3(width/2, height/2, 1)));
	glm::mat4 finalView = glm::lookAt(glm::vec3(0.0f, 0.001f, 0.55f), glm::vec3(0.0f, 0.001f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	
	GBubo.proj = glm::ortho<float>(-width /2, width / 2, height / 2, -height / 2, -1.f, 1.f);
	GBubo.view = finalView;
	GBubo.model = finalM;
	for (size_t i = 0; i < SAMPLES; i++) GBubo.kernel[i] = subsurfaceManager.kernel[i];
	GBubo.blurDirection = glm::vec2(1, 0); //Blur horizontal

	void* gbdata;
	vkMapMemory(device, GBUniformMemory, 0, sizeof(GBufferUniformBufferObject), 0, &gbdata);
	memcpy(gbdata, &GBubo, sizeof(GBufferUniformBufferObject));
	vkUnmapMemory(device, GBUniformMemory);

	//SSSS Second Pass
	SSubo = {};
	SSubo.proj = glm::ortho<float>(-width / 2, width / 2, height / 2, -height / 2, -1.f, 1.f);
	SSubo.view = finalView;
	SSubo.model = finalM;
	for (size_t i = 0; i < SAMPLES; i++) SSubo.kernel[i] = subsurfaceManager.kernel[i];
	SSubo.blurDirection = glm::vec2(0, 1);//Blur Verticle

	void* ssdata;
	vkMapMemory(device, subsurfaceManager.SSUniformMemory, 0, sizeof(GBufferUniformBufferObject), 0, &ssdata);
	memcpy(ssdata, &SSubo, sizeof(GBufferUniformBufferObject));
	vkUnmapMemory(device, subsurfaceManager.SSUniformMemory);
}

void VulkanApp::createDescriptorPool()
{
	unsigned int size = 0;
	for (unsigned int i = 0; i < m_Objects.size(); i++)
	{
		size += swapChainImages.size();
	}

	std::array<VkDescriptorPoolSize, 2> poolSizes = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(size*5);
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(size*7); //Need additional textures for the shadow map


	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(size*2);

	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void VulkanApp::createDescriptorSets()
{
	unsigned int size = 0;
	for (unsigned int i = 0; i < m_Objects.size(); i++)
	{
		size += swapChainImages.size();
	}

	//Allocate memory
	std::vector<VkDescriptorSetLayout> layouts(size, descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool; //Pass in the pool
	allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size()); //Same as pool descriptor count
	allocInfo.pSetLayouts = layouts.data(); //Pass in layout data

	
	
	
	//Allocate memory
	descriptorSets.resize(size);
	//Allocate desciptor sets 
	if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}
	//For each swap chain image set up and pass in the descriptor set and buffer for each image
	for (size_t i = 0; i < swapChainImages.size(); i++) {
		for (unsigned int j = 0; j < m_Objects.size(); j++)
		{
			unsigned int index = m_Objects.size() * i + j;
				
			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = uniformBuffers[index]; //Actual buffer to use
			bufferInfo.offset = 0; //Start at the start
			bufferInfo.range = sizeof(UniformBufferObject); //Size of each buffer

			VkDescriptorImageInfo imageInfo = {};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		
			//std::cout << index << std::endl;
			imageInfo.imageView = m_Objects[j]->GetTextureImageView();
			imageInfo.sampler = m_Objects[j]->GetTextureSampler();

			//Pass uniform buffer at binding 0
			std::array<VkWriteDescriptorSet, 5> descriptorWrites = {};
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = descriptorSets[index]; //desciptor to use
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;
				
			//Pass uniform sampler at binding 1
			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = descriptorSets[index];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pImageInfo = &imageInfo;

			VkDescriptorImageInfo imageInfoDepth = {};
			imageInfoDepth.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			imageInfoDepth.imageView = offscreenPass.depth.view;// m_Objects[j]->GetTextureImageView();
			imageInfoDepth.sampler = offscreenPass.depthSampler;//m_Objects[j]->GetTextureSampler();
			descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[2].dstSet = descriptorSets[index];
			descriptorWrites[2].dstBinding = 2;
			descriptorWrites[2].dstArrayElement = 0;
			descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[2].descriptorCount = 1;
			descriptorWrites[2].pImageInfo = &imageInfoDepth;

			VkDescriptorImageInfo imageInfoNormal = {};
			imageInfoNormal.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfoNormal.imageView = m_Objects[j]->GetNormalTextureImageView();
			imageInfoNormal.sampler = m_Objects[j]->GetNormalTextureSampler();
			descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[3].dstSet = descriptorSets[index];
			descriptorWrites[3].dstBinding = 3;
			descriptorWrites[3].dstArrayElement = 0;
			descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[3].descriptorCount = 1;
			descriptorWrites[3].pImageInfo = &imageInfoNormal;

			VkDescriptorImageInfo imageInfoSpec = {};
			imageInfoSpec.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfoSpec.imageView = m_Objects[j]->GetSpecTextureImageView();
			imageInfoSpec.sampler = m_Objects[j]->GetSpecTextureSampler();
			descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[4].dstSet = descriptorSets[index];
			descriptorWrites[4].dstBinding = 4;
			descriptorWrites[4].dstArrayElement = 0;
			descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[4].descriptorCount = 1;
			descriptorWrites[4].pImageInfo = &imageInfoSpec;



			//Set the descriptor set for this image
			vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
			
		}
	}


	//Allocate memory
	std::vector<VkDescriptorSetLayout> layoutsOff(m_Objects.size(), descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfoOff = {};
	allocInfoOff.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfoOff.descriptorPool = descriptorPool; //Pass in the pool
	allocInfoOff.descriptorSetCount = static_cast<uint32_t>(layoutsOff.size()); //Same as pool descriptor count
	allocInfoOff.pSetLayouts = layoutsOff.data(); //Pass in layout data
	offscreenDescSets.resize(m_Objects.size());
	if (vkAllocateDescriptorSets(device, &allocInfoOff, offscreenDescSets.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate offscreen descriptor sets!");
	}
	offscreenUBOs.resize(m_Objects.size());
	for (unsigned int i = 0; i < m_Objects.size(); i++)
	{
		VkDescriptorBufferInfo bufferInfoOff = {};
		bufferInfoOff.buffer = offscreenUniforms[i]; //Actual buffer to use
		bufferInfoOff.offset = 0; //Start at the start
		bufferInfoOff.range = sizeof(OffScreenUniformBufferObject);
		std::array<VkWriteDescriptorSet, 1> descriptorWritesOff = {};
		descriptorWritesOff[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWritesOff[0].dstSet = offscreenDescSets[i]; //desciptor to use
		descriptorWritesOff[0].dstBinding = 0;
		descriptorWritesOff[0].dstArrayElement = 0;
		descriptorWritesOff[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWritesOff[0].descriptorCount = 1;
		descriptorWritesOff[0].pBufferInfo = &bufferInfoOff;
		vkUpdateDescriptorSets(device, descriptorWritesOff.size(), descriptorWritesOff.data(), 0, nullptr);
	}

	//Allocate memory
	std::vector<VkDescriptorSetLayout> layoutsR(1, descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfoR = {};
	allocInfoR.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfoR.descriptorPool = descriptorPool; //Pass in the pool
	allocInfoR.descriptorSetCount = static_cast<uint32_t>(layoutsR.size()); //Same as pool descriptor count
	allocInfoR.pSetLayouts = layoutsR.data(); //Pass in layout data
	if (vkAllocateDescriptorSets(device, &allocInfoR, &finalRSet) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate offscreen descriptor sets!");
	}
	if (vkAllocateDescriptorSets(device, &allocInfoR, &subsurfaceManager.finalSSet) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate offscreen descriptor sets!");
	}

	UpdateGBufferSets();
}

void VulkanApp::createDepthResources()
{
	VkFormat depthFormat = findDepthFormat();

	m_Engine->createImage(swapChainExtent.width, swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory, VK_SAMPLE_COUNT_1_BIT);
	depthImageView = m_Engine->createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

	m_Engine->transitionImageLayout(graphicsQueue, commandPool, depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

VkFormat VulkanApp::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}

	throw std::runtime_error("failed to find supported format!");
}

VkFormat VulkanApp::findDepthFormat()
{
	return findSupportedFormat(
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

void VulkanApp::prepareOffscreenRenderpass()
{

	VkAttachmentDescription attDesc{};
	attDesc.format = VK_FORMAT_D16_UNORM;
	attDesc.samples = VK_SAMPLE_COUNT_1_BIT;
	attDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; //Clear depth at beginning
	attDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;	//Store the depth attachment results
	attDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;//No initial layout of the attachment
	attDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

	VkAttachmentReference depthReference = {};
	depthReference.attachment = 0;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; //DepthStencil attachment during render pass

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 0;//No color attachments. instead we will just sample the R channel automatically
	subpass.pDepthStencilAttachment = &depthReference;//Depth attachment

	//Subpass dependencies
	std::array<VkSubpassDependency, 2> dependencies;

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = 1;
	renderPassCreateInfo.pAttachments = &attDesc;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpass;
	renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassCreateInfo.pDependencies = dependencies.data();

	vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &offscreenPass.renderPass);
}

void VulkanApp::prepareOffscreenFramebuffer()
{
	offscreenPass.width = 2048;
	offscreenPass.height = 2048;

	VkFormat fbColorFormat = VK_FORMAT_R8G8B8A8_UNORM;

	// For shadow mapping we only need a depth attachment
	VkImageCreateInfo image{};
	image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image.imageType = VK_IMAGE_TYPE_2D;
	image.extent.width = offscreenPass.width;
	image.extent.height = offscreenPass.height;
	image.extent.depth = 1;
	image.mipLevels = 1;
	image.arrayLayers = 1;
	image.samples = VK_SAMPLE_COUNT_1_BIT;
	image.tiling = VK_IMAGE_TILING_OPTIMAL;
	image.format = VK_FORMAT_D16_UNORM;	//DepthStencil attachment
	image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT; //Sample directly from the depth attachment for the shadow mapping
	vkCreateImage(device, &image, nullptr, &offscreenPass.depth.image);

	VkMemoryAllocateInfo mem{};
	mem.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	VkMemoryRequirements memReqs;
	vkGetImageMemoryRequirements(device, offscreenPass.depth.image, &memReqs);
	mem.allocationSize = memReqs.size;
	mem.memoryTypeIndex = m_Engine->findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	vkAllocateMemory(device, &mem, nullptr, &offscreenPass.depth.mem);
	vkBindImageMemory(device, offscreenPass.depth.image, offscreenPass.depth.mem, 0);

	VkImageViewCreateInfo dsView{};
	dsView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	dsView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	dsView.format = VK_FORMAT_D16_UNORM;
	dsView.subresourceRange = {};
	dsView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	dsView.subresourceRange.baseMipLevel = 0;
	dsView.subresourceRange.levelCount = 1;
	dsView.subresourceRange.baseArrayLayer = 0;
	dsView.subresourceRange.layerCount = 1;
	dsView.image = offscreenPass.depth.image;
	vkCreateImageView(device, &dsView, nullptr, &offscreenPass.depth.view);

	//Sampler to sample from to depth attachment 
	//Sample in the fragment shader for shadowed rendering
	VkSamplerCreateInfo depthSampler{};
	depthSampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	depthSampler.maxAnisotropy = 1.0f;
	depthSampler.magFilter = VK_FILTER_LINEAR;
	depthSampler.minFilter = VK_FILTER_LINEAR;
	depthSampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	depthSampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	depthSampler.addressModeV = depthSampler.addressModeU;
	depthSampler.addressModeW = depthSampler.addressModeU;
	depthSampler.mipLodBias = 0.0f;
	depthSampler.maxAnisotropy = 1.0f;
	depthSampler.minLod = 0.0f;
	depthSampler.maxLod = 1.0f;
	depthSampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	vkCreateSampler(device, &depthSampler, nullptr, &offscreenPass.depthSampler);

	prepareOffscreenRenderpass();

	//Create frame buffer to store the depth infomation
	VkFramebufferCreateInfo fBuf{};
	fBuf.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fBuf.renderPass = offscreenPass.renderPass;
	fBuf.attachmentCount = 1;
	fBuf.pAttachments = &offscreenPass.depth.view;
	fBuf.width = offscreenPass.width;
	fBuf.height = offscreenPass.height;
	fBuf.layers = 1;

	vkCreateFramebuffer(device, &fBuf, nullptr, &offscreenPass.frameBuffer);
}

VkSampleCountFlagBits VulkanApp::getMaxUsableSampleCount()
{
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

	VkSampleCountFlags counts = std::min(physicalDeviceProperties.limits.framebufferColorSampleCounts, physicalDeviceProperties.limits.framebufferDepthSampleCounts);
	if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
	if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
	if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
	if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
	if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
	if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

	return VK_SAMPLE_COUNT_1_BIT;
}

void VulkanApp::createColorResources()
{
	VkFormat colorFormat = swapChainImageFormat;

	m_Engine->createImage(swapChainExtent.width, swapChainExtent.height, colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage, colorImageMemory, VK_SAMPLE_COUNT_1_BIT);
	colorImageView = m_Engine->createImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT);

	//m_Engine->transitionImageLayout(graphicsQueue, commandPool, colorImage, colorFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	////////////////
	m_Engine->createImage(swapChainExtent.width, swapChainExtent.height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, normalImage, normalImageMemory, VK_SAMPLE_COUNT_1_BIT);
	normalImageView = m_Engine->createImageView(normalImage, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);

	//m_Engine->transitionImageLayout(graphicsQueue, commandPool, normalImage, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	///////////
	m_Engine->createImage(swapChainExtent.width, swapChainExtent.height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, posImage, posImageMemory, VK_SAMPLE_COUNT_1_BIT);
	posImageView = m_Engine->createImageView(posImage, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);

	//m_Engine->transitionImageLayout(graphicsQueue, commandPool, posImage, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	//////////////
	VkFormat depthFormat = findDepthFormat();

	m_Engine->createImage(swapChainExtent.width, swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, dImage, dImageMemory, VK_SAMPLE_COUNT_1_BIT);
	dImageView = m_Engine->createImageView(dImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

	//m_Engine->transitionImageLayout(graphicsQueue, commandPool, dImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	
}

void VulkanApp::CreateGAttachment(VkFormat format, VkImageUsageFlagBits usage, FrameBufferAttachment * attachment)
{
	VkImageAspectFlags aspectMask = 0;
	VkImageLayout imageLayout;

	attachment->format = format;

	if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
	{
		aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}
	if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
		aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;// | VK_IMAGE_ASPECT_STENCIL_BIT;
		imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}

	assert(aspectMask > 0);

	VkImageCreateInfo image{};
	image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image.imageType = VK_IMAGE_TYPE_2D;
	image.format = format;
	image.extent.width = swapChainExtent.width;
	image.extent.height = swapChainExtent.height;
	image.extent.depth = 1;
	image.mipLevels = 1;
	image.arrayLayers = 1;
	image.samples = msaaSamples;
	image.tiling = VK_IMAGE_TILING_OPTIMAL;
	image.usage = usage | VK_IMAGE_USAGE_SAMPLED_BIT;

	VkMemoryAllocateInfo memAlloc{};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	VkMemoryRequirements memReqs;

	vkCreateImage(device, &image, nullptr, &attachment->image);
	vkGetImageMemoryRequirements(device, attachment->image, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = m_Engine->findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	vkAllocateMemory(device, &memAlloc, nullptr, &attachment->mem);
	vkBindImageMemory(device, attachment->image, attachment->mem, 0);

	VkImageViewCreateInfo imageView{};
	imageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageView.format = format;
	imageView.subresourceRange = {};
	imageView.subresourceRange.aspectMask = aspectMask;
	imageView.subresourceRange.baseMipLevel = 0;
	imageView.subresourceRange.levelCount = 1;
	imageView.subresourceRange.baseArrayLayer = 0;
	imageView.subresourceRange.layerCount = 1;
	imageView.image = attachment->image;
	vkCreateImageView(device, &imageView, nullptr, &attachment->view);
}

void VulkanApp::prepareGOffscreenFramebuffer()
{
	// Color attachments

	// (World space) Positions
	CreateGAttachment(
		VK_FORMAT_R16G16B16A16_SFLOAT,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		&offScreenFrameBuf.position);

	// (World space) Normals
	CreateGAttachment(
		VK_FORMAT_R16G16B16A16_SFLOAT,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		&offScreenFrameBuf.normal);

	// Albedo (color)
	CreateGAttachment(
		VK_FORMAT_B8G8R8A8_UNORM,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		&offScreenFrameBuf.albedo);

	// Depth attachment

	// Find a suitable depth format
	VkFormat attDepthFormat = findDepthFormat();

	CreateGAttachment(
		attDepthFormat,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		&offScreenFrameBuf.depth);

	// Set up separate renderpass with references
	// to the color and depth attachments

	std::array<VkAttachmentDescription, 8> attachmentDescs = {};

	// Init attachment properties
	for (uint32_t i = 0; i < 4; ++i)
	{
		attachmentDescs[i].samples = msaaSamples;
		attachmentDescs[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDescs[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescs[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescs[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		if (i == 3)
		{
			attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
		else
		{
			attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}
	}

	// Formats
	attachmentDescs[0].format = offScreenFrameBuf.position.format;
	attachmentDescs[1].format = offScreenFrameBuf.normal.format;
	attachmentDescs[2].format = offScreenFrameBuf.albedo.format;
	attachmentDescs[3].format = offScreenFrameBuf.depth.format;

	std::vector<VkAttachmentReference> colorReferences;
	colorReferences.push_back({ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
	colorReferences.push_back({ 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
	colorReferences.push_back({ 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

	VkAttachmentReference depthReference = {};
	depthReference.attachment = 3;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


	VkAttachmentDescription colorAttachmentResolve = {};
	colorAttachmentResolve.format = offScreenFrameBuf.position.format;
	colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	VkAttachmentDescription albedoAttachmentResolve = {};
	albedoAttachmentResolve.format = offScreenFrameBuf.albedo.format;
	albedoAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
	albedoAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	albedoAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	albedoAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	albedoAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	albedoAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	albedoAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	VkAttachmentDescription depthAttachmentResolve = {};
	depthAttachmentResolve.format = offScreenFrameBuf.depth.format;
	depthAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentResolveRef = {};
	colorAttachmentResolveRef.attachment = 4;
	colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	VkAttachmentReference colorAttachmentResolveRef2 = {};
	colorAttachmentResolveRef2.attachment = 5;
	colorAttachmentResolveRef2.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	VkAttachmentReference colorAttachmentResolveRef3 = {};
	colorAttachmentResolveRef3.attachment = 6;
	colorAttachmentResolveRef3.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	VkAttachmentReference colorAttachmentResolveRef4 = {};
	colorAttachmentResolveRef4.attachment = 7;
	colorAttachmentResolveRef4.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	attachmentDescs[4] = colorAttachmentResolve;
	attachmentDescs[5] = colorAttachmentResolve;
	attachmentDescs[6] = albedoAttachmentResolve;
	attachmentDescs[7] = depthAttachmentResolve;

	std::array<VkAttachmentReference, 4> refs = { colorAttachmentResolveRef,colorAttachmentResolveRef2,colorAttachmentResolveRef3,colorAttachmentResolveRef4 };
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.pColorAttachments = colorReferences.data();
	subpass.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
	subpass.pDepthStencilAttachment = &depthReference;
	subpass.pResolveAttachments = refs.data();

	// Use subpass dependencies for attachment layput transitions
	std::array<VkSubpassDependency, 2> dependencies;

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pAttachments = attachmentDescs.data();
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescs.size());
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 2;
	renderPassInfo.pDependencies = dependencies.data();

	vkCreateRenderPass(device, &renderPassInfo, nullptr, &offScreenFrameBuf.renderPass);

	std::array<VkImageView, 8> attachments;
	attachments[0] = offScreenFrameBuf.position.view;
	attachments[1] = offScreenFrameBuf.normal.view;
	attachments[2] = offScreenFrameBuf.albedo.view;
	attachments[3] = offScreenFrameBuf.depth.view;
	attachments[4] = posImageView;
	attachments[5] = normalImageView;
	attachments[6] = colorImageView;
	attachments[7] = dImageView;

	VkFramebufferCreateInfo fbufCreateInfo = {};
	fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbufCreateInfo.pNext = NULL;
	fbufCreateInfo.renderPass = offScreenFrameBuf.renderPass;
	fbufCreateInfo.pAttachments = attachments.data();
	fbufCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	fbufCreateInfo.width = swapChainExtent.width;
	fbufCreateInfo.height = swapChainExtent.height;
	fbufCreateInfo.layers = 1;
	vkCreateFramebuffer(device, &fbufCreateInfo, nullptr, &offScreenFrameBuf.frameBuffer);

	// Create sampler to sample from the color attachments
	VkSamplerCreateInfo sampler{};
	sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler.maxAnisotropy = 1.0f;
	sampler.magFilter = VK_FILTER_NEAREST;
	sampler.minFilter = VK_FILTER_NEAREST;
	sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler.addressModeV = sampler.addressModeU;
	sampler.addressModeW = sampler.addressModeU;
	sampler.mipLodBias = 0.0f;
	sampler.maxAnisotropy = 1.0f;
	sampler.minLod = 0.0f;
	sampler.minLod = 0.0f;
	sampler.maxLod = 1.0f;
	sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	vkCreateSampler(device, &sampler, nullptr, &colourSampler);

	
}

void VulkanApp::CleanGBuffer()
{
	//Clean up single samplesv
	vkDestroyImageView(device, colorImageView, nullptr);
	vkDestroyImage(device, colorImage, nullptr);
	vkFreeMemory(device, colorImageMemory, nullptr);

	vkDestroyImageView(device, normalImageView, nullptr);
	vkDestroyImage(device, normalImage, nullptr);
	vkFreeMemory(device, normalImageMemory, nullptr);

	vkDestroyImageView(device, posImageView, nullptr);
	vkDestroyImage(device, posImage, nullptr);
	vkFreeMemory(device, posImageMemory, nullptr);

	vkDestroyImageView(device, dImageView, nullptr);
	vkDestroyImage(device, dImage, nullptr);
	vkFreeMemory(device, dImageMemory, nullptr);

	// Color attachments
	vkDestroyImageView(device, offScreenFrameBuf.position.view, nullptr);
	vkDestroyImage(device, offScreenFrameBuf.position.image, nullptr);
	vkFreeMemory(device, offScreenFrameBuf.position.mem, nullptr);

	vkDestroyImageView(device, offScreenFrameBuf.normal.view, nullptr);
	vkDestroyImage(device, offScreenFrameBuf.normal.image, nullptr);
	vkFreeMemory(device, offScreenFrameBuf.normal.mem, nullptr);

	vkDestroyImageView(device, offScreenFrameBuf.albedo.view, nullptr);
	vkDestroyImage(device, offScreenFrameBuf.albedo.image, nullptr);
	vkFreeMemory(device, offScreenFrameBuf.albedo.mem, nullptr);

	// Depth attachment
	vkDestroyImageView(device, offScreenFrameBuf.depth.view, nullptr);
	vkDestroyImage(device, offScreenFrameBuf.depth.image, nullptr);
	vkFreeMemory(device, offScreenFrameBuf.depth.mem, nullptr);

	vkDestroySampler(device, colourSampler, nullptr);

	vkDestroyFramebuffer(device, offScreenFrameBuf.frameBuffer, nullptr);
	vkDestroyRenderPass(device, offScreenFrameBuf.renderPass, nullptr);

	vkDestroyPipeline(device, graphicsPipeline, nullptr);
}

void VulkanApp::UpdateGBufferSets()
{
	VkDescriptorBufferInfo bufferInfo = {};
	bufferInfo.buffer = GBUniform; //Actual buffer to use
	bufferInfo.offset = 0; //Start at the start
	bufferInfo.range = sizeof(GBufferUniformBufferObject); //Size of each buffer

	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	//std::cout << index << std::endl;
	imageInfo.imageView = colorImageView;
	imageInfo.sampler = colourSampler;

	//Pass uniform buffer at binding 0
	std::array<VkWriteDescriptorSet, 3> descriptorWrites = {};
	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].dstSet = finalRSet; //desciptor to use
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pBufferInfo = &bufferInfo;

	//Pass uniform sampler at binding 1
	descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[1].dstSet = finalRSet;
	descriptorWrites[1].dstBinding = 1;
	descriptorWrites[1].dstArrayElement = 0;
	descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[1].descriptorCount = 1;
	descriptorWrites[1].pImageInfo = &imageInfo;


	VkDescriptorImageInfo imageInfoNorm = {};
	imageInfoNorm.imageLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	imageInfoNorm.imageView = normalImageView;
	imageInfoNorm.sampler = colourSampler;
	descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[2].dstSet = finalRSet;
	descriptorWrites[2].dstBinding = 2;
	descriptorWrites[2].dstArrayElement = 0;
	descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[2].descriptorCount = 1;
	descriptorWrites[2].pImageInfo = &imageInfoNorm;


	vkUpdateDescriptorSets(device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);

	descriptorWrites[0].dstSet = subsurfaceManager.finalSSet;
	descriptorWrites[1].dstSet = subsurfaceManager.finalSSet;
	descriptorWrites[2].dstSet = subsurfaceManager.finalSSet;

	imageInfo.imageView = subsurfaceManager.SSImageView;
	descriptorWrites[1].pImageInfo = &imageInfo;
	bufferInfo.buffer = subsurfaceManager.SSUniform;
	descriptorWrites[0].pBufferInfo = &bufferInfo;
	
	vkUpdateDescriptorSets(device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
}

void VulkanApp::CreateSSFrameBuffer()
{
	//SS
	m_Engine->createImage(swapChainExtent.width, swapChainExtent.height, swapChainImageFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, subsurfaceManager.SSImage, subsurfaceManager.SSImageMemory, VK_SAMPLE_COUNT_1_BIT);
	subsurfaceManager.SSImageView = m_Engine->createImageView(subsurfaceManager.SSImage, swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	subsurfaceManager.computeKernel();

	//Render Pass
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; //Colour attachment must match swap chain format
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; //Clear after frame
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; //Store frame-buffer after render so we can use it later
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; //Not using stencil buffer for this
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; //Not using stencil buffer for this
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;  //Ignore previos layout
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; //Use image in the swap buffer

	//Refrence to the attachment for the sub passes
	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


	//Sub pass info
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; //Tell the pass that this is a graphics pass, not a compute pass
	subpass.colorAttachmentCount = 1; //Just one attachment for colour rendering
	subpass.pColorAttachments = &colorAttachmentRef; //Pass reference

	std::array<VkSubpassDependency, 2> dependencies;

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;


	std::array<VkSubpassDescription, 1> subpasses = { subpass };
	std::array<VkAttachmentDescription, 1> passAttachments = { colorAttachment };//, colorAttachmentResolve };
	//Set up the final render pass info passing in the above data
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(passAttachments.size());
	renderPassInfo.pAttachments = passAttachments.data();
	renderPassInfo.subpassCount = static_cast<uint32_t>(subpasses.size());
	renderPassInfo.pSubpasses = subpasses.data();
	renderPassInfo.dependencyCount = dependencies.size();
	renderPassInfo.pDependencies = dependencies.data();

	//Create render pass and error check
	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &subsurfaceManager.SSRenderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
	//For each swap chain view create a frame buffer

	std::array<VkImageView, 1> attachments = {
			subsurfaceManager.SSImageView

	};

	VkFramebufferCreateInfo framebufferInfo = {};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = subsurfaceManager.SSRenderPass; //Pass in the render pass
	framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());; //Two attachments in the swap chain (include depth)
	framebufferInfo.pAttachments = attachments.data(); //pass in the swap chain image view as an attachment along with depth/stencil
	framebufferInfo.width = swapChainExtent.width; //Resolution
	framebufferInfo.height = swapChainExtent.height;
	framebufferInfo.layers = 1; //Just 1 layer

	//Create frame buffer and error check
	if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &subsurfaceManager.SSFrameBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create framebuffer!");
	}
	


}



