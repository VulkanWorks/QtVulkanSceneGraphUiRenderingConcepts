#pragma once

#include <QObject>
#include <QMap>
#include "Transformation.h"
#include "../../engine/vulkan/core/VulkanHelper.h"

#include "Common.h"

class Node;
class Scene;
class QMouseEvent;
class RenderSchemeFactory;
class BaseRenderer;

class ViewInfo
{
public:
    enum VIEW_TYPE
    {
        VIEW_TYPE_NONE = -1,
        VIEW_TYPE_2D,
        VIEW_TYPE_3D,
        SUPPORTED_VIEW_TYPE_COUNT,
    };

    struct View2DParams
    {
        View2DParams(float p_Left, float p_Right, float p_Bottom, float p_Top, float p_Front = -100, float p_Back = 100)
            : left(p_Left)
            , right(p_Right)
            , bottom(p_Bottom)
            , top(p_Top)
            , front(p_Front)
            , back(p_Back)
        {
        }

        float left      = -100.0f;
        float right     =  100.0f;
        float bottom    = -100.0f;
        float top       =  100.0f;
        float front     = -100.0f;
        float back      =  100.0f;
    };

    struct View3DParams
    {
        View3DParams(float p_FOV, float p_AspectRatio, float p_Front, float p_Back, float p_CameraDistance)
            : fov(p_FOV)
            , aspectRatio(p_AspectRatio)
            , nearPlane(p_Front)
            , farPlane(p_Back)
            , cameraDistance(p_CameraDistance)
        {
        }

        float fov           = 60.0f;
        float aspectRatio   = 1.0f;
        float nearPlane     = 0.01f;
        float farPlane      = 1000.0f;
        float cameraDistance= -10.0f;
    };

    union ProjectionParam
    {
        ProjectionParam() {}
        View2DParams m_Param2D;
        View3DParams m_Param3D;
    };

    ViewInfo() {}
    ~ViewInfo() {}

    void SetView2DParam(float p_Left, float p_Right, float p_Bottom, float p_Top, float p_Front = -100, float p_Back = 100)
    {
        m_ViewAndProjectionInfo.m_ViewType = VIEW_TYPE_2D;
        m_ViewAndProjectionInfo.m_ProjectionParam.m_Param2D = { p_Left, p_Right, p_Bottom, p_Top, p_Front, p_Back };
    }
    View2DParams GetView2DParam() const { return m_ViewAndProjectionInfo.m_ProjectionParam.m_Param2D; }

    void SetView3DParam(float p_FOV, float p_AspectRatio, float p_Front, float p_Back, float p_CameraDistance)
    {
        m_ViewAndProjectionInfo.m_ViewType = VIEW_TYPE_3D;
        m_ViewAndProjectionInfo.m_ProjectionParam.m_Param3D = { p_FOV, p_AspectRatio, p_Front, p_Back, p_CameraDistance };
    }
    View3DParams GetView3DParam() const { return m_ViewAndProjectionInfo.m_ProjectionParam.m_Param3D; }

    void SetViewType(VIEW_TYPE p_ViewType) { m_ViewAndProjectionInfo.m_ViewType = p_ViewType; }
    VIEW_TYPE GetViewType() const { return m_ViewAndProjectionInfo.m_ViewType; }

    void SetIsUsingDynamicWindowExtent(bool p_UseDynamicWindowExtent) { m_ViewAndProjectionInfo.m_UseSurfaceExtent = p_UseDynamicWindowExtent; }
    bool IsUsingDynamicWindowExtent() const { return m_ViewAndProjectionInfo.m_UseSurfaceExtent; } // Engine automatically retreive the correct surface dimension for the drawable area.

private:

    struct ViewAndProjectionInfo
    {
        ViewAndProjectionInfo() { m_ViewType = VIEW_TYPE_2D; }

        VIEW_TYPE               m_ViewType;
        union ProjectionParam   m_ProjectionParam;
        bool                    m_UseSurfaceExtent = true;  // Metal: Metal layer extent/swapchain extent
                                                            // - (Value: true) Overides the user specified width and height dimension
                                                            //   and uses underlying layers drawable width and height. This is helpful
    };

    ViewAndProjectionInfo m_ViewAndProjectionInfo;
};

class Scene
{
public:
    Scene(BaseRenderer* p_Application = NULL, const QString& p_Name = QString());
    virtual ~Scene();

    void Setup();
    void Update();
    void Render(void* p_CommandBuffer); // TODO do not expose implementation here, Vulkan must be abstracted

    void AddItem(Node* p_Item);
    void RemoveItem(Node* p_Item);

    void AddToPipeline(Node* p_Node);
    void RemoveFromPipeline(Node* p_Node);

    void SetSceneRect(float p_OriginX, float p_OriginY, float p_Width, float p_Height);
    void SetScenePrespective(float p_FOV, float p_AspectRatio, float p_Front, float p_Back, float p_CameraDistance);

    void UseDynamicSurfaceDimensions(bool p_UseDynamicWindowExtent) { m_ViewInfo.SetIsUsingDynamicWindowExtent(p_UseDynamicWindowExtent);}
    bool IsUsingDynamicSurfaceDimensions() const { return m_ViewInfo.IsUsingDynamicWindowExtent(); }

    virtual void Resize(int p_Width, int p_Height);
    inline Transformation& Transform() { return m_Transform; }

    bool IsDirty() { return (m_DirtyType != SCENE_DIRTY_TYPE::NONE); }
    SCENE_DIRTY_TYPE GetDirtyType() { return m_DirtyType; }
    void SetDirtyType(SCENE_DIRTY_TYPE p_InvalidateType) { m_DirtyType = p_InvalidateType; }

    virtual void mousePressEvent(QMouseEvent* p_Event);
    virtual void mouseReleaseEvent(QMouseEvent* p_Event);
    virtual void mouseMoveEvent(QMouseEvent* p_Event);

    void PushMatrix() { m_Transform.PushMatrix(); }
    void PopMatrix() { m_Transform.PopMatrix(); }
    void ApplyTransformation(const glm::mat4& p_TransformationMatrix) { *m_Transform.GetModelMatrix() *= p_TransformationMatrix; }

private:
    RenderSchemeFactory* GetRenderSchemeFactory(Node* p_Item);
    void AppendToFlatNodeList(Node* p_Item);
    void GatherFlatNodeList();

private:
    std::vector<Node*>                      m_RootDrawableList;
    std::vector<Node*>                      m_FlatList;
    std::set<RenderSchemeFactory*>          m_RenderSchemeFactorySet;
    std::multimap<SHAPE, RenderSchemeFactory*>   m_ShapeRenderSchemeTypeMap;

    GETSET(Node*,                           CurrentHoverItem)  // Not owned by Scene
    GETSET(BaseRenderer*,                   Application)
    GETSET(QString,                         Name)
    GETSET(int,                             ScreenHeight);
    GETSET(int,                             ScreenWidth);
    GETSET(int,                             Frame);
    GETSET(Transformation,                  Transform);
    GETSET(bool,                            EarlyDepthTest);
    GETSET(float,                           PainterAlgoOrder); // Maintain the depth of all node added to scene in-order

private:
    SCENE_DIRTY_TYPE m_DirtyType;
    ViewInfo m_ViewInfo;
    friend class Node;
};
