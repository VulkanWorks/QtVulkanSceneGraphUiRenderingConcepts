#pragma once
#include "VulkanHelper.h"
#include "../common/SceneGraph/EngineInstance.h"

#include <memory>

#include <QElapsedTimer>
#include <QApplication>

/*********** GLM HEADER FILES ***********/
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

#define NUM_SAMPLES VK_SAMPLE_COUNT_1_BIT

class VulkanApp;
class Window;
//class DrawableInterface;

// Base class for Vulkan application
class VulkanApp : public BaseRenderer
{
public:
    VulkanApp();
    virtual ~VulkanApp();

    void Initialize(); // Initialize the Vulkan application
    virtual void Run();  // Render loop

    void AddValidationLayer(char* pName) { m_ValidationLayers.push_back(pName);  }
    void AddInstanceExtension(char* pName) { m_InstanceExtensionNames.push_back(pName); }

    virtual void CreateCommandBuffers(); // Overide the default implementation as per application requirement

    virtual void ResizeWindow(int p_Width, int p_Height);
//    virtual void MousePressEvent(QMouseEvent* p_Event) {} // Default implementation, Overide as required
//    virtual void MouseReleaseEvent(QMouseEvent* p_Event) {}
//    virtual void MouseMoveEvent(QMouseEvent* p_Event) {}

    void SetView(void *pView) {
      m_pView = pView;
    }

    void* GetWinID(WId p_WinID);
    void ShowWindow();

protected:
    // Core virtual methods used by derived classes
//    virtual void Configure();
//    virtual void Update();

    virtual bool Render();
    virtual bool Present();

private:
    // Initialization functions for Vulkan application
    void InitializeVulkan();
    void CreateVulkanInstance();
    void CreateSurface();

    // Device creation objects
    void CreateVulkanDeviceAndQueue();
        // Helper functions for CreateVulkanDeviceAndQueue()
        void SelectPhysicalDevice();
        void GetPhysicalDeviceInfo(VkPhysicalDevice device, PhysicalDeviceInfo* pDeviceInfo);
        bool IsDeviceSuitable(PhysicalDeviceInfo deviceInfo);
    void CreateDeviceAndQueueObjects();

    void CreateSwapChain();
    void CreateDepthImage();

    void CreateSemaphores();

    virtual void CreateRenderPass();
    void CreateFramebuffers();

public:
    struct {
        VkFormat        m_Format;
        VulkanImageView m_ImageView;
        VulkanImage     m_Image;
    }DepthImage;
    VkCommandBuffer     cmdBufferDepthImage; // Command buffer for depth image layout

    std::vector<const char *> m_InstanceExtensionNames;
    std::vector<const char *> m_ValidationLayers;

    // Vulkan specific objects
    VkInstance      m_hInstance; // Vulkan instance object    
    VkSurfaceKHR    m_hSurface;  // Vulkan presentation surface

    // Vulkan Device specific objects
    VkPhysicalDevice            m_hPhysicalDevice;
    PhysicalDeviceInfo          m_physicalDeviceInfo;
    VkDevice                    m_hDevice;
    std::vector<const char*>    m_requiredDeviceExtensionList;

    // Pointers to Graphics & Present queue
    VkQueue                     m_hGraphicsQueue;
    VkQueue                     m_hPresentQueue;

    // Swap chain specific objects
    VkSwapchainKHR              m_hSwapChain;
    VkFormat                    m_hSwapChainImageFormat;
    VkExtent2D                  m_swapChainExtent;
    std::vector<VkImageView>    m_hSwapChainImageViewList;
    uint32_t                    m_activeSwapChainImageIndex;

    // Render Pass
    VkRenderPass                m_hRenderPass;
    // Frame Buffer
    std::vector<VkFramebuffer>  m_hFramebuffers;

    // Command buffer related objects
    VkCommandPool                   m_hCommandPool;
    std::vector<VkCommandBuffer>    m_hCommandBufferList;

    // Presentation synchronization objects
    VkSemaphore                     m_hRenderReadySemaphore;
    VkSemaphore                     m_hPresentReadySemaphore;
    void* m_pView;
    QElapsedTimer FPS; int m_Frame;
};

//class DrawableInterface
//{
//public:
//    // Life Cycle
//    virtual void Setup() UNIMPLEMENTED_INTEFACE
//    virtual void Update() UNIMPLEMENTED_INTEFACE

//    // Transformation
//    void Rotate(float p_Angle, float p_X, float p_Y, float p_Z) { m_Model = glm::rotate(glm::mat4(), p_Angle, glm::vec3(p_X, p_Y, p_Z)); }
//    void Translate(float p_X, float p_Y, float p_Z) { m_Model = glm::translate(glm::mat4(), glm::vec3(p_X, p_Y, p_Z)); }
//    void Scale(float p_X, float p_Y, float p_Z) { m_Model = glm::translate(glm::mat4(), glm::vec3(p_X, p_Y, p_Z)); }
//    void Reset() { m_Model = glm::mat4(); }
//    GETSET(glm::mat4, Model)        // Owned by drawable item
//    GETSET(glm::mat4*, Projection)  // Not owned by drawable item
//    GETSET(glm::mat4*, View)        // Not owned by drawable item

//    // Mouse interaction: Dummy interface for now.
//    virtual void mousePressEvent() UNIMPLEMENTED_INTEFACE
//    virtual void mouseReleaseEvent() UNIMPLEMENTED_INTEFACE
//    virtual void mouseMoveEvent() UNIMPLEMENTED_INTEFACE
//    virtual void mouseDoubleClickEvent() UNIMPLEMENTED_INTEFACE

//    // Key interaction: Dummy interface for now.
//    virtual void keyPressEvent() UNIMPLEMENTED_INTEFACE

//    // Application Window resizing
//    virtual void ResizeWindow(int width, int height) {}
//};
