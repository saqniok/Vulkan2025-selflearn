#pragma once

#include <fstream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

// Vertex data reperesantation
struct Vertex {
	glm::vec3 pos; // Vertex position (xyz)
	glm::vec3 col; // Vertex colour (rgb)
};

// Indices (locations) of Queue Famalies (if they exist at all)
struct QueueFamilyIndices {
	int graphicsFamily = -1;			// Location of Graphic Queue Family
	int presentationFamily = -1;		// Location of Presentation Queue Family

	// Check if queue families are valid
	bool isValid()
	{
		return graphicsFamily >= 0 && presentationFamily >= 0;
	}
};

struct SwapChainDetails {
	VkSurfaceCapabilitiesKHR surfaceCapabilities;		// Surface properties. e.g. image size/extent
	std::vector<VkSurfaceFormatKHR> formats;			// Surface image formats, e.g. RGB and size of each colour
	std::vector<VkPresentModeKHR> presentationModes;		// How images should be presented to screen
};

/* Создаем тип, который обьеденяет VkImage и VkImageView, потому что мы хотим эти данные иметь в одном месте.
*	без этой структуры, код выглядел бы так
*	std::vector<VkImage> image;
*	std::vector<VkImageView;
*/
struct SwapchainImage {
	VkImage image;
	VkImageView imageView;
};

static std::vector<char> readFile(const std::string& filename)
{
	// Open stream from given file
	// std::ios::binary tells stream to read file as binary
	// std::ios::ate tells stream to start reading from end of file
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	// Check if file stream successfully opened
	if (!file.is_open()) throw std::runtime_error("Failed to open a file!");

	size_t fileSize = (size_t)file.tellg();	// tellg - Tell get-Position
	std::vector<char> fileBuffer(fileSize); // мы теперь знаем размер файла, потому что std::ios::ate показывает последний байт

	// Move read position (seek to) the start of the file
	file.seekg(0);

	// Read the file data into the buffer (stream "fileSize" in total)
	file.read(fileBuffer.data(), fileSize);

	// If you use the stream, aftre we must close it
	file.close();

	return fileBuffer;
}

static uint32_t findMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t allowedTypes, VkMemoryPropertyFlags properties)
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

static void createBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsage, 
	VkMemoryPropertyFlags bufferProperties, VkBuffer* buffer, VkDeviceMemory* bufferMemory) 
{
	// Information to create a buffer (doeasn't include assinging memory)
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = bufferSize;									// Size of buffer (size of 1 vertex * of vertices)
	bufferInfo.usage = bufferUsage;									// Multiple types of buffer possible, we want Vertex Buffer
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;				// Similar to Swap Chain images, can share vertex buffers

	VkResult result = vkCreateBuffer(device, &bufferInfo, nullptr, buffer);

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create a Vertex Buffer!");

	// Get buffer Memory REQUIREMENTS
	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(device, *buffer, &memoryRequirements);

	// ALLOCATE MEMMORY TO BUFFER
	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = findMemoryTypeIndex(physicalDevice, memoryRequirements.memoryTypeBits,		// Index of memory type on Physical Device that has required bit flags
		bufferProperties | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);													// VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT	: CPU can interact with memory
																													// VK_MEMORY_PROPERTY_HOST_COHERENT_BIT : Allows palcement of data straight into buffer after mapping
																													// Allocate memory to VkDeviceMemory
	result = vkAllocateMemory(device, &memoryAllocateInfo, nullptr, bufferMemory);
	if (result != VK_SUCCESS) throw std::runtime_error("Failed to allocate Vertex Buffer Memory!");

	// Allocate memory to given vertex buffer
	vkBindBufferMemory(device, *buffer, *bufferMemory, 0);
}

static void copyBuffer(VkDevice device, VkQueue transferQueue, VkCommandPool transferCommandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize)
{
	//Command buffer to hold transfer commands
	VkCommandBuffer transferCommandBuffer;

	// Command buffer details 
	VkCommandBufferAllocateInfo alocInfo = {};
	alocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;						// Primary Buffer
	alocInfo.commandPool = transferCommandPool;								// Buffer to allocate from
	alocInfo.commandBufferCount = 1;										// One buffer allocated

	// Alocate/Begin command buffer from pool
	vkAllocateCommandBuffers(device, &alocInfo, &transferCommandBuffer);

	// Information to begin the command buffer recording
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;			// Buffer only used once

	// Begin recording transfer commands to command buffer
	vkBeginCommandBuffer(transferCommandBuffer, &beginInfo);

	// Copy region details
	VkBufferCopy bufferCopyRegion = {};
	bufferCopyRegion.srcOffset = 0;				// Optional
	bufferCopyRegion.dstOffset = 0;				// Optional
	bufferCopyRegion.size = bufferSize;			// Size of data to copy

	// Command to copy buffer
	vkCmdCopyBuffer(transferCommandBuffer, srcBuffer, dstBuffer, 1, &bufferCopyRegion);

	vkEndCommandBuffer(transferCommandBuffer);

	// Queue submission information
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &transferCommandBuffer;

	// Submit to transfer queue and wait until completed
	vkQueueSubmit(transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(transferQueue);

	// Free temporary command buffer
	vkFreeCommandBuffers(device, transferCommandPool, 1, &transferCommandBuffer);  
}