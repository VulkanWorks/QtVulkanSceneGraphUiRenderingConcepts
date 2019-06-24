#pragma once
#include "../engine/vulkan/core/VulkanApp.h"

#include "../common/SceneGraph/RenderSchemeFactory.h"

struct CircleDescriptorSet;
class GRPX_Pipeline
{
public:
    GRPX_Pipeline();
    ~GRPX_Pipeline();

    void* InputBinding();     // Specify Geometry layout in the vertex shader
    void* Pipeline();         // Underlaying pipeline implementation
    void* DescriptorSet();    // Specify uniform locations
    void* AssemblyState();

    VkGraphicsPipelineCreateInfo pipelineInfo   = VulkanHelper::GraphicsPipelineCreateInfo();
//        pipelineInfo.stageCount                     = 2;
//        pipelineInfo.pStages                        = shaderStages;
//        pipelineInfo.pVertexInputState              = &vertexInputInfo;
//        pipelineInfo.pInputAssemblyState            = &inputAssembly;
//        pipelineInfo.pViewportState                 = &viewportState;
//        pipelineInfo.pRasterizationState            = &rasterizer;
//        pipelineInfo.pMultisampleState              = &multisampling;
//        pipelineInfo.pColorBlendState               = &colorBlending;
//#ifdef ENABLE_DYNAMIC_STATE
//        pipelineInfo.pDynamicState                  = &dynamicState;
//#endif
//        pipelineInfo.layout                         = graphicsPipelineLayout;
//        pipelineInfo.renderPass                     = m_VulkanRenderer->m_hRenderPass;
//        pipelineInfo.subpass                        = 0;
//        pipelineInfo.basePipelineHandle             = VK_NULL_HANDLE;
//        pipelineInfo.pDepthStencilState             = &depthStencilStateInfo;

};

class Circle;
class VulkanCircleMultiDrawScheme : public RenderSchemeFactory
{
public:
    VulkanCircleMultiDrawScheme(BaseRenderer* p_Renderer);
    virtual ~VulkanCircleMultiDrawScheme();

public:
    //virtual void Setup();
    virtual void Update();
    virtual void Render(void* p_CommandBuffer);

    void ResizeWindow();

private:
    void CreateGraphicsPipeline(bool p_ClearGraphicsPipelineMap = false);
    void CreateCircleFillPipeline();
    void CreateCircleOutlinePipeline();
    void CreateVertexLayoutBinding();

    void createPushConstants();

    virtual void UpdatePipelineNodeList(Node* p_Item);
    virtual void RemovePipelineNodeList(Node* p_Model);
    std::map<Node*, VulkanBuffer*> m_NodeBufferMap; // TODO: Fix the memory leak present the VulkanBuffer is not released.

    enum CIRCLE_GRAPHICS_PIPELINES
    {
        PIPELINE_FILLED = 0,
        PIPELINE_OUTLINE,
        PIPELINE_COUNT,
    };

    // UpdateDirtyMultiDrawData: This function allocate all the GPU object in the contigous location and store the index in the repective CPU objects
#ifdef DEPRECATE_PIPELINE_SETUP
    void PrepareMultiDrawData(Circle* p_Circle);
#else
    void PrepareMultiDrawData();
#endif
    // UpdateDirtyMultiDrawData: This function only update the dirty location in the GPU.
    void UpdateDirtyMultiDrawData();


    std::vector<VkVertexInputBindingDescription>   m_VertexInputBinding[PIPELINE_COUNT];   // 0 for (position and color) 1 for ()
    std::vector<VkVertexInputAttributeDescription> m_VertexInputAttribute[PIPELINE_COUNT]; // Why 7 = 2(for position and color) + 5 (transform and rotation) + Color

    typedef std::vector<Node*> NodeVector;
    NodeVector m_PipelineTypeModelVector[PIPELINE_COUNT];

    std::shared_ptr<CircleDescriptorSet> m_DescriptorSet;
    VulkanApp* m_VulkanRenderer;
};
