#pragma once




#define GLFW_INCLUDE_VULKAN
#include <glfw3.h>

#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <array>
#include <vector>


#include <unordered_map>




class VulkanEngine;

/*! Vertex Struct
Holds the vertex information, main the position and colour.
*/
struct Vertex {
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 texCoord;

	//Returns the vertex binding desciption
	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0; //Bind to 0
		bindingDescription.stride = sizeof(Vertex); //Size of the vertex struct for each vertex
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; //Input as a vertex

		return bindingDescription;
	}

	//Get the details for each attribute stream
	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};

		//Both are bound to zero as they are both in the same stream
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0; //Location at 0
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT; //Vector3D/RGB value
		attributeDescriptions[0].offset = offsetof(Vertex, pos); //Offset upto the vertex position (current this is no offset)

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1; //Location 1
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT; //Vector3D
		attributeDescriptions[1].offset = offsetof(Vertex, normal); //Offset upto the normal variable in the vertex struct

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		return attributeDescriptions;
	}
	bool operator==(const Vertex& other) const { //Override the equals operator to check for the same position, normal and texture coords
		return pos == other.pos && normal == other.normal && texCoord == other.texCoord;
	}
};
namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}
//! VulkanObject
/*!
Contains all required variables and functions for storing and displaying mesh and texture data (otherwise known as a gameObject or VulkanObject for this)
*/
class VulkanObject
{
private:
	//! Private vec3.
	/*! Position of the object*/
	glm::vec3 m_Position = glm::vec3(0, 0, 0);
	//! Private vec3.
	/*! Rotation (eular) of the object*/
	glm::vec3 m_Rotation = glm::vec3(0, 0, 0);
	//! Private vec3.
	/*! Scale of the object*/
	glm::vec3 m_Scale = glm::vec3(1, 1, 1);
	//! Private boolean.
	/*! True if the object reseives lighting, if false it is "unlit" and displays the full colour of the albedo texture*/
	bool m_bLit = true;

	//! Private VulkanEngine pointer.
	/*! Used to access the vulkan engine for utility functions*/
	VulkanEngine* m_Engine;
	//! Private VkDevice reference.
	/*! Required to create or destroying most vulkan objects */
	VkDevice& m_Device;
	

	//! Private VkImage, VkDeviceMemory, VkImageView and VkSampler.
	/*! Required components for storing the albedo texture */
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;
	//! Private VkImage, VkDeviceMemory, VkImageView and VkSampler.
	/*! Required components for storing the normal map texture */
	VkImage ntextureImage;
	VkDeviceMemory ntextureImageMemory;
	VkImageView ntextureImageView;
	VkSampler ntextureSampler;
	//! Private VkImage, VkDeviceMemory, VkImageView and VkSampler.
	/*! Required components for storing the specular texture */
	VkImage stextureImage;
	VkDeviceMemory stextureImageMemory;
	VkImageView stextureImageView;
	VkSampler stextureSampler;

	

	//Vector of vertacies each with a position and colour
	std::vector<Vertex> vertices;
	//Index of each vertex in order to make the pyrimid shapes
	std::vector<uint32_t> indices;

	//! Private VkBuffer and VkDeviceMemory.
	/*! Required components for storing vertex infomation */
	VkBuffer m_VertexBuffer;
	VkDeviceMemory m_VertexBufferMemory;
	//! Private VkBuffer and VkDeviceMemory.
	/*! Required components for storing vertex indecies */
	VkBuffer m_IndexBuffer;
	VkDeviceMemory m_IndexBufferMemory;

public:
	//! VulkanObject Contructor
	/*!
	Sets up the VulkanObject included creating texture objects
	\param engine VulkanEngine*, pointer to the vulkan engine
	\param device VkDevice&, logical device referance
	\param graphicsQueue VkQueue, the graphics queue used to render this object
	\param VkCommandPool commandPool, used in the processes for creating objects related to textures
	\param modelPath const char*, text path to the mesh data file
	\param texturePath const char*, text path to the albedo texture file
	\param nTexturePath const char*, text path to the normal texture file
	\param sTexturePath const char*, text path to the specular texture file
	*/
	VulkanObject(VulkanEngine* engine, VkDevice& device, VkQueue graphicsQueue, VkCommandPool commandPool, const char* modelPath, const char* texturePath, const char* nTexturePath, const char* sTexturePath);
	//! VulkanObject Decontructor
	/*!
	Cleans up memory and objects in class
	*/
	~VulkanObject();

	
	//! Public Get and Set functions
	/*! 
	Functions for getting and settings vertecies, indecies, buffers and memory
	*/
	VkBuffer& GetVertexBuffer() { return m_VertexBuffer; }
	VkBuffer& GetIndexBuffer() { return m_IndexBuffer; }
	const std::vector<uint32_t>& GetIndices() { return indices; }
	const std::vector<Vertex>& GetVertices() { return vertices; }
	VkDeviceMemory& GetVertexMemory() { return m_VertexBufferMemory; }
	VkDeviceMemory& GetIndexMemory() { return m_IndexBufferMemory; }

	//! Public Get and Set functions.
	/*!
	Functions for getting and setting transform infomation (position, rotation and scale)
	*/
	const void SetPos(glm::vec3 pos) { m_Position = pos; }
	const glm::vec3 GetPos() const { return m_Position; }
	const void SetRot(glm::vec3 rot) { m_Rotation = rot; }
	const glm::vec3 GetRot() const { return m_Rotation; }
	const void SetScale(glm::vec3 scale) { m_Scale = scale; }
	const glm::vec3 GetScale() const { return m_Scale; }

	//! Public Get and Set functions.
	/*!
	Functions for getting and setting objects related to the albedo texture
	*/
	VkImage& GetTextureImage() { return textureImage; }
	void SetTextureImageView(VkImageView view) { textureImageView = view; }
	VkImageView& GetTextureImageView() { return textureImageView; }
	VkSampler& GetTextureSampler() { return textureSampler; }
	//! Public Get functions.
	/*!
	Functions for getting objects related to the normal texture
	*/
	VkImageView& GetNormalTextureImageView() { return ntextureImageView; }
	VkSampler& GetNormalTextureSampler() { return ntextureSampler; }
	VkImageView& GetSpecTextureImageView() { return stextureImageView; }
	VkSampler& GetSpecTextureSampler() { return stextureSampler; }

	//! Public Lit function.
	/*!
	Returns true if the object is affected by lighting
	*/
	const bool Lit() const { return m_bLit; }
	//! Public SetLit function.
	/*!
	Set to true for the object to be effected by lighting
	*/
	const void SetLit(bool lit) { m_bLit = lit; }

	
	//! Public loadModel function.
	/*!
	Load in model data.
	\param path const char*, text path to mesh data
	*/
	void loadModel(const char* path);

};