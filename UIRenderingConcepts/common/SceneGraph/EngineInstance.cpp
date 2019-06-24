#include "EngineInstance.h"

BaseRenderer::BaseRenderer()
{
}

BaseRenderer::~BaseRenderer()
{
}

void BaseRenderer::SetApplicationName(std::string p_AppName)
{
    m_AppName = p_AppName;
}

std::string BaseRenderer::GetAppllicationName()
{
    return (m_AppName);
}

void BaseRenderer::SetWindowDimension(int p_Width, int p_Height)
{
    m_WindowDimension.x = p_Width;
    m_WindowDimension.y = p_Height;
}

glm::u32vec2 BaseRenderer::GetWindowDimension()
{
    return m_WindowDimension;
}

void BaseRenderer::EnableDepthBuffer(bool p_DepthEnabled)
{
    m_DepthEnabled = p_DepthEnabled;
}

bool BaseRenderer::IsDepthBufferEnabled()
{
    return m_DepthEnabled;
}

void BaseRenderer::EnableWindowResize(bool p_ResizeEnabled)
{
    m_ResizeEnabled = p_ResizeEnabled;
}

bool BaseRenderer::IsWindowResizeEnabled()
{
    return m_ResizeEnabled;
}
