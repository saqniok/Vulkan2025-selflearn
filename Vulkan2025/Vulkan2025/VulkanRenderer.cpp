#include "VulkanRenderer.h"

VulkanRenderer::VulkanRenderer()
{
}

int VulkanRenderer::init(GLFWwindow* newWindow)
{

	window = newWindow; // Сохраняем переданное окно (GLFWwindow*) в член класса window, чтобы использовать его внутри других методов (createSurface, например).

	try {
		createIntstance();
		createSurface();
		getPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createRenderPass();
		createGraphicPipeline();
	}
	catch (const std::runtime_error& e) {
		printf("ERROR: %s\n", e.what());
		return EXIT_FAILURE;
		// Если на любом этапе произойдёт ошибка (например, не найден GPU или не удалось создать swapchain),
		// программа поймает std::runtime_error и выведет сообщение.
	}

	return 0;
}

void VulkanRenderer::cleanup()
{
	vkDestroyPipelineLayout(mainDevice.logicalDevice, pipelineLayout, nullptr);
	for (auto image : swapChainImages) vkDestroyImageView(mainDevice.logicalDevice, image.imageView, nullptr);
	vkDestroySwapchainKHR(mainDevice.logicalDevice, swapchain, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyDevice(mainDevice.logicalDevice, nullptr);
	vkDestroyInstance(instance, nullptr);

}

VulkanRenderer::~VulkanRenderer()
{
}

void VulkanRenderer::createIntstance()
{
	// Creation Aplication Info
	// Most data here doeasn't affect the program and is for developer convenience
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;		// The type of this structuren
	appInfo.pApplicationName = "Vulkan Application";		// Custom name of the application
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);	// Custom Version of the application
	appInfo.pEngineName = "No Engine";						// Custom name of the engine
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);		// Custom Version of the engine
	appInfo.apiVersion = VK_API_VERSION_1_0;				// Version of the Vulkan API

	// Creation infromation for a VkInstance 
	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo; // "p"ApplicationInfo is a pointer to the appInfo structure

	//Create list to hold instance extension
	std::vector<const char*> instanceExtensions = std::vector<const char*>();

	// Set up extensions Instance will use
	uint32_t glfwExtensionCount = 0;						// GLF may require multiple extensions
	const char** glfwExtensions;							// Extensions passed as array of cstrings, so need pointer (the array) to pointer (the cstrings)
	
	// Get GLFW extensions
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	
	// Add GLFW extension to list of extensions
	for (size_t i = 0; i < glfwExtensionCount; i++) instanceExtensions.push_back(glfwExtensions[i]);

	// Check Instance Extensions supported
	if (!checkInstanceExtensionSupport(&instanceExtensions)) throw std::runtime_error("VkInstance does not support required extensions!");

	createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
	createInfo.ppEnabledExtensionNames = instanceExtensions.data();

	//TODO: Set up Validation Layers that Instance will use
	createInfo.enabledLayerCount = 0;
	createInfo.ppEnabledLayerNames = nullptr;

	// Create Instance
	VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
	if(result != VK_SUCCESS) throw std::runtime_error("Failed to create Vulkan Instance!");
}

void VulkanRenderer::createLogicalDevice()
{
	// Get the queue family indices for the chosen Physical Device
	QueueFamilyIndices indices = getQueueFamilies(mainDevice.physicalDevice);

	// Vector for queue creation information, and set for family indices.
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<int> queueFamilyIndices = { indices.graphicsFamily, indices.presentationFamily };

	// Queues the logical device needs to create and info do so
	for(int queueFamilyIndex : queueFamilyIndices)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamilyIndex;					// The index of the family to create a queue from
		queueCreateInfo.queueCount = 1;											// Number of queue to create
		float priority = 1.f;													// MAX 1.f 
		queueCreateInfo.pQueuePriorities = &priority;							// Vulkan needs to know how to  handle multiple queues, so decide priority ( 1 = highest priority)
	
		queueCreateInfos.push_back(queueCreateInfo);
	}


	// Information to create Logical device ( sometimes called "device")
	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());		// Number of Queue Create Info
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();								// List of queue create infos so device can create required queues
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());	// Number of enabled logical device extensions
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();											// List of enables logical device extensions
	
	// Physical Device Features the logical Device will be using
	VkPhysicalDeviceFeatures deviceFeatures = {};
	//deviceFeatures.depthClamp = VK_TRUE;


	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;			// Physical Device features Logical Device will use

	// Create the logical device for the given physical device
	VkResult result = vkCreateDevice(mainDevice.physicalDevice, &deviceCreateInfo, nullptr, &mainDevice.logicalDevice);
	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create a Logical Device!");

	// Queues are created at the same time as the device
	// So we want handle to quese
	// From given logical devicem of given Queue Family, of given Queue Index (0 since only one queue), place reference in graphicsQueue
	vkGetDeviceQueue(mainDevice.logicalDevice, indices.graphicsFamily, 0, &graphicsQueue);
	vkGetDeviceQueue(mainDevice.logicalDevice, indices.presentationFamily, 0, &presentationQueue);
}

