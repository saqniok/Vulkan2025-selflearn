#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include "Utilities.h"
class Mesh
{
public:
	Mesh();

	Mesh(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool,
		std::vector<Vertex>* vertices, std::vector<uint32_t>* indices);
	// VkPhysicalDevice will handle memory managment
	// VkDevice to be handle the buffer itself

	int getVertexCount();
	VkBuffer getVertexBuffer();

	int getIndexCount();
	VkBuffer getIndexBuffer();

	void destroyBuffers();

	~Mesh();

	
private:
	int vertexCount;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;

	int indexCount;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	VkPhysicalDevice physicalDevice;
	VkDevice device;

	void createVertexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<Vertex> * vertices);
	void createIndexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<uint32_t>* indices);
	uint32_t findMemoryTypeIndex(uint32_t allowedTypes, VkMemoryPropertyFlags properties);
};

