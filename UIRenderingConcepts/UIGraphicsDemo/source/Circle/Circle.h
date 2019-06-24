#pragma once
#include "../../../engine/vulkan/core/VulkanApp.h"
//#include "../Rectangle/Rect.h"
//#include "../Rectangle/RectangleDescriptorSet.h"
#include  "../engine/vulkan/implementation/Circle/CircleDescriptorSet.h"

#include "../../../common/SceneGraph/Transformation.h"
#include "../../../common/SceneGraph/Scene.h"
#include "../../../common/SceneGraph/Node.h"

class VulkanCircleMultiDrawScheme;

class Circle : public Node
{
public:
    enum DRAW_TYPE
    {
        FILLED = 0,
        OUTLINE,
        ROUNDED,
        DRAW_TYPE_COUNT
    };

public:
    Circle(Scene* p_Scene, Node* p_Parent, const BoundingRegion& p_BoundedRegion, const QString& p_Name = "");
    Circle(Scene* p_Scene, Node* p_Parent, glm::vec2 m_Center, float radius, const QString& p_Name = "");
    virtual ~Circle();

    GETSET(DRAW_TYPE, DrawType)
        
protected:
//    virtual void UpXXXdate(Node* p_Item = NULL)
//    {
//        Node::UpXXXdate(p_Item);

//        CreateCircleVertexBuffer();
//    }

public:
//    void UpdateMeAndMyChildren()
//    {
//        Update(this);
//    }

    RenderSchemeFactory* GetRenderSchemeFactory();

//    virtual void Setup();
//    void CreateCircleVertexBuffer();
//    VulkanBuffer m_VertexBuffer;
};
