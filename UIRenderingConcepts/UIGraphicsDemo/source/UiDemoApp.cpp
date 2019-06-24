#include "UIDemoApp.h"
#include "UIDemo.h"
#include "UiMetalPaintEngine.h"
#include "Circle/Circle.h"
#include "Rectangle/Rect.h"
#include "../common/SceneGraph/Window.h"

#include <QApplication>

int main(int argc, char **argv)
{
    QApplication qtApp(argc, argv);

    UIDemoApp* app = new UIDemoApp();
    app->EnableDepthBuffer(true);
    app->EnableWindowResize(true);
    app->Initialize();
    app->ShowWindow();
    qtApp.exec();

    delete app;
    return 0;
}

UIDemoApp::UIDemoApp()
{
    VulkanHelper::GetInstanceLayerExtensionProperties();
}

UIDemoApp::~UIDemoApp()
{
    delete m_Scene;
//    delete m_ScenePainterEngine;
}

void UIDemoApp::Configure()
{
    SetApplicationName("Metal performance test");
    SetWindowDimension(1200 , 800);

#ifdef _WIN32
    // Add Validation Layers
    AddValidationLayer("VK_LAYER_LUNARG_standard_validation");

    // Add Vulkan instance extensions
    AddInstanceExtension(VK_KHR_SURFACE_EXTENSION_NAME);
    AddInstanceExtension(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
    AddInstanceExtension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
   #ifdef _WIN64
   #else
   #endif
#elif __APPLE__
    AddInstanceExtension("VK_KHR_surface");
    AddInstanceExtension("VK_MVK_macos_surface");
#endif

//    // m_SceneVector.push_back(std::make_shared<Scene>(this));
//    m_Scene = new Scene(this);//m_SceneVector[0].get();
//    m_ScenePainterEngine = new Scene(this);//m_SceneVector[0].get();

//    InitMetalEngine();
//     //BoundingRegion bgDim(10, 10, 400, 50);
//     //Node* background = new Rectangl(p_Scene, this, bgDim);
//    //Node* m_Parent = new Rectangl(m_Scene, NULL, BoundingRegion(), "Node 1", (false ? SHAPE::SHAPE_RECTANGLE_MULTIDRAW : SHAPE::SHAPE_RECTANGLE_INSTANCED));
//    m_Scene->SetEarlyDepthTest(true);
//    Node* m_Parent = new Rectangl(m_Scene, NULL, BoundingRegion(500, 300, 20, 20), "Node 1", SHAPE::SHAPE_RECTANGLE_INSTANCED);
//    m_Parent->SetColor(glm::vec4(1.0, 0.0, 1.0, 1.0));
//    m_Parent->SetDefaultColor(glm::vec4(1.0, 0.0, 0.0, 1.0));
//    m_Parent->SetMemPoolIdx(0);
//    m_Parent->SetZOrder(15);

////    Node* m_Parent1 = new Rectangl(m_Scene, NULL, BoundingRegion(150, 150, 200, 200), "Node 2", SHAPE::SHAPE_RECTANGLE_MULTIDRAW);
////    m_Parent1->SetColor(glm::vec4(0.0, 1.0, 0.0, 1.0));
////    m_Parent1->SetDefaultColor(glm::vec4(0.0, 1.0, 0.0, 1.0));
////    m_Parent1->SetMemPoolIdx(0);
////    m_Parent1->SetZOrder(20);

//    //m_UIDemo.Grid(m_Scene, m_WindowDimension.x, m_WindowDimension.y);            // Grid demo
//    //m_UIDemo.SliderFunc(m_Scene);                                                // Slider
//    //m_UIDemo.MixerView(m_Scene, m_WindowDimension.x, m_WindowDimension.y);        // Mixer View demo
//    m_UIDemo.TransformationConformTest(m_Scene);                           // Transformation test demo
}

void UIDemoApp::Setup()
{
    m_Scene = new Scene(this);
    m_Scene->SetSceneRect(0, 0, m_Window->width(), m_Window->height());
    m_ScenePainterEngine = new Scene(this);
    m_ScenePainterEngine->SetSceneRect(0, 0, m_Window->width(), m_Window->height());

    InitMetalEngine();
     //BoundingRegion bgDim(10, 10, 400, 50);
     //Node* background = new Rectangl(p_Scene, this, bgDim);
    //Node* m_Parent = new Rectangl(m_Scene, NULL, BoundingRegion(), "Node 1", (false ? SHAPE::SHAPE_RECTANGLE_MULTIDRAW : SHAPE::SHAPE_RECTANGLE_INSTANCED));
    m_Scene->SetEarlyDepthTest(true);
    Node* m_Parent = new Rectangl(m_Scene, NULL, BoundingRegion(0, 0, 20, 20), "Node 1", SHAPE::SHAPE_RECTANGLE_INSTANCED);
    m_Parent->SetColor(glm::vec4(1.0, 0.0, 1.0, 1.0));
    m_Parent->SetDefaultColor(glm::vec4(1.0, 0.0, 0.0, 1.0));
    m_Parent->SetMemPoolIdx(0);
    m_Parent->SetZOrder(15);

//    Node* m_Parent1 = new Rectangl(m_Scene, NULL, BoundingRegion(150, 150, 200, 200), "Node 2", SHAPE::SHAPE_RECTANGLE_MULTIDRAW);
//    m_Parent1->SetColor(glm::vec4(0.0, 1.0, 0.0, 1.0));
//    m_Parent1->SetDefaultColor(glm::vec4(0.0, 1.0, 0.0, 1.0));
//    m_Parent1->SetMemPoolIdx(0);
//    m_Parent1->SetZOrder(20);

    //m_UIDemo.Grid(m_Scene, m_WindowDimension.x, m_WindowDimension.y);            // Grid demo
    //m_UIDemo.SliderFunc(m_Scene);                                                // Slider
    m_UIDemo.MixerView(m_Scene, m_WindowDimension.x, m_WindowDimension.y);        // Mixer View demo
    //m_UIDemo.TransformationConformTest(m_Scene);                           // Transformation test demo

    m_Scene->Setup();
    m_ScenePainterEngine->Setup();

    RecordRenderPass(1, SG_STATE_SETUP); // Parminder: Double check if this is required

    // At least update the scene once so that in case UpdateMeAndMyChildren() is being used it has all transformation readily available
    m_Scene->Update();
    m_ScenePainterEngine->Update();
}

void UIDemoApp::Update()
{
    m_Scene->Update();
    m_ScenePainterEngine->Update();
}

bool UIDemoApp::Render()
{
    QRectF rect;
    m_MetalPaintEngine->drawRects(&rect, 1);

    RecordRenderPass(1, SG_STATE_RENDER);

    return VulkanApp::Render();
}

void UIDemoApp::MousePressEvent(QMouseEvent* p_Event)
{
    m_Scene->mousePressEvent(p_Event);
}

void UIDemoApp::MouseReleaseEvent(QMouseEvent* p_Event)
{
    m_Scene->mouseReleaseEvent(p_Event);
}

void UIDemoApp::MouseMoveEvent(QMouseEvent* p_Event)
{
    m_Scene->mouseMoveEvent(p_Event);
}

void UIDemoApp::ResizeWindow(int p_Width, int p_Height)
{
    VulkanApp::ResizeWindow(p_Width, p_Height);

    RecordRenderPass(3, SG_STATE_RESIZE, p_Width, p_Height);
}

bool UIDemoApp::InitMetalEngine()
{
    if (!m_MetalPaintEngine)
    {
        m_MetalPaintEngine.reset(new UiMetalPaintEngine());

        return m_MetalPaintEngine->Init(m_ScenePainterEngine);
    }

    return true;
}

void UIDemoApp::RecordRenderPass(int p_Argcount, ...)
{
//    va_list args;
//    va_start(args, b);
//    exampleV(b, args);
//    va_end(args);
//    va_list list;
//    va_start(list, p_Argcount);

//    m_Scene->RecordRenderPass(p_Argcount, list);
//    va_end(list);

//return;
    va_list list;
    va_start(list, p_Argcount);
    SCENE_GRAPH_STATES currentState = SG_STATE_NONE;
    QSize resizedDimension;

    for (int i = 0; i < p_Argcount; ++i)
    {
        switch (i)
        {
        case 0:
            currentState = static_cast<SCENE_GRAPH_STATES>(va_arg(list, int));
            break;

        case 1:
            resizedDimension.setWidth(va_arg(list, int));
            break;

        case 2:
            resizedDimension.setHeight(va_arg(list, int));
            break;

        default:
        {
            if (currentState == SG_STATE_NONE)
            {
                va_end(list);
                return;
            }
            break;
        }
        }
    }
    va_end(list);

    // Specify the clear color value
    VkClearValue clearColor[2];
    clearColor[0].color.float32[0] = 0.0f;
    clearColor[0].color.float32[1] = 0.0f;
    clearColor[0].color.float32[2] = 0.0f;
    clearColor[0].color.float32[3] = 0.0f;

    // Specify the depth/stencil clear value
    clearColor[1].depthStencil.depth = 1.0f;
    clearColor[1].depthStencil.stencil = 0;

    // Offset to render in the frame buffer
    VkOffset2D   renderOffset = { 0, 0 };
    // Width & Height to render in the frame buffer
    VkExtent2D   renderExtent = m_swapChainExtent;

    // For each command buffers in the command buffer list
    for (size_t i = 0; i < m_hCommandBufferList.size(); i++)
    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        // Indicate that the command buffer can be resubmitted to the queue
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

        // Begin command buffer
        vkBeginCommandBuffer(m_hCommandBufferList[i], &beginInfo);

        VkRenderPassBeginInfo renderPassInfo    = {};
        renderPassInfo.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass               = m_hRenderPass;
        renderPassInfo.framebuffer              = m_hFramebuffers[i];
        renderPassInfo.renderArea.offset        = renderOffset;
        renderPassInfo.renderArea.extent        = renderExtent;
        renderPassInfo.clearValueCount          = 2;
        renderPassInfo.pClearValues             = clearColor;

        // Begin render pass
        vkCmdBeginRenderPass(m_hCommandBufferList[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        switch (currentState)
        {
            case SG_STATE_SETUP:
            case SG_STATE_RENDER:
                m_Scene->Render(static_cast<void*>(m_hCommandBufferList[i]));
                m_ScenePainterEngine->Render(static_cast<void*>(m_hCommandBufferList[i]));
                break;

            case SG_STATE_RESIZE:
                m_Scene->Resize(resizedDimension.width(), resizedDimension.height());
                m_ScenePainterEngine->Resize(resizedDimension.width(), resizedDimension.height());
                break;

            default:
                break;
        }

        // End the Render pass
        vkCmdEndRenderPass(m_hCommandBufferList[i]);

        // End the Command buffer
        VkResult vkResult = vkEndCommandBuffer(m_hCommandBufferList[i]);
        if (vkResult != VK_SUCCESS)
        {
            VulkanHelper::LogError("vkEndCommandBuffer() failed!");
            assert(false);
        }
    }
}

//void UIDemoApp::RecordRenderPass(int p_Argcount, ...)
//{
//    va_list list;
//    va_start(list, p_Argcount);
//    SCENE_GRAPH_STATES currentState = SG_STATE_NONE;
//    QSize resizedDimension;

//    for (int i = 0; i < p_Argcount; ++i)
//    {
//        switch (i)
//        {
//        case 0:
//            currentState = static_cast<SCENE_GRAPH_STATES>(va_arg(list, int));
//            break;

//        case 1:
//            resizedDimension.setWidth(va_arg(list, int));
//            break;

//        case 2:
//            resizedDimension.setHeight(va_arg(list, int));
//            break;

//        default:
//        {
//            if (currentState == SG_STATE_NONE)
//            {
//                va_end(list);
//                return;
//            }
//            break;
//        }
//        }
//    }
//    va_end(list);

//    // Specify the clear color value
//    VkClearValue clearColor[2];
//    clearColor[0].color.float32[0] = 0.0f;
//    clearColor[0].color.float32[1] = 0.0f;
//    clearColor[0].color.float32[2] = 0.0f;
//    clearColor[0].color.float32[3] = 0.0f;

//    // Specify the depth/stencil clear value
//    clearColor[1].depthStencil.depth = 1.0f;
//    clearColor[1].depthStencil.stencil = 0;

//    // Offset to render in the frame buffer
//    VkOffset2D   renderOffset = { 0, 0 };
//    // Width & Height to render in the frame buffer
//    VkExtent2D   renderExtent = m_swapChainExtent;

//    // For each command buffers in the command buffer list
//    for (size_t i = 0; i < m_hCommandBufferList.size(); i++)
//    {
//        VkCommandBufferBeginInfo beginInfo = {};
//        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//        // Indicate that the command buffer can be resubmitted to the queue
//        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

//        // Begin command buffer
//        vkBeginCommandBuffer(m_hCommandBufferList[i], &beginInfo);

//        VkRenderPassBeginInfo renderPassInfo = {};
//        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
//        renderPassInfo.renderPass = m_hRenderPass;
//        renderPassInfo.framebuffer = m_hFramebuffers[i];
//        renderPassInfo.renderArea.offset = renderOffset;
//        renderPassInfo.renderArea.extent = renderExtent;
//        renderPassInfo.clearValueCount = 2;
//        renderPassInfo.pClearValues = clearColor;

//        // Begin render pass
//        vkCmdBeginRenderPass(m_hCommandBufferList[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

//        switch (currentState)
//        {
//        case SG_STATE_SETUP:
//        case SG_STATE_RENDER:
//            m_Scene->Render(m_hCommandBufferList[i]);
//            m_ScenePainterEngine->Render(m_hCommandBufferList[i]);
//            break;

//        case SG_STATE_RESIZE:
//            m_Scene->Resize(m_hCommandBufferList[i], resizedDimension.width(), resizedDimension.height());
//            m_ScenePainterEngine->Resize(m_hCommandBufferList[i], resizedDimension.width(), resizedDimension.height());
//            break;

//        default:
//            break;
//        }

//        // End the Render pass
//        vkCmdEndRenderPass(m_hCommandBufferList[i]);

//        // End the Command buffer
//        VkResult vkResult = vkEndCommandBuffer(m_hCommandBufferList[i]);
//        if (vkResult != VK_SUCCESS)
//        {
//            VulkanHelper::LogError("vkEndCommandBuffer() failed!");
//            assert(false);
//        }
//    }
//}
