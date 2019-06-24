#pragma once
#include "../engine/vulkan/core/VulkanApp.h"

#include "../common/SceneGraph/RenderSchemeFactory.h"

#include "RectangleDescriptorSet.h"

class Rectangl;
class VulkanRectangleInstancingScheme : public RenderSchemeFactory
{
public:
    VulkanRectangleInstancingScheme(BaseRenderer* p_Renderer);
    virtual ~VulkanRectangleInstancingScheme();

public:
    virtual void Setup();
    virtual void Update();
    void ResizeWindow();

private:
    void CreateGraphicsPipeline(bool p_ClearGraphicsPipelineMap = false);
    void CreateRectFillPipeline();
    void CreateRectOutlinePipeline();

    void RecordCommandBuffer();
    void CreateVertexBuffer();

    void Render(void* p_CmdBuffer);

    virtual void UpdatePipelineNodeList(Node* p_Item);
    virtual void RemovePipelineNodeList(Node* p_Item);

    enum RECTANGLE_GRAPHICS_PIPELINES
    {
        PIPELINE_FILLED = 0,
        PIPELINE_OUTLINE,
        PIPELINE_COUNT,
    };

    // PrepareInstance: This function allocate all the GPU object in the contigous location and store the index in the repective CPU objects
    void PrepareInstanceData(RECTANGLE_GRAPHICS_PIPELINES p_Pipeline = PIPELINE_COUNT);
#ifdef DEPRECATE_PIPELINE_SETUP
    void PrepareInstanceData(Rectangl* p_Rectangle);
#else
    void PrepareInstanceData(RECTANGLE_GRAPHICS_PIPELINES p_Pipeline = PIPELINE_COUNT);
#endif

    // UpdateDirtyInstanceData: This function only update the dirty location in the GPU.
    void UpdateDirtyInstanceData();

    std::vector<VkVertexInputBindingDescription>   m_VertexInputBinding[PIPELINE_COUNT];   // 0 for (position and color) 1 for ()
    std::vector<VkVertexInputAttributeDescription> m_VertexInputAttribute[PIPELINE_COUNT]; // Why 7 = 2(for position and color) + 5 (transform and rotation) + Color

    ////////////////////////////////////////////////////////////////
public:
    // Per-instance data block
    struct InstanceData {
        glm::mat4 m_Model;
        glm::vec4 m_Rect;
        glm::vec4 m_Color;
        uint m_BoolFlags; // [0] Visibility [1] Unused [2] Unused [3] Unused
    };

    VulkanBuffer m_VertexBuffer[PIPELINE_COUNT];
    int m_VertexCount[PIPELINE_COUNT];
    typedef std::vector<Node*> NodeVector;
    VulkanBuffer m_InstanceBuffer[PIPELINE_COUNT];
    NodeVector m_PipelineTypeModelVector[PIPELINE_COUNT];
    int m_OldInstanceDataSize[PIPELINE_COUNT];

//    NodeVector m_PipelineTypeDirtyNodeList[PIPELINE_COUNT];

//    /*std::shared_ptr<*/RectangleDescriptorSet*/*>*/ CDS;// std::make_shared<CubeDescriptorSet>(m_VulkanApplication);
//    RectangleDescriptorSet::UniformBufferObj* UniformBuffer = NULL;
    std::shared_ptr<RectangleDescriptorSet> m_DescriptorSet;
    VulkanApp* m_VulkanRenderer;
};