void VulkanRenderer::createSurface()
{
	// Create Surface (creating a surface craete infto struct, runs the create surface function, returns result)
	VkResult result = glfwCreateWindowSurface(instance, window, nullptr, &surface);
	if (result != VK_SUCCESS) throw std::runtime_error("Failed to Create Vulkan Surface!");
}

void VulkanRenderer::createSwapChain()
{
	// Get SwapChainDetails so we can pick best settings
	SwapChainDetails swapChainDetails = getSwapchainDetails(mainDevice.physicalDevice);

	// 1. CHOOSE BEST SURFACE FORMAT
	VkSurfaceFormatKHR surfaceFormat = chooseBestSurfaceFormat(swapChainDetails.formats);
	// 2. CHOOSE BEST PRESENTATION MODE
	VkPresentModeKHR presentMode = chooseBestPresentationMode(swapChainDetails.presentationModes);
	// 3. CHOOSE SWAP CHAIN IMAGE RESOLUTION
	VkExtent2D extent = chooseSwapExtent(swapChainDetails.surfaceCapabilities);

	//How many images are in the swapChain? Get 1 more than the minimum to allow triple buffering
	uint32_t imageCount = swapChainDetails.surfaceCapabilities.minImageCount + 1;

	// If imageCount higher than max, then clamp down to max
	// If 0, then limitless
	if (swapChainDetails.surfaceCapabilities.maxImageCount > 0
		&& swapChainDetails.surfaceCapabilities.maxImageCount < imageCount)
	{
		imageCount = swapChainDetails.surfaceCapabilities.maxImageCount;
	}

	// Creationt infomation for swapChain
	VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
	swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;					// 1000001000
	swapChainCreateInfo.surface = surface;														// SwapChain Surface
	swapChainCreateInfo.imageFormat = surfaceFormat.format;										// swapChain format
	swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;								// swapChain color space
	swapChainCreateInfo.presentMode = presentMode;												// swapChain presentation mode
	swapChainCreateInfo.imageExtent = extent;													// swapChain image extents	
	swapChainCreateInfo.minImageCount = imageCount;												// Minimum images in swapchains
	swapChainCreateInfo.imageArrayLayers = 1;													// Number of layers for each imgae in chain
	swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;						// 0x00000010, What attachment images will be used as.
	swapChainCreateInfo.preTransform = swapChainDetails.surfaceCapabilities.currentTransform;	// Transform to perfom on swap chain images
	swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;						// 0x00000001, how to handle blending images with external graphics (e.g. other windows)
	swapChainCreateInfo.clipped = VK_TRUE;														// Wheter to clip parts of image not in view (e.g. behind another window/screen)

	// Get Queue Family indices
	QueueFamilyIndices indices = getQueueFamilies(mainDevice.physicalDevice);

	// If Graphics and Presentation families ar different, then swapchain must let images be shared between families
	if (indices.graphicsFamily != indices.presentationFamily)
	{
		// Queues to share betweem
		uint32_t queueFamilyIndices[] = {
			(uint32_t)indices.graphicsFamily,
			(uint32_t)indices.presentationFamily
		};

		swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;		// VK_SHARING_MODE_CONCURRENT = 1, Image share handling
		swapChainCreateInfo.queueFamilyIndexCount = 2;							// Number of queues to share images between
		swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;			// Array of queues to share between
	}
	else
	{
		swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;		// VK_SHARING_MODE_EXCLUSIVE = 0
		swapChainCreateInfo.queueFamilyIndexCount = 0;
		swapChainCreateInfo.pQueueFamilyIndices = nullptr;
	}

	// If old swap chain been destroyed and this one replaces it, then link old one to quickly hand over responsibiliteis
	swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	// Create SwapChain
	VkResult result = vkCreateSwapchainKHR(mainDevice.logicalDevice, &swapChainCreateInfo, nullptr, &swapchain);
	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create SwapChain!");


	// Store for later reference
	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;

	// Get swap chain images ( irst count, then values)
	uint32_t swapChainImageCount;
	vkGetSwapchainImagesKHR(mainDevice.logicalDevice, swapchain, &swapChainImageCount, nullptr);
	std::vector<VkImage> images(swapChainImageCount);
	vkGetSwapchainImagesKHR(mainDevice.logicalDevice, swapchain, &swapChainImageCount, images.data());

	for (VkImage image : images)
	{
		//Store image handle
		SwapchainImage swapChainImage = {};
		swapChainImage.image = image;
		swapChainImage.imageView = createImageView(image, swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);

		// Add to swapchain image list
		swapChainImages.push_back(swapChainImage);

	}
}

