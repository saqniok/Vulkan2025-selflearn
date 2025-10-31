#include "Mesh.h"

Mesh::Mesh() 
{
}

Mesh::Mesh(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, std::vector<Vertex> * vertices)
{
	vertexCount = vertices->size();
	physicalDevice = newPhysicalDevice;
	device = newDevice;
	createVertexBuffer(vertices);

}

int Mesh::getVertexCount()
{
	return vertexCount;
}

VkBuffer Mesh::getVertexBuffer()
{
	return vertexBuffer;
}

void Mesh::destroyVertexBuffer()
{
	vkDestroyBuffer(device, vertexBuffer, nullptr);			// Destroy Buffer - Does not free up memory
	vkFreeMemory(device, vertexBufferMemory, nullptr);		// Frees GPU memory - Does not destroy the buffer descriptor
}

Mesh::~Mesh()
{
}

VkBuffer Mesh::createVertexBuffer(std::vector<Vertex>* vertices)
{
	// Information to create a buffer (doeasn't include assinging memory)
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = sizeof(Vertex) * vertices->size();			// Size of buffer (size of 1 vertex * of vertices)
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;			// Multiple types of buffer possible, we want Vertex Buffer
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;				// Similar to Swap Chain images, can share vertex buffers

	VkResult result = vkCreateBuffer(device, &bufferInfo, nullptr, &vertexBuffer);

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create a Vertex Buffer!");

	// Get buffer Memory REQUIREMENTS
	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(device, vertexBuffer, &memoryRequirements);

	// ALLOCATE MEMMORY TO BUFFER
	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;	
	memoryAllocateInfo.memoryTypeIndex = findMemoryTypeIndex(memoryRequirements.memoryTypeBits,		// Index of memory type on Physical Device that has required bit flags
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);				// VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT	: CPU can interact with memory
																									// VK_MEMORY_PROPERTY_HOST_COHERENT_BIT : Allows palcement of data straight into buffer after mapping
	// Allocate memory to VkDeviceMemory
	result = vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &vertexBufferMemory);
	if (result != VK_SUCCESS) throw std::runtime_error("Failed to allocate Vertex Buffer Memory!");

	// Allocate memory to given vertex buffer
	vkBindBufferMemory(device, vertexBuffer, vertexBufferMemory, 0);

	// Map Memory to Vertex Buffer
	void* data;																// 1. Create poitner for normal memory
	vkMapMemory(device, vertexBufferMemory, 0, bufferInfo.size, 0, &data);	// 2. `Map` the vertex buffer memory to that point
	memcpy(data, vertices->data(), (size_t)bufferInfo.size);				// 3. Copy memory from vertices vector to the point
	vkUnmapMemory(device, vertexBufferMemory);								// 4. `Unmap` the vertex buffer memory
}

uint32_t Mesh::findMemoryTypeIndex(uint32_t allowedTypes, VkMemoryPropertyFlags properties)
{
	// Get properties of physicalDevice memory
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		/*
		*	MEMORY TYPE VALIDATION
			0000000100 & 0011001000 return false		&&		0001 & 0010 return false
			0000000100 & 0011001100 return true					0001 & 0001 return true
			== properties; 
		*/
		if ((allowedTypes & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)				// Index of memory types must match correpsonding bit in `allowedTypes`
		{
			// This memory types is valid
			return i;
		}
	}


}
