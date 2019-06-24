#pragma once
#include "../../../engine/vulkan/core/VulkanApp.h"

#include "../../../common/SceneGraph/Node.h"

class Rectangl : public Node
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
    Rectangl(Scene* p_Scene, Node* p_Parent, const BoundingRegion& p_BoundedRegion, const QString& p_Name = "", SHAPE p_ShapeType = SHAPE::SHAPE_NONE);
    virtual ~Rectangl();

    GETSET(DRAW_TYPE, DrawType)

public:
    virtual RenderSchemeFactory* GetRenderSchemeFactory();
};
