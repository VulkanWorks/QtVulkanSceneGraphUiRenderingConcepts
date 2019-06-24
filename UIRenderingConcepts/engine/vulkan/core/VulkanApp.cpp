#include "VulkanApp.h"
#include "../common/SceneGraph/Window.h"

//#include <QElapsedtimer>
#define FPS_TIME_START { \
                            /*static QElapsedTimer FPS; static int m_Frame;*/\
                            if (!FPS.isValid()) { FPS.start(); }

#define FPS_TIME_END        ++m_Frame; \
                            const int threshold = 3 * 1000; \
                            const int timing = FPS.elapsed(); \
                            if (timing > threshold) \
                            { \
                                const int fps = m_Frame / 3.0f; qDebug("FPS: %d", fps); \
                                m_Window->setTitle(QString("MacBook Air (13-inch, Early 2015), 1.6 GHz Intel Core i5, Intel HD Graphics 6000 1536 MB,       FPS: %1").arg(fps)); \
                                m_Frame = 0; \
                                FPS.start(); \
                            } \
                        }

#ifdef VK_USE_PLATFORM_MACOS_MVK
extern "C" {
void makeViewMetalCompatible(void* handle);
}
#endif

//Window::Window(VulkanApp* vulkanApp) : m_VulkanApp(vulkanApp)
//{
//    assert(vulkanApp);

//    VkExtent2D dimension = vulkanApp->GetWindowDimension();

//    setWidth(dimension.width);
//    setHeight(dimension.height);
//    setTitle(QString(vulkanApp->GetAppllicationName().c_str()));

//    renderTimer = new QTimer();
//    renderTimer->setInterval(1);

//    connect(renderTimer, SIGNAL(timeout()), this, SLOT(Run()));
//    renderTimer->start();
//}

//void Window::Run()
//{
//    m_VulkanApp->Run();
//}

//void Window::resizeEvent(QResizeEvent* pEvent)
//{
//    if (m_VulkanApp->m_ResizeEnabled == true &&
//        isVisible() == true)
//    {
//        int newWidth = width();
//        int newHeight = height();

//        m_VulkanApp->ResizeWindow(newWidth, newHeight);
//    }
//    QWindow::resizeEvent(pEvent);
//}

VulkanApp::VulkanApp()
{
    m_Window = nullptr;

    m_AppName = "Vulkan Application";    // Default application name

    SetWindowDimension(640, 480);    // Default application window dimension

    // Vulkan specific objects
    m_hInstance = VK_NULL_HANDLE;
    m_hSurface = VK_NULL_HANDLE;
    m_hPhysicalDevice = VK_NULL_HANDLE;
    m_hDevice = VK_NULL_HANDLE;
    m_hGraphicsQueue = VK_NULL_HANDLE;
    m_hPresentQueue = VK_NULL_HANDLE;
    m_hSwapChain = VK_NULL_HANDLE;
    m_hSwapChainImageFormat = VK_FORMAT_UNDEFINED;
    m_hRenderPass = VK_NULL_HANDLE;
    m_hCommandPool = VK_NULL_HANDLE;
    m_hRenderReadySemaphore = VK_NULL_HANDLE;
    m_hPresentReadySemaphore = VK_NULL_HANDLE;

    memset(&m_swapChainExtent, 0, sizeof(m_swapChainExtent));

    m_hSwapChainImageViewList.clear();
    m_hFramebuffers.clear();
    m_hCommandBufferList.clear();
    
    m_activeSwapChainImageIndex = 0;

    // VK_KHR_SWAPCHAIN_EXTENSION_NAME extension support is required to
    // use the selected vulkan device to present rendered output to a window
    m_requiredDeviceExtensionList.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    memset(&DepthImage, 0, sizeof(DepthImage));
    m_DepthEnabled = false;
    m_ResizeEnabled = false;
}

