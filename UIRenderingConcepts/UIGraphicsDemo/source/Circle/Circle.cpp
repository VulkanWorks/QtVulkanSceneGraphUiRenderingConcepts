#include  "Circle.h"

#ifdef ENGINE_METAL
#else
#include  "../engine/vulkan/implementation/Circle/CircleDescriptorSet.h"
#include  "../engine/vulkan/implementation/Circle/CircleGeometry.h"
#include  "../engine/vulkan/implementation/Circle/VulkanCircleMultiDrawScheme.h"
#include  "../engine/vulkan/implementation/Circle/CircleShaderTypes.h"
#endif

Circle::Circle(Scene* p_Scene, Node* p_Parent, const BoundingRegion& p_BoundedRegion, const QString& p_Name)
    : Node(p_Scene, p_Parent, p_BoundedRegion, p_Name, SHAPE::SHAPE_CIRCLE_MULTIDRAW)
    , m_DrawType(FILLED)
{
    p_Scene->AddToPipeline(this);
}

Circle::Circle(Scene* p_Scene, Node* p_Parent, glm::vec2 m_Center, float radius, const QString& p_Name)
    : Node(p_Scene, p_Parent, BoundingRegion(m_Center.x - (radius * 0.5f), m_Center.y - (radius * 0.5f), radius, radius), p_Name, SHAPE::SHAPE_CIRCLE_MULTIDRAW)
    , m_DrawType(FILLED)
{
    p_Scene->AddToPipeline(this);
}

Circle::~Circle()
{
    GetScene()->RemoveFromPipeline(this);
}

RenderSchemeFactory* Circle::GetRenderSchemeFactory()
{
#ifdef ENGINE_METAL
#else
    return new VulkanCircleMultiDrawScheme(static_cast<VulkanApp*>(m_Scene->GetApplication()));
#endif
}

//void Circle::Setup()
//{
//    CreateCircleVertexBuffer();

//    Node::Setup();
//}

//void Circle::CreateCircleVertexBuffer()
//{
//#ifdef ENGINE_METAL
//#else
//    glm::mat4 parentTransform = GetAbsoluteTransformation();//m_Model * GetParentsTransformation(GetParent());

//    CircleVertex rectVertices[6];
//    memcpy(rectVertices, circleFilledVertices, sizeof(CircleVertex) * 6);
//    uint32_t dataSize = sizeof(rectVertices);
//    uint32_t dataStride = sizeof(rectVertices[0]);
//    const int vertexCount = dataSize / dataStride;
//    for (int i = 0; i < vertexCount; ++i)
//    {
//        glm::vec4 pos(circleFilledVertices[i].m_Position, 1.0);
//        pos.x = pos.x * m_BoundedRegion.m_Dimension.x;
//        pos.y = pos.y * m_BoundedRegion.m_Dimension.y;

//        pos = parentTransform * pos;
//        //std::cout << m_Name.toStdString() << "=+ x:" << pos.x << ", y:" << pos.y << ", z:" << pos.z << endl;

//        rectVertices[i].m_Position.x = pos.x;
//        rectVertices[i].m_Position.y = pos.y;
//        rectVertices[i].m_Position.z = pos.z;
//    }

//    VulkanApp* app = static_cast<VulkanApp*>(m_Scene->GetApplication());
//    const VkDevice& device = app->m_hDevice;

//    if (m_VertexBuffer.m_Buffer != VK_NULL_HANDLE)
//    {
//        vkDestroyBuffer(device, m_VertexBuffer.m_Buffer, NULL);
//        vkFreeMemory(device, m_VertexBuffer.m_Memory, NULL);
//    }

//    m_VertexBuffer.m_DataSize = dataSize;
//    m_VertexBuffer.m_MemoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

//    const VkPhysicalDeviceMemoryProperties& memProp = app->m_physicalDeviceInfo.memProp;
//    VulkanHelper::CreateBuffer(device, memProp, m_VertexBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, rectVertices);
//#endif
//}