void VulkanRenderer::createRenderPass()
{
	//Color attachment of render pass
	VkAttachmentDescription colourAttachment = {}; // Создаётся структура, описывающая один цветовой буфер (attachment), который будет использоваться в render pass'е.
	colourAttachment.format = swapChainImageFormat; // Указываем формат пикселей, с которым будет работать этот буфер.
	colourAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // один сэмпл на пиксель, т.е. без MSAA (anti-aliasing).
	/*
	* Если включать MSAA, то здесь нужно будет поставить VK_SAMPLE_COUNT_4_BIT или выше,
	* а потом разрешать результат в отдельный буфер.
	*/
	colourAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;				// Очищает буфер перед началом рендерингом.
	colourAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;			// После окончания рендеринга — результат сохраняется (чтобы показать на экране).
	colourAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;	// Не использует трафарет 
	colourAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // Не сохраняет трафрает

	// Framebuffer data will be stored as an image, but images can be given different data layouts
	// to give optimal use for certain operations in the pipeline
	colourAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; //  перед рендерингом изображение не имеет определёного формата
	colourAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // после рендеринга изображение будет готово к показу на экране

	// Attachment reference uses an attachment index that refers to index in the attachment list passed to renderPassCreateInfo
	VkAttachmentReference colourAttachmentReference = {}; // Создаётся объект, который будет использоваться внутри subpass'а и указывает, какой attachment и в каком layout использовать.
	colourAttachmentReference.attachment = 0; // Индекс attachment'а — это первый (и единственный) attachment в pAttachments.
	colourAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Layout, в котором attachment будет использоваться во время рендеринга. COLOR_ATTACHMENT_OPTIMAL — лучший вариант для записи в цветовой буфер.

	// Information about a particular subopass the Render Pass is using
	VkSubpassDescription subpass = {}; // Создаётся описание подпрохода (subpass), который будет использоваться в render pass.
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // Subpass будет использоваться в графическом пайплайне (не compute и не raytracing).
	subpass.colorAttachmentCount = 1; // У subpass'а будет один цветовой буфер — ссылка на colourAttachmentReference.
	subpass.pColorAttachments = &colourAttachmentReference;

	// Необходимо определить, когда происходят переходы между макетами, используя зависимости от subpass


	VkRenderPassCreateInfo renderPassCreateInfo = {}; // Создаётся структура, содержащая всю информацию для создания render pass.
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO; // Тип структуры Vulkan (обязательное поле для всех CreateInfo). 
	renderPassCreateInfo.attachmentCount = 1; // Указываем, что в этом render pass'е используется 1 attachment —  colourAttachment.
	renderPassCreateInfo.pAttachments = &colourAttachment;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpass;
}