VulkanApp::~VulkanApp()
{ 
    vkDestroySemaphore(m_hDevice, m_hPresentReadySemaphore, nullptr);
    vkDestroySemaphore(m_hDevice, m_hRenderReadySemaphore, nullptr);
    vkDestroyCommandPool(m_hDevice, m_hCommandPool, nullptr);

    for (size_t i = 0; i < m_hFramebuffers.size(); i++)
    {
        vkDestroyFramebuffer(m_hDevice, m_hFramebuffers[i], nullptr);
    }

    vkDestroyRenderPass(m_hDevice, m_hRenderPass, nullptr);

    // Release Color attachment resources
    for (size_t i = 0; i < m_hSwapChainImageViewList.size(); i++)
    {
        vkDestroyImageView(m_hDevice, m_hSwapChainImageViewList[i], nullptr);
    }

    if (m_DepthEnabled)
    {
        // Release Depth attachment resources - image, image view and allocated device memory
        vkDestroyImage(m_hDevice, DepthImage.m_Image.out.image, nullptr);
        vkDestroyImageView(m_hDevice, DepthImage.m_ImageView.imageView, nullptr);
        vkFreeMemory(m_hDevice, DepthImage.m_Image.out.deviceMemory, nullptr);
    }

    vkDestroySwapchainKHR(m_hDevice, m_hSwapChain, nullptr);
    vkDestroyDevice(m_hDevice, nullptr);
    vkDestroySurfaceKHR(m_hInstance, m_hSurface, nullptr);
    vkDestroyInstance(m_hInstance, nullptr);
}

void VulkanApp::Initialize()
{
    Configure();
    InitializeVulkan();     // Initialize Vulkan
    Setup();                // Setup applications specific objects
}

void VulkanApp::Run()
{
    FPS_TIME_START
    Update(); // For derive implementation to update data
    Render(); // Render application specific data
    Present(); // Display the drawing output on presentation window
    FPS_TIME_END
}

//void VulkanApp::SetWindowDimension(int p_Width, int p_Height)
//{
//    m_WindowDimension.width = p_Width;
//    m_WindowDimension.height = p_Height;
//}

void VulkanApp::InitializeVulkan()
{
    CreateVulkanInstance();         // Create Vulkan Instance
    CreateVulkanDeviceAndQueue();   // Create Vulkan Device and Queue
    CreateSwapChain();              // Create Swap chain

    CreateCommandBuffers();
    if (m_DepthEnabled)
    {
        CreateDepthImage();         // Create the depth image
    }

    CreateRenderPass();             // Create Render Pass
    CreateFramebuffers();           // Create Frame buffer
    CreateSemaphores();             // Create semaphores for rendering synchronization between window system and presentaion rate
}

void VulkanApp::CreateVulkanInstance()
{
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = NULL;
    appInfo.pApplicationName = m_AppName.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "VulkanApp";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    // Fill in the required createInfo structure
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
    createInfo.ppEnabledLayerNames = m_ValidationLayers.data();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(m_InstanceExtensionNames.size());
    createInfo.ppEnabledExtensionNames = m_InstanceExtensionNames.data();

    // Create the Vulkan Instance
    VkResult result = vkCreateInstance(&createInfo, nullptr, &m_hInstance);
	if (result != VK_SUCCESS)
	{
		VulkanHelper::LogError("Error creating Vulkan Instance");
	}

	CreateSurface();

	assert (result == VK_SUCCESS);
}

void VulkanApp::CreateSurface()
{
    m_Window = new Window(this); // Create the window to provide display housing
    #ifdef _WIN32
    m_Window->show(); // This is fatal, on MacOS do not show the window as is.
    #endif // Remove this show() once fixed for all example call show from the app itself.
    m_pView = GetWinID(m_Window->winId());

    VkResult  result;
    // Create the Vulkan surface for the application window
#ifdef _WIN32
	VkWin32SurfaceCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext = NULL;
	createInfo.hinstance = GetModuleHandle(nullptr);
	createInfo.hwnd = HWND(m_Window->winId());

	result = vkCreateWin32SurfaceKHR(m_hInstance, &createInfo, NULL, &m_hSurface);
#elif __APPLE__
    VkMacOSSurfaceCreateInfoMVK createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.pView = m_pView;

    result = vkCreateMacOSSurfaceMVK(m_hInstance, &createInfo, nullptr, &m_hSurface);
#endif

	if (result != VK_SUCCESS)
	{
		VulkanHelper::LogError("Error: Unable to create the window surface.");
	}

	assert(result == VK_SUCCESS);
}

