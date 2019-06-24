#include "Window.h"

#include <assert.h>

//#include "../engine/vulkan/core/VulkanApp.h"
#include "../common/SceneGraph/EngineInstance.h"

#ifdef VK_USE_PLATFORM_MACOS_MVK
extern "C" {
void makeViewMetalCompatible(void* handle);
}
#endif

Window::Window(BaseRenderer* vulkanApp) : m_VulkanApp(vulkanApp)
{
    assert(vulkanApp);

    glm::u32vec2 dimension = vulkanApp->GetWindowDimension();

    setWidth(dimension.x);
    setHeight(dimension.y);
    setTitle(QString(vulkanApp->GetAppllicationName().c_str()));

    renderTimer = new QTimer();
    renderTimer->setInterval(1);

    connect(renderTimer, SIGNAL(timeout()), this, SLOT(Run()));
    renderTimer->start();
}

void Window::Run()
{
    m_VulkanApp->Run();
}

void Window::resizeEvent(QResizeEvent* pEvent)
{
    if (m_VulkanApp->IsDepthBufferEnabled() == true &&
        isVisible() == true)
    {
        int newWidth = width();
        int newHeight = height();

        m_VulkanApp->ResizeWindow(newWidth, newHeight);
    }

    QWindow::resizeEvent(pEvent);
}

void Window::mousePressEvent(QMouseEvent *p_Event)
{
    // Graphics engine Mouse Press Event entry point
    const bool oldIsAcceptedState = p_Event->isAccepted();
    // TODO: Do we need to store the old p_Event accepted state

    p_Event->ignore();

    m_VulkanApp->MousePressEvent(p_Event);
}

void Window::mouseReleaseEvent(QMouseEvent *p_Event)
{
    p_Event->ignore();

    m_VulkanApp->MouseReleaseEvent(p_Event);
}

void Window::mouseMoveEvent(QMouseEvent *p_Event)
{
    p_Event->ignore();

    m_VulkanApp->MouseMoveEvent(p_Event);
}