void VulkanRenderer::createGraphicPipeline()
{
	// Read in SPIR-V code of shaders
	auto vertexShaderCode = readFile("Shaders/vert.spv");
	auto fragmentShaderCode = readFile("Shaders/frag.spv");

	// Build shader Moduls to link to Graphic Pipelines
	VkShaderModule vertexShaderModule = createShaderModule(vertexShaderCode);
	VkShaderModule fragmentShaderModule = createShaderModule(fragmentShaderCode);

	// -- SHADER STAGE CREATETION INFORMATION --
	// Vertex Stage creation information
	VkPipelineShaderStageCreateInfo vertexShaderCreateInfo = {};
	vertexShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;				// Shader stage name					
	vertexShaderCreateInfo.module = vertexShaderModule;						// Shader module to be used by stage
	vertexShaderCreateInfo.pName = "main";									// Entry point in to shader

	// Fragment Stage creation informatio
	VkPipelineShaderStageCreateInfo fragmentShaderCreateInfo = {};
	fragmentShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderCreateInfo.module = fragmentShaderModule;
	fragmentShaderCreateInfo.pName = "main";

	//	Put shader stage creation onfo in to array
	//	Graphic Pipeline creation info reaquires arrays of shader stage creates
	VkPipelineShaderStageCreateInfo shadersStagers[] = { vertexShaderCreateInfo, fragmentShaderCreateInfo };

	// -- VERTEX INPUT (TODO: Put in vertex descriptions when resources created)--
	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
	vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCreateInfo.vertexBindingDescriptionCount = 0;
	vertexInputCreateInfo.pVertexBindingDescriptions = nullptr;			// List of Vertex Binding Descriptions (data spacing/stride information)
	vertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
	vertexInputCreateInfo.pVertexAttributeDescriptions = nullptr;		// List of Vertex Attributre Descriptions (data format and where to bind to/from)

	// -- INPUT ASSEMBLY --
	VkPipelineInputAssemblyStateCreateInfo assemblyCreateInfo = {};
	assemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	assemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;	// Primitive typoe to assemble vertices
	assemblyCreateInfo.primitiveRestartEnable = VK_FALSE;				// Allow overriding of "strip" topology to start new primitives

	// -- VIEWPORT & SCISSOR --
	// Create a viewport info struct
	VkViewport viewport = {};							//
	viewport.x = 0.f;									//	x start coordinate
	viewport.y = 0.f;									//	y start coordinate
	viewport.width = (float)swapChainExtent.width;		//	width of viewport
	viewport.height = (float)swapChainExtent.height;	//	height of viewport
	viewport.minDepth = 0.f;							//	min framebuffer depth
	viewport.maxDepth = 1.f;							//	max framebuffer depth

	// Create a scissor info struct
	VkRect2D scissor = {};
	scissor.offset = { 0,0 };			// Offset to use region from
	scissor.extent = swapChainExtent;	// Extent to describe region to use, starting at offset
	
	VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
	viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateCreateInfo.viewportCount = 1;
	viewportStateCreateInfo.pViewports = &viewport;
	viewportStateCreateInfo.scissorCount = 1;
	viewportStateCreateInfo.pScissors = &scissor;

		/*
		*	// -- DYNAMIC STATES --
		*	// Dynamic states to enable
		*	std::vector<VkDynamicState> dynamicStateEnables;
		*	dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);	// Can resize in command buffer with vkCmdSetViewPort(commandbuffer, 0, 1, %viewport)
		*	dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);	// Can resize in command buffer with vkCmdSetScissor(commandbuffer, 0, 1, &scissor)
		*
		*	// Dynamic State creation info
		*	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
		*	dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		*	dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
		*	dynamicStateCreateInfo.pDynamicStates = dynamicStateEnables.data();
		*
		*/

	// -- RASTERIZER --
	VkPipelineRasterizationStateCreateInfo rasterizationCreateInfo = {};
	rasterizationCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationCreateInfo.depthClampEnable = VK_FALSE;			// Change if fragments beyod near/far planes are clipped (default) or clamped to plane
	rasterizationCreateInfo.rasterizerDiscardEnable = VK_FALSE;		// Wheter to discard data and skip rasterizer. Never creates fragments, only suitable for pipeline without framebuffer output
	rasterizationCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;		// How to handle filling points between vertices
	rasterizationCreateInfo.lineWidth = 1.f;						// How thick lines should be when drawn. If >1.f we must enable other extension
	rasterizationCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;		// Which face of a triangle to draw
	rasterizationCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;	// Winding to determine which side is front
	rasterizationCreateInfo.depthBiasEnable = VK_FALSE;				// Whether to add depth bias to fragments (good for stopping "shadow acne" in shadow mapping)

	// -- MULTISAMPLING --
	VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo = {};
	multisamplingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisamplingCreateInfo.sampleShadingEnable = VK_FALSE;					// Enable Multisample shading or not
	multisamplingCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;	// Number of samples to use per fragment

	// -- BLENDING --
	// Blending decides how to blend a new colour being written to a fragment, with the old value

	// Blend Attachment State(how blending is handled)
	VkPipelineColorBlendAttachmentState colorStateAttachments = {};
	colorStateAttachments.colorWriteMask = VK_COLOR_COMPONENT_R_BIT 
		| VK_COLOR_COMPONENT_G_BIT 
		| VK_COLOR_COMPONENT_B_BIT 
		| VK_COLOR_COMPONENT_A_BIT;
	colorStateAttachments.blendEnable = VK_TRUE;	// Colors to apply blending

	// Blending uses qeuation: (srcColorBlendFactor * new color) colorBlendOp (dstColorBlendFactor * old colour)
	colorStateAttachments.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorStateAttachments.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorStateAttachments.colorBlendOp = VK_BLEND_OP_ADD;


	// Summarised:	(VK_BLEND_FACTOR_SRC_APLHA * new colour) colorBlendOp (dstColorBlendFactor * old colour)
	//				(new colour aplha * new colour) + ((1 - new colour alpha) * old colour);

	colorStateAttachments.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorStateAttachments.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorStateAttachments.alphaBlendOp = VK_BLEND_OP_ADD;
	// Summarised:	(1 * new alpha) + (0 * old alpha) = new alpha

	VkPipelineColorBlendStateCreateInfo colorBlendinCraeteInfo = {};
	colorBlendinCraeteInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendinCraeteInfo.logicOpEnable = VK_FALSE;		// Alternative to calculation is to use logical operations
	colorBlendinCraeteInfo.attachmentCount = 1;
	colorBlendinCraeteInfo.pAttachments = &colorStateAttachments;
	//	colorBlendinCraeteInfo.logicOp = VK_LOGIC_OP_COPY;

	//	-- PIPELINE LAYOUT -- (TODO: Apply Future Description Set Layouts)
	VkPipelineLayoutCreateInfo piplineLayoutCreateInfo = {}; // Это нужно, чтобы поля, которые мы не заполним явно, имели нулевые значения (без мусора).
	piplineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO; // Vulkan требует, чтобы sType точно указывал тип структуры.
	piplineLayoutCreateInfo.setLayoutCount = 0; //  количество layout'ов (обычно для uniform buffer, textures и т.д.)
	piplineLayoutCreateInfo.pSetLayouts = nullptr; // указатель на массив layout'ов (если они есть)
	piplineLayoutCreateInfo.pPushConstantRanges = nullptr;
	/*
	* Push-константы — быстрый способ передавать мелкие данные (до 128 байт) напрямую в шейдер.
	* Например, матрицу mat4 или vec4 напрямую — нужно будет указать массив VkPushConstantRange.
	*/

	// Create Pipeline Layout (if you create - you must destroy)
	VkResult result = vkCreatePipelineLayout(mainDevice.logicalDevice, &piplineLayoutCreateInfo, nullptr, &pipelineLayout);
	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create Pipeline Layout!");

	// -- DEPTH STENCIL TESTING --
	// TODO: Set up depth stencil testing
	 
	// Destroy Shader Modules, no longer needed after Pipeline created
	vkDestroyShaderModule(mainDevice.logicalDevice, fragmentShaderModule, nullptr);
	vkDestroyShaderModule(mainDevice.logicalDevice, vertexShaderModule, nullptr);
}

