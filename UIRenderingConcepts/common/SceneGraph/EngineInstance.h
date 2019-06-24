#pragma once

#include <string>

#include <QMouseEvent>

/*********** GLM HEADER FILES ***********/
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"

class Window;
class BaseRenderer
{
public:
    BaseRenderer();
    virtual ~BaseRenderer();

    virtual void ShowWindow() = 0;

    void SetApplicationName(std::string p_AppName);
    std::string GetAppllicationName();
    void SetWindowDimension(int p_Width, int p_Height);
    glm::u32vec2 GetWindowDimension();

    void EnableDepthBuffer(bool p_DepthEnabled);
    bool IsDepthBufferEnabled();
    void EnableWindowResize(bool p_ResizeEnabled);
    bool IsWindowResizeEnabled();

protected:
    // Core virtual methods used by derived classes
    virtual void Configure() = 0;   // Application's user configuration prior to Setup()
    virtual void Setup() = 0;       // Set's up the drawing pipeline
    virtual void Update() = 0;      // Update data prior to Render() & Present() such as updating locals, uniforms etc.

    virtual bool Render() = 0;      // Draw the primitive on surface
    virtual bool Present() = 0;     // Swap the drawn surface on application window

public:
    virtual void Run() = 0;

public:
    virtual void ResizeWindow(int p_Width, int p_Height) = 0;
    virtual void MousePressEvent(QMouseEvent* p_Event) = 0;
    virtual void MouseReleaseEvent(QMouseEvent* p_Event) = 0;
    virtual void MouseMoveEvent(QMouseEvent* p_Event) = 0;

protected:
    // Application display window
    Window*         m_Window;               // Display window object
    glm::u32vec2    m_WindowDimension;      // Display window dimension
    std::string     m_AppName;              // Display name
    bool            m_DepthEnabled;         // Is depth buffer supported
    bool            m_ResizeEnabled;        // Is Window resize support enabled
};

