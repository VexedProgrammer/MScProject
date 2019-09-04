#include "VulkanObject.h"

#include "VulkanEngine.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h> 

VulkanObject::VulkanObject(VulkanEngine* engine, VkDevice& device, VkQueue graphicsQueue, VkCommandPool commandPool, const char* modelPath, const char* texturePath, const char* nTexturePath, const char* sTexturePath) : m_Device(device)
{
	m_Engine = engine;

	loadModel(modelPath);

	m_Engine->createVertexBuffer(graphicsQueue, commandPool, this);
	m_Engine->createIndexBuffer(graphicsQueue, commandPool, this);

	m_Engine->createTextureImage(graphicsQueue, commandPool, textureImage, textureImageMemory, texturePath);
	m_Engine->createTextureImageView(this, textureImageView, textureImage);
	m_Engine->createTextureSampler(this, textureSampler);

	m_Engine->createTextureImage(graphicsQueue, commandPool, ntextureImage, ntextureImageMemory, nTexturePath);
	m_Engine->createTextureImageView(this, ntextureImageView, ntextureImage);
	m_Engine->createTextureSampler(this, ntextureSampler);

	m_Engine->createTextureImage(graphicsQueue, commandPool, stextureImage, stextureImageMemory, sTexturePath);
	m_Engine->createTextureImageView(this, stextureImageView, stextureImage);
	m_Engine->createTextureSampler(this, stextureSampler);
}
VulkanObject::~VulkanObject()
{

	//Clean up index buffer
	vkDestroyBuffer(m_Device, m_IndexBuffer, nullptr);
	vkFreeMemory(m_Device, m_IndexBufferMemory, nullptr);

	//clean up vertex buffer
	vkDestroyBuffer(m_Device, m_VertexBuffer, nullptr);
	vkFreeMemory(m_Device, m_VertexBufferMemory, nullptr);

	//Cleanup Texture
	vkDestroyImage(m_Device, textureImage, nullptr);
	vkDestroyImageView(m_Device, textureImageView, nullptr);
	vkDestroySampler(m_Device, textureSampler, nullptr);
	vkFreeMemory(m_Device, textureImageMemory, nullptr);

	vkDestroyImage(m_Device, ntextureImage, nullptr);
	vkDestroyImageView(m_Device, ntextureImageView, nullptr);
	vkDestroySampler(m_Device, ntextureSampler, nullptr);
	vkFreeMemory(m_Device, ntextureImageMemory, nullptr);

	vkDestroyImage(m_Device, stextureImage, nullptr);
	vkDestroyImageView(m_Device, stextureImageView, nullptr);
	vkDestroySampler(m_Device, stextureSampler, nullptr);
	vkFreeMemory(m_Device, stextureImageMemory, nullptr);

}

void VulkanObject::loadModel(const char * path)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path)) {
		throw std::runtime_error(warn + err);
	}

	std::unordered_map<Vertex, uint32_t> uniqueVertices = {};

	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			Vertex vertex = {};

			vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.normal = {
				attrib.normals[3 * index.normal_index + 0],
				attrib.normals[3 * index.normal_index + 1],
				attrib.normals[3 * index.normal_index + 2]
			};

			vertex.texCoord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}

			indices.push_back(uniqueVertices[vertex]);
		}
	}
}