void VulkanRenderer::getPhysicalDevice()
{
	// Enumerate Physical devices the vkInstance can access
	uint32_t physicalDeviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);

	// if 0 devices available, then none support Vulkan!
	if (physicalDeviceCount == 0) throw std::runtime_error("Can't find GPRU's that support Vulkan Instance!");

	// Get list of Physical devices
	std::vector<VkPhysicalDevice> deviceList(physicalDeviceCount);
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, deviceList.data());

	for (const auto &device : deviceList)
	{
		if (checkDeviceSuitable(device))
		{
			mainDevice.physicalDevice = device;
			break;
		}
	}

}

bool VulkanRenderer::checkInstanceExtensionSupport(std::vector<const char*>* checkExtensions)
{
	// Need to get number of extension to create array of correct size to hold extensions
	uint32_t extensionCount = 0; // создаем счётчик, что бы вывести кол-во расширений из этой переменной
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr); // получаем кол-во расширений из Vulkan функции

	// Create a list VkExtensionProperties using count
	std::vector<VkExtensionProperties> extensions(extensionCount); // создаем вектор/массив нужной длинны, а то есть то число, которе выдаст функий с кол-вом расширений
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data()); 

	// Check if given extensions are in list of available extensions
	for (const auto& checkExtension : *checkExtensions)
	{
		bool hasExtension = false;
		for (const auto& extension : extensions)
		{
			if (strcmp(checkExtension, extension.extensionName)) { hasExtension = true; break; }
		}
		if (!hasExtension) return false;
	}

	return true;
}

