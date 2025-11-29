#include "Mesh.h"

Mesh::Mesh() 
{
}

Mesh::Mesh(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool, 
	std::vector<Vertex>* vertices, std::vector<uint32_t>* indices)
{
	vertexCount = vertices->size();
	indexCount = indices->size();
	physicalDevice = newPhysicalDevice;
	device = newDevice;
	createVertexBuffer(transferQueue, transferCommandPool, vertices);
	createIndexBuffer(transferQueue, transferCommandPool, indices);
}

int Mesh::getVertexCount()
{
	return vertexCount;
}

VkBuffer Mesh::getVertexBuffer()
{
	return vertexBuffer;
}

int Mesh::getIndexCount()
{
	return indexCount;
}

VkBuffer Mesh::getIndexBuffer()
{
	return indexBuffer;
}

void Mesh::destroyBuffers()
{
	vkDestroyBuffer(device, vertexBuffer, nullptr);			// Destroy Buffer - Does not free up memory
	vkFreeMemory(device, vertexBufferMemory, nullptr);		// Frees GPU memory - Does not destroy the buffer descriptor
	vkDestroyBuffer(device, indexBuffer, nullptr);
	vkFreeMemory(device, indexBufferMemory, nullptr);
}

Mesh::~Mesh()
{
}

void Mesh::createVertexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<Vertex>* vertices)
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

	// Copy data from staging buffer to vertex buffer on GPU
	copyBuffer(device, transferQueue, transferCommandPool, stagingBuffer, vertexBuffer, bufferSize);

	// Destroy staging buffer and free memory
	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void Mesh::createIndexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<uint32_t>* indices)
{
	// Get size of buffer neede for indices
	VkDeviceSize bufferSize = sizeof(uint32_t) * indices->size();

	// Temporary buffer to "stage" index data before tranferring to GPU memory
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	createBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&stagingBuffer, &stagingBufferMemory);

	// Map Memory to Index Buffer
	void* data;																// 1. Create poitner for normal memory
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);		// 2. `Map` the index buffer memory to that point
	memcpy(data, indices->data(), (size_t)bufferSize);						// 3. Copy memory from index vector to the point
	vkUnmapMemory(device, stagingBufferMemory);								// 4. `Unmap` the index buffer memory


	// Create buffer for Index Data on GPU access only area
	createBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&indexBuffer, &indexBufferMemory);

	// Copy data from staging buffer to index buffer on GPU 
	copyBuffer(device, transferQueue, transferCommandPool, stagingBuffer, indexBuffer, bufferSize);

	// Destroy staging buffer and free memory
	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr); 
}