void VulkanApp::CreateVulkanDeviceAndQueue()
{
	// Select the physical device(m_hPhysicalDevice) suitable for Vulkan
    SelectPhysicalDevice();

	// Query layer and extension for m_hPhysicalDevice
	VulkanHelper::GetDeviceLayerExtensionProperties(m_hPhysicalDevice);

	// Create the Vulkan device & get pointer to graphics & present queue
    CreateDeviceAndQueueObjects();
}

// Returns detailed information for the given physical device handle
void VulkanApp::GetPhysicalDeviceInfo(VkPhysicalDevice device, PhysicalDeviceInfo* pDeviceInfo)
{
    vkGetPhysicalDeviceProperties(device, &pDeviceInfo->prop);
    vkGetPhysicalDeviceFeatures(device, &pDeviceInfo->features);
    vkGetPhysicalDeviceMemoryProperties(device, &pDeviceInfo->memProp);

    // Get count of queue family
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    pDeviceInfo->familyPropList.resize(queueFamilyCount);

    // Get the list of queue family properties
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, pDeviceInfo->familyPropList.data());

    // Get the family queue index for graphics & present
    pDeviceInfo->graphicsFamilyIndex = -1;
    pDeviceInfo->presentFamilyIndex = -1;
    // For each queue family property in the list
    for (unsigned int i = 0; i<pDeviceInfo->familyPropList.size(); i++)
    {
        if (pDeviceInfo->familyPropList[i].queueCount > 0)
        {
            // Check if it supports graphics queue
            if (pDeviceInfo->familyPropList[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                // Device queue supports graphics queue
                // Store the graphics queue index
                pDeviceInfo->graphicsFamilyIndex = i;
            }


            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_hSurface, &presentSupport);

            // Check if it supports present queue
            if (presentSupport)
            {
                // Device queue family supports present
                // Store the present queue index
                pDeviceInfo->presentFamilyIndex = i;
            }

            // Check if device queue family supports both 
            // graphics & present queue
            if (pDeviceInfo->graphicsFamilyIndex >= 0 &&
                pDeviceInfo->presentFamilyIndex >= 0)
            {
                break;
            }
        }
    }

    // Get device extension count
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    pDeviceInfo->extensionList.resize(extensionCount);
    // Get device extensions
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, pDeviceInfo->extensionList.data());

    // Get surface capabilities of the given physical device
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_hSurface, &pDeviceInfo->capabilities);

    // Get the surface format count
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_hSurface, &formatCount, nullptr);

    if (formatCount != 0)
    {
        pDeviceInfo->formatList.resize(formatCount);
        // Get the surface formats
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_hSurface, &formatCount, pDeviceInfo->formatList.data());
    }

    // Get the surface present mode count
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_hSurface, &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        pDeviceInfo->presentModeList.resize(presentModeCount);
        // Get the surface present modes
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_hSurface, &presentModeCount, pDeviceInfo->presentModeList.data());
    }
}


// This method checks if the given device 
// a) Contains graphics and present queue families and
// b) Supports the extensions required and
// c) Supports swap chain requirements
// The method returns true when all 3 conditions are met
// or false otherwise
bool VulkanApp::IsDeviceSuitable(PhysicalDeviceInfo deviceInfo)
{
    bool result = false;

    // Step 1: Check if graphics & present queue families are
    // are supported
    if (deviceInfo.graphicsFamilyIndex >= 0 &&
        deviceInfo.presentFamilyIndex >= 0)
    {
        // Step 2: Check if required device extensions are supported
        std::set<std::string> requiredExtensions(m_requiredDeviceExtensionList.begin(), m_requiredDeviceExtensionList.end());

        uint32_t deviceExtensionCount = static_cast<uint32_t>(deviceInfo.extensionList.size());

        for (uint32_t i = 0; i < deviceExtensionCount && !result; i++)
        {
            requiredExtensions.erase(deviceInfo.extensionList[i].extensionName);
            // Exit loop when all required extensions are found
            result = requiredExtensions.empty();
        }

        if (result)
        {
            // Step 3: Check if swap chain requirements are met
            result = (!deviceInfo.formatList.empty()) && (!deviceInfo.presentModeList.empty());
        }
    }

    return (result);
}

