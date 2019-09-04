#pragma once

#define GLFW_INCLUDE_VULKAN
#include <glfw3.h>

#include <cstdlib>
#include <stdexcept>

#include <vector>

#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include "VulkanObject.h"
#include <random>

//! VulkanEngine
/*!
Class containing utility functions that are required in multiple other classes
*/
class VulkanEngine
{

private:
	//! Private VkPhysicalDevice&.
	/*! Reference to the physical device*/
	VkPhysicalDevice& m_PhyDevice;
	//! Private VkDevice&.
	/*! Reference to the logical device*/
	VkDevice& m_Device;
public: 
	//! VulkanObject Contructor
	/*!
	Sets up material shader
	\param phyDevice VkPhysicalDevice&, physical device referance
	\param device VkDevice&, logical device referance
	*/
	VulkanEngine(VkPhysicalDevice& phyDevice, VkDevice& device);

	//! Public findMemoryType function
	/*!
	Finds a suitable memory type for storing infomation on the GPU
	*/
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	//! Public beginSingleTimeCommands function
	/*!
	Used to start a single use command buffer, mianly use for copying buffers and trasitioning image layouts
	*/
	VkCommandBuffer beginSingleTimeCommands(VkCommandPool& comPool);
	//! Public endSingleTimeCommands function
	/*!
	Ends the command buffer started using beginSingleTimeCommands
	*/
	void endSingleTimeCommands(VkQueue& graphicsQueue, VkCommandPool& comPool, VkCommandBuffer commandBuffer);
	//! Public createBuffer function
	/*!
	Creates a Vulkan Buffer using the passed in flags and properties
	*/
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	//! Public copyBuffer function
	/*!
	Duplicates a buffer by copying one buffer to another
	*/
	void copyBuffer(VkQueue& graphicsQueue, VkCommandPool& comPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	//! Public createVertexBuffer function
	/*!
	Creates a vertex buffer based on the mesh data stored in a VulkanObject
	*/
	void createVertexBuffer(VkQueue& graphicsQueue, VkCommandPool& comPool, VulkanObject* object);
	//! Public createIndexBuffer function
	/*!
	Creates a index buffer based on the mesh data stored in a VulkanObject
	*/
	void createIndexBuffer(VkQueue& graphicsQueue, VkCommandPool& comPool, VulkanObject* object);

	//Textures

	//! Public createImage function
	/*!
	Create a VkImage object using the passed in properties
	*/
	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, VkSampleCountFlagBits numSamples);
	//! Public createTextureImage function
	/*!
	Create a VkImage object using the texture file at the texturePath
	*/
	void createTextureImage(VkQueue& graphicsQueue, VkCommandPool& comPool, VkImage& textureImage, VkDeviceMemory& textureImageMemory, const char* texturePath);
	//! Public createTextureImage function
	/*!
	Create a VkImage object using radonmly generated noise
	*/
	void createNoiseTextureImage(VkQueue& graphicsQueue, VkCommandPool& comPool, VkImage& textureImage, VkDeviceMemory& textureImageMemory, float distribution);
	//! Public transitionImageLayout function
	/*!
	Changes the layout properties of an image to a new Image layout
	*/
	void transitionImageLayout(VkQueue& graphicsQueue, VkCommandPool& comPool, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	//! Public copyBufferToImage function
	/*!
	Copy infomation from a buffer into an image
	*/
	void copyBufferToImage(VkQueue& graphicsQueue, VkCommandPool& comPool, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	//! Public createTextureImageView function
	/*!
	Create the image view for a texture image
	*/
	void createTextureImageView(VulkanObject* object, VkImageView& view, VkImage& image);
	//! Public createTextureSampler function
	/*!
	Create the texture sampler for a texture image
	*/
	void createTextureSampler(VulkanObject* object, VkSampler& sampler);
	//! Public createImageView function
	/*!
	Create the an image view for a non-texture image
	*/
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

	//! Public createTextureImageView function
	/*!
	Create the image view for a texture image
	*/
	VkImageView createTextureImageView(VkImage& image);
	//! Public createTextureSampler function
	/*!
	Create the texture sampler for a texture image
	*/
	void createTextureSampler(VkSampler& sampler);


	bool hasStencilComponent(VkFormat format);
};