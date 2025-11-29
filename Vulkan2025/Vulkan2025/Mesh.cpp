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

void Mesh::createVertexBuffer(std::vector<Vertex>* vertices)
{
	// Get size of buffer neede for vertices
	VkDeviceSize bufferSize = sizeof(Vertex) * vertices->size();			// Size of buffer (size of 1 vertex * of vertices)

	// Temporary buffer to "stage" vertex data before tranferring to GPU memory
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	// Create Buffer and Allocate Memory to it
	createBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&stagingBuffer, &stagingBufferMemory);

	// Map Memory to Vertex Buffer
	void* data;																// 1. Create poitner for normal memory
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);		// 2. `Map` the vertex buffer memory to that point
	memcpy(data, vertices->data(), (size_t)bufferSize);						// 3. Copy memory from vertices vector to the point
	vkUnmapMemory(device, stagingBufferMemory);								// 4. `Unmap` the vertex buffer memory

	// Create buffer with TRANSFER_DST_BIT to mark as recipient transfer data + vertexBuffer
	// Buffer memory is to be DEVICE_LOCAL_BIT meaning memory is on GPU and only accessible by it and not CPU(host)
	createBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &vertexBuffer, &vertexBufferMemory);

}

