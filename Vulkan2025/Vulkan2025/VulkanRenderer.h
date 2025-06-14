﻿#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>	// Предоставляет стандартные исключения, связанные с логическими и арифметическими ошибками
						/*
						*	std::runtime_error		- для ошибок времени выполнения
						*	std::logic_error		- для логических ошибок (например предусловия) 
						*	std::out_of_range		- если обращение к вектору, но индекса такого нету
						*	std::invalid_argument	- если передан некоректный агрумент в функцию
						*/
#include <vector>		// динамический массив TODO: покапатся в этих массивах и понять что такое push_back
#include <set>			// std::set отсортировывает уникальлные элементы, исключает дубли.
#include <algorithm>	// алшоритмы STL: find, sort, cout, min, max...
#include <array>		// std::array - статический массив, размер которого задается при компиляции, в отличии от std::vector, который динамический	

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
	VkInstance instance;	//	базовый обьект, представляет приложение внутри Vulkan. Через него запрашиваются расширения, дебаг и т.д.
	
	/*
	*	VkPhysicalDevice	- GPU которое доступно
	*	VkDevice			- логическое устройство, "связь" между приложением и ГПУ, оно предоставляет очереди, позволяет отпралять команды на ГПУ
	*/
	struct {										//*	Создаем сразу переменую со структурой, удобно и валидно если мы используем 
		VkPhysicalDevice physicalDevice;			//*	только внутри класса, не засоряя пространство имён
		VkDevice logicalDevice;						//*	struct mainDevice{} - создает тип mainDevice, после которого нужно создать отдельно
	} mainDevice;									//*	переменую типа mainDevice device;
	
	
	VkQueue graphicsQueue;							//	Очередь для отправки графических команд, Vulkan требует указать в какую очередь отправлять команду			
	VkQueue presentationQueue;						//	Очередь для показа изображения на экране. Может совпадать с очередью graphicQueue			
	VkSurfaceKHR surface;							//	Это ПОВЕРХНОСТЬ ВЫВОДА изображения, связаное с окном GLFW. Vulkan рендерит не напрямую в окно, а в surface
	VkSwapchainKHR swapchain;						//	Это набор изображений, между которыми перелючается, что бы показать картинку на экран
	
	// Есть созданый тип SwapchainImage, включающий в себя VkImage и VkImageView, мы создаем вектор обьектов
	std::vector<SwapchainImage> swapChainImages;	

	// - Pipeline components
	VkPipeline graphicsPipeline;
	VkPipelineLayout pipelineLayout;
	VkRenderPass renderPass;

	// - Utility
	VkFormat swapChainImageFormat;		//	Формат пикселей изображений из swapchain. Например, VK_FORMAT_B8G8R8A8_SRGB — 8 бит на канал, включая альфу.
	VkExtent2D swapChainExtent;			//	Размер swapchain-а (ширина и высота картинки, которую ты рендеришь).

	// Vulkan Functions
	// - Create Functions
	void createIntstance(); // корневой объект Vulkan.
	/**
	* Внутри:
	*	Название приложения,	
	*	Версию Vulkan,	
	*	Расширения (например, VK_KHR_surface),
	*	Валидационные слои (например, VK_LAYER_KHRONOS_validation).
	*/

	void createLogicalDevice(); // Создаёт логическое устройство (VkDevice) для выбранного GPU.
	/*
	* Внутри:
	*	Настраиваются очереди: graphics, present, compute и т.д.
	*	Указываются расширения (VK_KHR_swapchain обязательно).
	*	Получаешь VkQueue — чтобы отправлять команды в GPU.
	*/

	void createSurface(); // Создаёт surface (VkSurfaceKHR) — поверхность вывода для окна.
	/*
	*	Она нужна, чтобы Vulkan понимал, куда рендерить (в окно).
	*	GLFW умеет создавать surface через glfwCreateWindowSurface(instance, window, ...).
	*	Этот объект используется позже при создании swapchain.
	*/

	void createSwapChain(); // Cоздаёт swapchain — цепочку изображений, которые будут отображаться на экране поочерёдно.
	/*
	*	формат изображения (VkFormat, например, VK_FORMAT_B8G8R8A8_UNORM),
	*	количество буферов (обычно 2 или 3),
	*	режим синхронизации (vsync или нет),
	*	разрешение и surface capabilities.
	*/

	void createRenderPass(); // Создаёт render pass — это описание этапов отрисовки кадра.
	/*
	*	Сколько буферов (color, depth, stencil),
	*	Как они очищаются,
	*	Какой layout используется до/после рендеринга,
	*	Какие подпроходы (subpasses).
	*/

	void createGraphicPipeline(); // Создаёт графический пайплайн (VkPipeline) — самый сложный объект.
	/*
	*	vertex/fragment шейдеры,
	*	структура вершин (binding + attributes),
	*	viewport + scissor,
	*	rasterization,
	*	blending,
	*	pipeline layout (с push-константами и дескрипторами),
	*	render pass, с которым pipeline будет совместим.
	*/


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

