		//// #include <vulkan/vulkan.h> Это нормальная практика включать вулкан в прокет, но за нас это может сделать glfw
		//#define GLFW_INCLUDE_VULKAN // поискать в glfw3.h что это, автоматичекское подключение vulkan.h от glwf. так же в будущем меньше может быть конфликтов
		//#include <GLFW/glfw3.h> 
		/////////////////////////////////////////////////////////
		////////		#if defined(GLFW_INCLUDE_VULKAN)	///////
		////////			#include <vulkan/vulkan.h>		///////
		////////		#endif / Vulkan header /			///////
		/////////////////////////////////////////////////////////
		//
		//#define GLM_FORCE_RADIANS // Защита от ошибок с градусами. GLM может использовать как радианы так и градусы, но OpenGL и Vulkan используют только радианы.
		//#define GLM_FORCE_DEPTH_ZERO_TO_ONE // Z-буфер для Vulkan. Изменяет диапозон с (-1,1)OpenGL на (0,1)Vulkan, что бы нормально работальки Vulkan формулы матриц
		//#include <glm/glm.hpp> // включает в себя другие include с мат формулами glm - graphic lib math
		//#include <glm/mat4x4.hpp>
		//
		//#include <iostream>
		//
		//int main()
		//{
		//	glfwInit(); //Инициализирует библиотеку GLFW. Это обязательный вызов перед созданием окна.
		//
		//	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); //По умолчанию GLFW ожидает, что ты будешь использовать OpenGL.
		//	/*
		//	*	GLFW_CLIENT_API — указывает, какой графический API будет использоваться.
		//	*	GLFW_NO_API - говорит GLFW не создавать контекст OpenGL, так как мы будем использовать Vulkan. 
		//	*/
		//	GLFWwindow * window = glfwCreateWindow(800, 600, "TEST WINDOW", nullptr, nullptr);
		//
		//	// Проверка кол-ва extentions, int(-n,n). uint(0,n);
		//	uint32_t extenstionCount = 0;
		//	// функции из Vulkan в основном все начанаются на vk...
		//	vkEnumerateInstanceExtensionProperties(nullptr, &extenstionCount, nullptr); // Эта функция выводит кол-во extensions
		//		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
		//		//////			Как работает ?																										//////
		//		//////			Функцию вызывают в два шага :																						//////
		//		//////			Сначала получаем количество доступных расширений(чтобы выделить память). Потом заполняем массив с расширениями.		//////
		//		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//	printf("Extenstion count: %i\n", extenstionCount);
		//	
		//	glm::mat4 testMatrix(1.f);
		//	glm::vec4 testVector(1.f);
		//
		//	auto testResult = testMatrix * testVector;
		//
		//	while (!glfwWindowShouldClose(window))
		//	{
		//		glfwPollEvents();
		//	}
		//
		//	glfwDestroyWindow(window);
		//	glfwTerminate();
		//	return 0;
		//}
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept> // проверить что это?
#include <vector>
#include <iostream>

#include "VulkanRenderer.h"

GLFWwindow* window;
VulkanRenderer vulkanRenderer;

void initWindow(std::string wName = "TEST WINDOW", const int width = 800, const int heigth = 600) 
{
	// Initialise GLFW
	glfwInit();

	// set GLFW to NOT wortk with OpenGL
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(width, heigth, wName.c_str(), nullptr, nullptr);
}

int main()
{
	//Create Window
	initWindow();

	//Create Vulkan Rendere Instance
	if (vulkanRenderer.init(window) == EXIT_FAILURE) return EXIT_FAILURE;

	//Loop until close
	while (!glfwWindowShouldClose(window)) glfwPollEvents();

	vulkanRenderer.cleanup();

	//Destroy GLFW window and STOP GLFW
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}