bool VulkanRenderer::checkDeviceExtentionSupport(VkPhysicalDevice device)
{
	// Get device extension count
	uint32_t extensionSupportCount = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionSupportCount, nullptr);
	
	if (extensionSupportCount == 0) return false;

	// Populate list of extensions
	std::vector<VkExtensionProperties> extensions(extensionSupportCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionSupportCount, extensions.data());

	// Check for extension
	for (const auto& deviceExtension : deviceExtensions)
	{
		bool hasExtension = false;
		for (const auto& extension : extensions)
		{
			if (strcmp(deviceExtension, extension.extensionName) == 0) { hasExtension = true; break; }
		}

		if (!hasExtension) return false;
	}
	return true;
}

bool VulkanRenderer::checkDeviceSuitable(VkPhysicalDevice device)
{
	/*	// Information about the device itself (ID, name, type, vender, etc)
	*	VkPhysicalDeviceProperties deviceProps;
	*	vkGetPhysicalDeviceProperties(device, &deviceProps);
	*
	*	// Info about what the device can do(geo shader, tess shader, wide lines, etc)
	*	VkPhysicalDeviceFeatures deviceFeatures;
	*	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
	*/

	QueueFamilyIndices indices = getQueueFamilies(device);

	bool extensionSupported = checkDeviceExtentionSupport(device);

	bool swapChainValid = false;
	if(extensionSupported)
	{
		SwapChainDetails swapChainDetails = getSwapchainDetails(device);
		swapChainValid = !swapChainDetails.presentationModes.empty() && !swapChainDetails.formats.empty();
	}

	return indices.isValid() && extensionSupported && swapChainValid;
}

QueueFamilyIndices VulkanRenderer::getQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	// Get all Queue Family Property info for the given device
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilyList(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyList.data());

	// Go through each queu family and chack i it has at least 1 of the required types of queue
	int i = 0;
	for (const auto& queueFamily : queueFamilyList)
	{
		// First check if queue family has at least 1 queue in that family ( could havce no queues)
		// Queu can be multiple types defined through bitfield. Need to bitwise AND with VK_QUEUE_*_BIT to check if has required
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i; // if queue family is valid, then get index
		}

		// Check if Queue Family supports presentation
		VkBool32 presentationSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface,&presentationSupport);
		// Check if queue is presentation type (can be both, graphic and presentation)
		if (queueFamily.queueCount > 0 && presentationSupport) indices.presentationFamily = i;

		// Check if queue family indices are in a valid state, stop searching if so.
		if (indices.isValid()) break;

		i++;
	}

	return indices;
}

SwapChainDetails VulkanRenderer::getSwapchainDetails(VkPhysicalDevice device)
{
	SwapChainDetails swapChainDetails;

	// -- CAPABILITIES --
	// Get the surface capabilities for the given surface on the given physical device
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &swapChainDetails.surfaceCapabilities);

	// -- FORMATS --
	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	// If formats returned, get list of formats
	if (formatCount != 0) 
	{
		swapChainDetails.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, swapChainDetails.formats.data());
	}

	// -- PRESENTATION MODES --
	uint32_t presentationCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentationCount, nullptr);
	// If presentModes returned, get list of presentModes
	if (presentationCount != 0)
	{
		swapChainDetails.presentationModes.resize(presentationCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentationCount, swapChainDetails.presentationModes.data());
	}

	return swapChainDetails;
}