// This method finds the appropriate physical device (GPU)
// suitable for rendering using Vulkan. The method returns 
// true on success, false otherwise
void VulkanApp::SelectPhysicalDevice()
{
    bool result = true;
    uint32_t deviceCount = 0;

    // Get the number of GPUs that supports Vulkan
    vkEnumeratePhysicalDevices(m_hInstance, &deviceCount, nullptr);

    if (deviceCount >= 0)
    {
        std::vector<VkPhysicalDevice> deviceList(deviceCount);

        // Get the GPU devices that are accessible to a Vulkan
        vkEnumeratePhysicalDevices(m_hInstance, &deviceCount, deviceList.data());

        // Select the device that is suitable for rendering
        for (uint32_t i = 0; i<deviceCount; i++)
        {
            // Get all required device information to select the device
            PhysicalDeviceInfo deviceInfo = {};
            GetPhysicalDeviceInfo(deviceList[i], &deviceInfo);

            // Check if this device meets our application criteria
            result = IsDeviceSuitable(deviceInfo);

            if (result)
            {
                // Store the pointer to the selected physical device (GPU)
                m_hPhysicalDevice = deviceList[i];
                m_physicalDeviceInfo = deviceInfo;
                break;
            }
        }

        if (m_hPhysicalDevice == VK_NULL_HANDLE)
        {
			VulkanHelper::LogError("Could not find a suitable GPU");
            result = false;
        }
    }
    else
    {
		VulkanHelper::LogError("Could not find a GPU that supports Vulkan");
        result = false;
    }

    assert (result);
}

void VulkanApp::CreateDeviceAndQueueObjects()
{
    bool result = true;

    // Fill-in the queue info structures. Note that when the graphics family queue index
    // is different from the present family index we need to pass in 2 seperate
    // queue create info structures
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<int> uniqueQueueFamilies = { m_physicalDeviceInfo.graphicsFamilyIndex,
        m_physicalDeviceInfo.presentFamilyIndex };

    float queuePriority[] = { 1.0f };
    for (std::set<int>::iterator it = uniqueQueueFamilies.begin(); it != uniqueQueueFamilies.end(); ++it)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = *it;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // Now fill-in the VkDeviceCreateInfo structure
    VkPhysicalDeviceFeatures deviceFeatures = {};
    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(m_requiredDeviceExtensionList.size());
    createInfo.ppEnabledExtensionNames = m_requiredDeviceExtensionList.data();
    createInfo.enabledLayerCount = 0;

    // Create the Vulkan device
    if (vkCreateDevice(m_hPhysicalDevice, &createInfo, nullptr, &m_hDevice) == VK_SUCCESS)
    {
        // Get the pointer to the graphics queue and present queue
        // This is used to submit commands to the GPU
        vkGetDeviceQueue(m_hDevice, m_physicalDeviceInfo.graphicsFamilyIndex, 0, &m_hGraphicsQueue);
        vkGetDeviceQueue(m_hDevice, m_physicalDeviceInfo.presentFamilyIndex, 0, &m_hPresentQueue);
    }
    else
    {
		VulkanHelper::LogError("Vulkan Device creation failed");
        result = false;
    }

    assert (result);
}

