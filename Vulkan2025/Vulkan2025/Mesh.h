#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include "Utilities.h"
class Mesh
{
	Mesh();
	Mesh(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, std::vector<Vertex>* vertices);
	// VkPhysicalDevice will handle memory managment
	// VkDevice to be handle the buffer itself

	int getVertexCount();
	VkBuffer getVertexBuffer();

	
private:
	int vertexCount;
	VkBuffer vertexVuffer;

	VkPhysicalDevice physicalDevice;
	VkDevice device;

	void createVertexBuffer(std::vector<Vertex> * vertices);
};