// Best format is subjective, but our will be:
// Format		:	VK_FORMAT_R8G8B8A8_UNORM (VK_FORMAT_B8G8R8A8_UNORM as backup)
// colorSpace	:	VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
VkSurfaceFormatKHR VulkanRenderer::chooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &formats)
{
	// If only 1 format available and is undefined, then this means ALL formats ar available
	if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
	{
		return { VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	// If restricted, search for optimal format
	for (const auto &format : formats)
	{
		if ((format.format == VK_FORMAT_R8G8B8A8_UNORM || format.format == VK_FORMAT_B8G8R8A8_UNORM) 
			&& format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
		{
			return format;
		}
	}

	// If can't find optimal format, then just return first format
	return formats[0];
}

VkPresentModeKHR VulkanRenderer::chooseBestPresentationMode(const std::vector<VkPresentModeKHR> &presentationModes)
{
	// Look for Mailbox presentationMode
	for (const auto& presentationMode : presentationModes)
	{
		if (presentationMode == VK_PRESENT_MODE_MAILBOX_KHR) return presentationMode;
	}

	// If can't find, use FIFO as Vulkan spec says it must be present
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanRenderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities)
{
	// If current extent is at numeric limits, then extent can vary. Otherwise, it is the size of the window
	if (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) return surfaceCapabilities.currentExtent;
	else
	{
		// If value can vary, need to set manually

		// Get window size
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		// Create new extetn using window size
		VkExtent2D newExtent = {};
		newExtent.width = static_cast<uint32_t>(width);
		newExtent.height = static_cast<uint32_t>(height);

		// Surface also defines MAX and MIN, so make sure withtin bounderies by clamping value
		newExtent.width = std::max(surfaceCapabilities.minImageExtent.width, std::min(surfaceCapabilities.maxImageExtent.width, newExtent.width));
		newExtent.height = std::max(surfaceCapabilities.minImageExtent.height, std::min(surfaceCapabilities.maxImageExtent.height, newExtent.height));

		return newExtent;
	}
}

VkImageView VulkanRenderer::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
	VkImageViewCreateInfo viewCreateInfo = {};
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.image = image;										// Image to create view for
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;					// VK_IMAGE_VIEW_TYPE_2D = 1, Type of image (1D, 2D, 3D, Cube etc.)
	viewCreateInfo.format = format;										// Format of image date
	viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;		///////////////////////////////////////////////////////////////////////
	viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;		///		Allows remapping of rgba components to other rgba values	///
	viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;		///																	///
	viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;		///////////////////////////////////////////////////////////////////////

	// Subresources allow the view to view only a part of an image
	viewCreateInfo.subresourceRange.aspectMask = aspectFlags;			// Which aspect of image to view (e.g. COLOR_BIT for viewwing color)
	viewCreateInfo.subresourceRange.baseMipLevel = 0;					// Start mipmap level to view from
	viewCreateInfo.subresourceRange.levelCount = 1;						// Number of mipmap levels to view
	viewCreateInfo.subresourceRange.baseArrayLayer = 0;					// Start array level to view from
	viewCreateInfo.subresourceRange.layerCount = 1;						// Number of array levels to view

	// Create image view and return it
	VkImageView imageView;
	VkResult result = vkCreateImageView(mainDevice.logicalDevice, &viewCreateInfo, nullptr, &imageView);
	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create image!");

	return imageView;
}

VkShaderModule VulkanRenderer::createShaderModule(const std::vector<char>& code)
{
	// Shader Module create information
	VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.codeSize = code.size();										//	Size of code
	shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());		// Pointer to code (of uint32_t pointer type)

	VkShaderModule shaderModule;
	VkResult result = vkCreateShaderModule(mainDevice.logicalDevice, &shaderModuleCreateInfo, nullptr, &shaderModule);
	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create Shader Module!");

	return shaderModule;
}