void VulkanApp::CreateSwapChain()
{
    VkSwapchainKHR oldSwapChain = m_hSwapChain;

    VkSurfaceFormatKHR surfaceFormat = VulkanHelper::SelectBestSurfaceFormat(m_physicalDeviceInfo.formatList);
    VkPresentModeKHR presentMode = VulkanHelper::SelectBestPresentMode(m_physicalDeviceInfo.presentModeList);
    VkExtent2D windowDim = VkExtent2D { m_WindowDimension.x, m_WindowDimension.y };
    VkExtent2D extent = VulkanHelper::SelectBestExtent(m_physicalDeviceInfo.capabilities, windowDim);
    uint32_t imageCount = m_physicalDeviceInfo.capabilities.minImageCount + 1;

    // Validate the image count bounds
    if (m_physicalDeviceInfo.capabilities.maxImageCount > 0 &&
        imageCount > m_physicalDeviceInfo.capabilities.maxImageCount)
    {
        imageCount = m_physicalDeviceInfo.capabilities.maxImageCount;
    }

    // Fill in VkSwapchainCreateInfoKHR to create the swap chain
    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_hSurface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queueFamilyIndices[] = { (uint32_t)m_physicalDeviceInfo.graphicsFamilyIndex, (uint32_t)m_physicalDeviceInfo.presentFamilyIndex };

    if (m_physicalDeviceInfo.graphicsFamilyIndex != m_physicalDeviceInfo.presentFamilyIndex)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = m_physicalDeviceInfo.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = oldSwapChain;

    // Create the Swap chain
    if (vkCreateSwapchainKHR(m_hDevice, &createInfo, nullptr, &m_hSwapChain) != VK_SUCCESS)
    {
		VulkanHelper::LogError("vkCreateSwapchainKHR() failed!");
        assert(false);
    }
    else
    {
        if (oldSwapChain != VK_NULL_HANDLE)
        {
            for (uint32_t i = 0; i < imageCount; i++)
            {
                vkDestroyImageView(m_hDevice, m_hSwapChainImageViewList[i], nullptr);
            }
            m_hSwapChainImageViewList.clear();
            vkDestroySwapchainKHR(m_hDevice, oldSwapChain, nullptr);
        }

        // Get the count of swap chain images
        vkGetSwapchainImagesKHR(m_hDevice, m_hSwapChain, &imageCount, nullptr);
		std::vector<VkImage> swapChainImageList;
		swapChainImageList.resize(imageCount);
        // Get the swap chain images
        vkGetSwapchainImagesKHR(m_hDevice, m_hSwapChain, &imageCount, swapChainImageList.data());

        m_hSwapChainImageFormat = surfaceFormat.format;
        m_swapChainExtent = extent;

        m_hSwapChainImageViewList.resize(swapChainImageList.size());

        // For every image in the swap chain
        for (size_t i = 0; i < swapChainImageList.size(); i++)
        {
            // Fill in VkImageViewCreateInfo to create the image view
            VkImageViewCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapChainImageList[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = m_hSwapChainImageFormat;
			createInfo.components = VkComponentMapping{};
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            // Create the swap chain image view
            if (vkCreateImageView(m_hDevice, &createInfo, nullptr, &m_hSwapChainImageViewList[i]) != VK_SUCCESS)
            {
				VulkanHelper::LogError("vkCreateImageView() failed!");
				assert(false);
			}
        }
    }
}

void VulkanApp::CreateDepthImage()
{
    DepthImage.m_Format = VK_FORMAT_D16_UNORM;
    DepthImage.m_Image.in.extent = { static_cast<uint32_t>(m_Window->width()), static_cast<uint32_t>(m_Window->height()), 1 };

    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.pNext = NULL;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = DepthImage.m_Format;
    imageInfo.extent = DepthImage.m_Image.in.extent;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = NUM_SAMPLES;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.queueFamilyIndexCount = 0;
    imageInfo.pQueueFamilyIndices = NULL;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.flags = 0;

    VulkanHelper::CreateImage(m_hDevice, m_physicalDeviceInfo.memProp, DepthImage.m_Image, &imageInfo);

    VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    if (DepthImage.m_Format == VK_FORMAT_D16_UNORM_S8_UINT ||
        DepthImage.m_Format == VK_FORMAT_D24_UNORM_S8_UINT ||
        DepthImage.m_Format == VK_FORMAT_D32_SFLOAT_S8_UINT) {
        aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    // Set image layout for depth stencil image
    if (!m_hCommandPool) { VulkanHelper::CreateCommandPool(m_hDevice, m_hCommandPool, m_physicalDeviceInfo); }

    VulkanHelper::AllocateCommandBuffer(m_hDevice, m_hCommandPool, &cmdBufferDepthImage);
    VulkanHelper::BeginCommandBuffer(cmdBufferDepthImage);

    VulkanHelper::SetImageLayout(DepthImage.m_Image.out.image, aspectMask,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, (VkAccessFlagBits)0, cmdBufferDepthImage);

    VulkanHelper::EndCommandBuffer(cmdBufferDepthImage);
    VulkanHelper::SubmitCommandBuffer(m_hGraphicsQueue, cmdBufferDepthImage);

    // Create the image view and allow the application to use the images.
    VkImageViewCreateInfo imgViewInfo = {};
    imgViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imgViewInfo.pNext = NULL;
    imgViewInfo.image = DepthImage.m_Image.out.image;
    imgViewInfo.format = VK_FORMAT_D16_UNORM;
    imgViewInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY };
    imgViewInfo.subresourceRange.aspectMask = aspectMask;
    imgViewInfo.subresourceRange.baseMipLevel = 0;
    imgViewInfo.subresourceRange.levelCount = 1;
    imgViewInfo.subresourceRange.baseArrayLayer = 0;
    imgViewInfo.subresourceRange.layerCount = 1;
    imgViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imgViewInfo.flags = 0;

    VulkanHelper::CreateImageView(m_hDevice, DepthImage.m_ImageView, &imgViewInfo);
}

void VulkanApp::CreateRenderPass()
{
    // Fill in the color attachment

    VkAttachmentDescription attachments[2] = {};
    attachments[0].format = m_hSwapChainImageFormat;
    attachments[0].samples = NUM_SAMPLES;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    if (m_DepthEnabled)
    {
        attachments[1].format = DepthImage.m_Format;
        attachments[1].samples = NUM_SAMPLES;
        attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        attachments[1].flags = VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT;
    }

    VkAttachmentReference attachmentRef[2] = {};
    attachmentRef[0] = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };          // Color attachment
    if (m_DepthEnabled)
    {
        attachmentRef[1] = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };  // Depth attachment
    }

    // Fill in the sub pass
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attachmentRef[0];
    if (m_DepthEnabled)
    {
        subpass.pDepthStencilAttachment = &attachmentRef[1];
    }

    // Fill in the sub pass dependency
    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // Now fill in the render pass info with all the above details
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = (m_DepthEnabled ? 2 : 1);
    renderPassInfo.pAttachments = attachments;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    // Create the render pass
    if (vkCreateRenderPass(m_hDevice, &renderPassInfo, nullptr, &m_hRenderPass) != VK_SUCCESS)
    {
        VulkanHelper::LogError("vkCreateRenderPass() failed to create render pass!");
        assert(false);
    }
}

