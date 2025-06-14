﻿#pragma once

#include <fstream>

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
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