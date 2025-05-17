		//// #include <vulkan/vulkan.h> ��� ���������� �������� �������� ������ � ������, �� �� ��� ��� ����� ������� glfw
		//#define GLFW_INCLUDE_VULKAN // �������� � glfw3.h ��� ���, ��������������� ����������� vulkan.h �� glwf. ��� �� � ������� ������ ����� ���� ����������
		//#include <GLFW/glfw3.h> 
		/////////////////////////////////////////////////////////
		////////		#if defined(GLFW_INCLUDE_VULKAN)	///////
		////////			#include <vulkan/vulkan.h>		///////
		////////		#endif / Vulkan header /			///////
		/////////////////////////////////////////////////////////
		//
		//#define GLM_FORCE_RADIANS // ������ �� ������ � ���������. GLM ����� ������������ ��� ������� ��� � �������, �� OpenGL � Vulkan ���������� ������ �������.
		//#define GLM_FORCE_DEPTH_ZERO_TO_ONE // Z-����� ��� Vulkan. �������� �������� � (-1,1)OpenGL �� (0,1)Vulkan, ��� �� ��������� ���������� Vulkan ������� ������
		//#include <glm/glm.hpp> // �������� � ���� ������ include � ��� ��������� glm - graphic lib math
		//#include <glm/mat4x4.hpp>
		//
		//#include <iostream>
		//
		//int main()
		//{
		//	glfwInit(); //�������������� ���������� GLFW. ��� ������������ ����� ����� ��������� ����.
		//
		//	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); //�� ��������� GLFW �������, ��� �� ������ ������������ OpenGL.
		//	/*
		//	*	GLFW_CLIENT_API � ���������, ����� ����������� API ����� ��������������.
		//	*	GLFW_NO_API - ������� GLFW �� ��������� �������� OpenGL, ��� ��� �� ����� ������������ Vulkan. 
		//	*/
		//	GLFWwindow * window = glfwCreateWindow(800, 600, "TEST WINDOW", nullptr, nullptr);
		//
		//	// �������� ���-�� extentions, int(-n,n). uint(0,n);
		//	uint32_t extenstionCount = 0;
		//	// ������� �� Vulkan � �������� ��� ���������� �� vk...
		//	vkEnumerateInstanceExtensionProperties(nullptr, &extenstionCount, nullptr); // ��� ������� ������� ���-�� extensions
		//		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
		//		//////			��� �������� ?																										//////
		//		//////			������� �������� � ��� ���� :																						//////
		//		//////			������� �������� ���������� ��������� ����������(����� �������� ������). ����� ��������� ������ � ������������.		//////
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

#include <stdexcept> // ��������� ��� ���?
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