void VulkanApp::CreateFramebuffers()
{
	// Resize the list based on swap chain image view count
	m_hFramebuffers.resize(m_hSwapChainImageViewList.size());

	VkImageView attachments[2];
	if (m_DepthEnabled)
	{
		attachments[1] = DepthImage.m_ImageView.imageView;
	}

	// Setup VkFramebufferCreateInfo to create frame buffer object
	VkFramebufferCreateInfo framebufferInfo = {};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = m_hRenderPass;
	framebufferInfo.attachmentCount = (m_DepthEnabled ? 2 : 1);
	framebufferInfo.pAttachments = attachments;
	framebufferInfo.width = m_swapChainExtent.width;
	framebufferInfo.height = m_swapChainExtent.height;
	framebufferInfo.layers = 1;

    // For each item in swap chain image view list
    for (size_t i = 0; i < m_hSwapChainImageViewList.size(); i++)
    {
		attachments[0] = m_hSwapChainImageViewList[i];

		// Create frame buffer object
        VkResult vkResult = vkCreateFramebuffer(m_hDevice, &framebufferInfo, nullptr, &m_hFramebuffers[i]);
        if (vkResult != VK_SUCCESS)
        {
			VulkanHelper::LogError("vkCreateFramebuffer() failed!");
			assert(false);
        }
    }
}

void VulkanApp::CreateCommandBuffers()
{
    // Create the command buffer pool object
	if (!m_hCommandPool)
	{
        // Note: We are overidding the default Create Command pool with VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
        // because we need to re-record the command buffer when the instance data size changes. 
        // This need to recreate the command buffer. 
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = m_physicalDeviceInfo.graphicsFamilyIndex;
        VulkanHelper::CreateCommandPool(m_hDevice, m_hCommandPool, m_physicalDeviceInfo, &poolInfo);
    }

    VkResult vkResult;
    if (m_hCommandPool)
    {
        m_hCommandBufferList.resize(m_hSwapChainImageViewList.size());

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_hCommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)m_hCommandBufferList.size();

        vkResult = vkAllocateCommandBuffers(m_hDevice, &allocInfo, m_hCommandBufferList.data());
        if (vkResult != VK_SUCCESS)
        {
            VulkanHelper::LogError("vkAllocateCommandBuffers() failed!");
            assert(false);
        }
    }
    else
    {
		VulkanHelper::LogError("vkCreateCommandPool() failed!");
		assert(false);
	}
}

