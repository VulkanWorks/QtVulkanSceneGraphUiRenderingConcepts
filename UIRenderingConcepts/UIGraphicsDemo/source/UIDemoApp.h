#pragma once
#include "../../engine/vulkan/core/VulkanApp.h"
#include "../../common/SceneGraph/Scene.h"
#include "UIDemo.h"

class UiMetalPaintEngine;
class Rectangl;
class Circle;
class UIDemoApp : public VulkanApp
{
public:
    UIDemoApp();
    virtual ~UIDemoApp();

    virtual void Configure();
    virtual void Setup();
    virtual void Update();
    virtual bool Render();
    virtual void ResizeWindow(int p_Width, int p_Height);

    virtual bool InitMetalEngine();

protected:
    virtual void MousePressEvent(QMouseEvent* p_Event);
    virtual void MouseReleaseEvent(QMouseEvent* p_Event);
    virtual void MouseMoveEvent(QMouseEvent* p_Event);

private:
    void RecordRenderPass(int p_Argcount, ...);

private:
    //std::vector<std::shared_ptr<Scene>> m_SceneVector;
    Scene* m_Scene;
    Scene* m_ScenePainterEngine;
    UIDemo m_UIDemo;
    QScopedPointer<UiMetalPaintEngine> m_MetalPaintEngine;
};
