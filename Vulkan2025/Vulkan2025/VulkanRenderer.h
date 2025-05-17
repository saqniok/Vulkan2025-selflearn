#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>	// ������������� ����������� ����������, ��������� � ����������� � ��������������� ��������
						/*
						*	std::runtime_error		- ��� ������ ������� ����������
						*	std::logic_error		- ��� ���������� ������ (�������� �����������) 
						*	std::out_of_range		- ���� ��������� � �������, �� ������� ������ ����
						*	std::invalid_argument	- ���� ������� ����������� �������� � �������
						*/
#include <vector>		// ������������ ������ TODO: ��������� � ���� �������� � ������ ��� ����� push_back
#include <set>			// std::set ��������������� ����������� ��������, ��������� �����.
#include <algorithm>	// ��������� STL: find, sort, cout, min, max...

#include "Utilities.h"

class VulkanRenderer
{
public:
	VulkanRenderer();

	int init(GLFWwindow* newWindow);
	void cleanup();

	~VulkanRenderer();

private:
	GLFWwindow* window;

	// Vulkan Components
	// - main components
	VkInstance instance;	//	������� ������, ������������ ���������� ������ Vulkan. ����� ���� ������������� ����������, ����� � �.�.
	
	/*
	*	VkPhysicalDevice	- GPU ������� ��������
	*	VkDevice			- ���������� ����������, "�����" ����� ����������� � ���, ��� ������������� �������, ��������� ��������� ������� �� ���
	*/
	struct {										//*	������� ����� ��������� �� ����������, ������ � ������� ���� �� ���������� 
		VkPhysicalDevice physicalDevice;			//*	������ ������ ������, �� ������� ������������ ���
		VkDevice logicalDevice;						//*	struct mainDevice{} - ������� ��� mainDevice, ����� �������� ����� ������� ��������
	} mainDevice;									//*	��������� ���� mainDevice device;
	
	
	VkQueue graphicsQueue;							//	������� ��� �������� ����������� ������, Vulkan ������� ������� � ����� ������� ���������� �������			
	VkQueue presentationQueue;						//	������� ��� ������ ����������� �� ������. ����� ��������� � �������� graphicQueue			
	VkSurfaceKHR surface;							//	��� ����������� ������ �����������, �������� � ����� GLFW. Vulkan �������� �� �������� � ����, � � surface
	VkSwapchainKHR swapchain;						//	��� ����� �����������, ����� �������� ������������, ��� �� �������� �������� �� �����
	
	// ���� �������� ��� SwapchainImage, ���������� � ���� VkImage � VkImageView, �� ������� ������ ��������
	std::vector<SwapchainImage> swapChainImages;	

	// - Utility
	VkFormat swapChainImageFormat;		//	������ �������� ����������� �� swapchain. ��������, VK_FORMAT_B8G8R8A8_SRGB � 8 ��� �� �����, ������� �����.
	VkExtent2D swapChainExtent;			//	������ swapchain-� (������ � ������ ��������, ������� �� ���������).

	// Vulkan Functions
	// - Create Functions
	void createIntstance();
	void createLogicalDevice();
	void createSurface();
	void createSwapChain();
	void createGraphicPipeline();

	// - Get Functions
	void getPhysicalDevice();

	// - Support Functions
	// -- Checker Functions
	bool checkInstanceExtensionSupport(std::vector<const char*>* checkExtensions);
	bool checkDeviceExtentionSupport(VkPhysicalDevice device);
	bool checkDeviceSuitable(VkPhysicalDevice device);


	// -- Geter Functions
	QueueFamilyIndices getQueueFamilies(VkPhysicalDevice device);
	SwapChainDetails getSwapchainDetails(VkPhysicalDevice device);

	// -- CHOOSE Functions
	VkSurfaceFormatKHR chooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &formats);
	VkPresentModeKHR chooseBestPresentationMode(const std::vector<VkPresentModeKHR> &presentationModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &surfaceCapabilities);

	// -- Create Support Functions
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	VkShaderModule createShaderModule(const std::vector<char> &code);
};

