#include "Rect.h"

#ifdef ENGINE_METAL
#else
#include "../engine/vulkan/implementation/Rectangle/RectangleDescriptorSet.h"
#include "../engine/vulkan/implementation/Rectangle/RectangleGeometry.h"
#include "../engine/vulkan/implementation/Rectangle/VulkanRectangleMultiDrawScheme.h"
#include "../engine/vulkan/implementation/Rectangle/VulkanRectangleInstancingScheme.h"
#include "../engine/vulkan/implementation/Rectangle/RectangleShaderTypes.h"
#endif

Rectangl::Rectangl(Scene* p_Scene, Node* p_Parent, const BoundingRegion& p_BoundedRegion, const QString& p_Name, SHAPE p_ShapeType)
    : Node(p_Scene, p_Parent, p_BoundedRegion, p_Name, p_ShapeType)
    , m_DrawType(FILLED)
{
    p_Scene->AddToPipeline(this);
}

Rectangl::~Rectangl()
{
    GetScene()->AddToPipeline(this);
}

RenderSchemeFactory* Rectangl::GetRenderSchemeFactory()
{
#ifdef ENGINE_METAL
#else
    if (m_ShapeType == SHAPE::SHAPE_RECTANGLE_MULTIDRAW)
    {
        return new VulkanRectangleMultiDrawScheme(static_cast<VulkanApp*>(m_Scene->GetApplication()));
    }
    else if (m_ShapeType == SHAPE::SHAPE_RECTANGLE_INSTANCED)
    {
        return new VulkanRectangleInstancingScheme(static_cast<VulkanApp*>(m_Scene->GetApplication()));
    }
#endif

    assert(false);
    return NULL;
}