void VulkanApp::CreateSemaphores()
{
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreInfo.flags = 0;

    // Create 2 semaphore objects
    // One to signal when the image is ready to render
    // Another one to signal when the image is ready to present
    if (vkCreateSemaphore(m_hDevice, &semaphoreInfo, nullptr, &m_hRenderReadySemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(m_hDevice, &semaphoreInfo, nullptr, &m_hPresentReadySemaphore) != VK_SUCCESS)
    {
        VulkanHelper::LogError("vkCreateSemaphore() failed");
        assert(false);
    }
}

void VulkanApp::ResizeWindow(int p_Width, int p_Height)
{
    SetWindowDimension(p_Width, p_Height);

    vkDeviceWaitIdle(m_hDevice);

    // Destroy and create new swap chain
    CreateSwapChain();

    // Destroy and create new depth buffer
    if (m_DepthEnabled)
    {
        // Release Depth attachment resources - image, image view and allocated device memory
        vkDestroyImage(m_hDevice, DepthImage.m_Image.out.image, nullptr);
        vkDestroyImageView(m_hDevice, DepthImage.m_ImageView.imageView, nullptr);
        vkFreeMemory(m_hDevice, DepthImage.m_Image.out.deviceMemory, nullptr);

        CreateDepthImage();
    }

    // Destroy and create new frame buffers
    for (size_t i = 0; i < m_hFramebuffers.size(); i++)
    {
        vkDestroyFramebuffer(m_hDevice, m_hFramebuffers[i], nullptr);
        m_hFramebuffers[i] = nullptr;
    }
    m_hFramebuffers.clear();
    CreateFramebuffers();

    // Parminder: It seems there is no need to destroy the command buffer. Just record them with new render pass.
    //// Destroy and create new command buffers
    //vkFreeCommandBuffers(m_hDevice, m_hCommandPool, (uint32_t)m_hCommandBufferList.size(), m_hCommandBufferList.data());
    //m_hCommandBufferList.clear();

    m_activeSwapChainImageIndex = 0;

    vkDeviceWaitIdle(m_hDevice);
}

void* VulkanApp::GetWinID(WId p_WinID)
{
    void* viewId = NULL;

#ifdef VK_USE_PLATFORM_MACOS_MVK
    makeViewMetalCompatible(reinterpret_cast<void*>(p_WinID));
    viewId = reinterpret_cast<void*>(p_WinID);
#elif VK_USE_PLATFORM_WIN32_KHR
    viewId = reinterpret_cast<HWND>(p_WinID);
#endif

    return viewId;
}

void VulkanApp::ShowWindow()
{
    if (!m_Window) return;

    m_Window->show();
}

bool VulkanApp::Render()
{
    m_activeSwapChainImageIndex = 0;

    const uint64_t timeOut = 999;
    vkAcquireNextImageKHR(m_hDevice, m_hSwapChain, timeOut, m_hRenderReadySemaphore, VK_NULL_HANDLE, &m_activeSwapChainImageIndex);

    VkPipelineStageFlags waitDstStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    // Setup VkSubmitInfo to submit graphics queue workload to GPU
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &m_hRenderReadySemaphore;
    submitInfo.pWaitDstStageMask = waitDstStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_hCommandBufferList[m_activeSwapChainImageIndex];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &m_hPresentReadySemaphore;

    VkResult vkResult = vkQueueSubmit(m_hGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);

    assert (vkResult == VK_SUCCESS);
    return VK_SUCCESS;
}

bool VulkanApp::Present()
{
    bool result;

    // Setup VkPresentInfoKHR to execute the present queue
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &m_hPresentReadySemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_hSwapChain;
    presentInfo.pImageIndices = &m_activeSwapChainImageIndex;

    result = vkQueuePresentKHR(m_hPresentQueue, &presentInfo);

    if (result == VK_SUCCESS)
    {
        // Wait for present queue to become idle
        result = vkQueueWaitIdle(m_hPresentQueue);
    }

    return (result == VK_SUCCESS);
}
