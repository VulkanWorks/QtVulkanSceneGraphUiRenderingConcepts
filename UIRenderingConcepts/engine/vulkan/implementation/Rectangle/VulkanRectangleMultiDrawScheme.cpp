#include "VulkanRectangleMultiDrawScheme.h"
#include <QDebug>

#include "../../../../UIGraphicsDemo/source/Rectangle/Rect.h"
#include "RectangleDescriptorSet.h"
#include "RectangleGeometry.h"
#include "RectangleShaderTypes.h"

VulkanRectangleMultiDrawScheme::VulkanRectangleMultiDrawScheme(BaseRenderer* p_Renderer)
    : RenderSchemeFactory(p_Renderer)
    , m_VulkanRenderer(static_cast<VulkanApp*>(p_Renderer))
{
    assert(m_VulkanRenderer);
    /////////////////

#ifdef DEPRECATE_PIPELINE_SETUP
    m_DescriptorSet = std::make_shared<RectangleDescriptorSet>(m_VulkanRenderer);

    CreateVertexLayoutBinding();

    CreateGraphicsPipeline();

    createPushConstants();
#else
    m_DescriptorSet = NULL;
#endif
}

VulkanRectangleMultiDrawScheme::~VulkanRectangleMultiDrawScheme()
{
    for (int pipelineIdx = 0; pipelineIdx < RECTANGLE_GRAPHICS_PIPELINES::PIPELINE_COUNT; pipelineIdx++)
    {
        NodeVector& modelList = m_PipelineTypeModelVector[pipelineIdx];
        const int modelSize = modelList.size();
        if (!modelSize) continue;

        // Destroy Vertex Buffer
        for (int j = 0; j < modelSize; j++)
        {
            if (modelList.at(j)->GetRefShapeType() == SHAPE::SHAPE_RECTANGLE_MULTIDRAW)
            {
                Rectangl* model = (static_cast<Rectangl*>(modelList.at(j)));
                if (!model) return;

                std::map<Node*, VulkanBuffer*>::iterator it = m_NodeBufferMap.find(modelList.at(j));
                if (it != m_NodeBufferMap.end())
                {
                    vkDestroyBuffer(m_VulkanRenderer->m_hDevice, it->second->m_Buffer, NULL);
                    vkFreeMemory(m_VulkanRenderer->m_hDevice, it->second->m_Memory, NULL);
                    delete it->second;
                    m_NodeBufferMap.erase(it);
                }
            }
        }
    }

    //// Destroy descriptors
    //for (int i = 0; i < descLayout.size(); i++) {
    //       vkDestroyDescriptorSetLayout(m_VulkanRenderer->m_hDevice, descLayout[i], NULL);
    //}
    //descLayout.clear();

    //vkFreeDescriptorSets(m_VulkanRenderer->m_hDevice, descriptorPool, (uint32_t)descriptorSet.size(), &descriptorSet[0]);
    //vkDestroyDescriptorPool(m_VulkanRenderer->m_hDevice, descriptorPool, NULL);

    RectangleDescriptorSet::UniformBufferObj* UniformBuffer = m_DescriptorSet->UniformBuffer;
    vkUnmapMemory(m_VulkanRenderer->m_hDevice, UniformBuffer->m_BufObj.m_Memory);
    vkDestroyBuffer(m_VulkanRenderer->m_hDevice, UniformBuffer->m_BufObj.m_Buffer, NULL);
    vkFreeMemory(m_VulkanRenderer->m_hDevice, UniformBuffer->m_BufObj.m_Memory, NULL);
}

//void VulkanRectangleMultiDrawScheme::Setup()
//{
//#ifndef DEPRECATE_PIPELINE_SETUP
//    m_DescriptorSet = std::make_shared<RectangleDescriptorSet>(m_VulkanRenderer);

//    CreateVertexLayoutBinding();

//    CreateGraphicsPipeline();

//    PrepareMultiDrawData();

//    // Build the push constants
//    createPushConstants();
//#endif
//}

void VulkanRectangleMultiDrawScheme::Update()
{
    RectangleDescriptorSet::UniformBufferObj* UniformBuffer = m_DescriptorSet->UniformBuffer;
    VulkanHelper::WriteMemory(m_VulkanRenderer->m_hDevice,
        UniformBuffer->m_MappedMemory,
        UniformBuffer->m_MappedRange,
        UniformBuffer->m_BufObj.m_MemoryFlags,
                  &m_ProjectViewMatrix, sizeof(m_ProjectViewMatrix));

    UpdateDirtyMultiDrawData();
}

void VulkanRectangleMultiDrawScheme::ResizeWindow(/*void* p_CommandBuffer*/)
{
#ifdef RECREATE_PSO
    CreateGraphicsPipeline(true);
    //Render(p_CommandBuffer);
#endif
}

void VulkanRectangleMultiDrawScheme::CreateRectOutlinePipeline()
{
    VkShaderModule vertShader = VulkanHelper::CreateShaderFromQRCResource(m_VulkanRenderer->m_hDevice, "://RectangleMultiDrawSchemeVert.spv");
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = VulkanHelper::PipelineShaderStageCreateInfo();
        vertShaderStageInfo.stage                       = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module                      = vertShader;
        vertShaderStageInfo.pName                       = "main";

    VkShaderModule fragShader = VulkanHelper::CreateShaderFromQRCResource(m_VulkanRenderer->m_hDevice, "://RectangleMultiDrawSchemeFrag.spv");
    VkPipelineShaderStageCreateInfo fragShaderStageInfo = VulkanHelper::PipelineShaderStageCreateInfo();
        fragShaderStageInfo.stage                       = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module                      = fragShader;
        fragShaderStageInfo.pName                       = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

#ifdef ENABLE_DYNAMIC_STATE
    VkDynamicState dynamicStateEnables[VK_DYNAMIC_STATE_RANGE_SIZE];
    memset(dynamicStateEnables, 0, sizeof dynamicStateEnables);
    VkPipelineDynamicStateCreateInfo dynamicState = VulkanHelper::PipelineDynamicStateCreateInfo();
        dynamicState.pDynamicStates    = dynamicStateEnables;
#endif

    VkPipelineVertexInputStateCreateInfo vertexInputInfo    = VulkanHelper::PipelineVertexInputStateCreateInfo();
        vertexInputInfo.vertexBindingDescriptionCount       = static_cast<uint32_t>(m_VertexInputBinding[PIPELINE_OUTLINE].size());
        vertexInputInfo.pVertexBindingDescriptions          = m_VertexInputBinding[PIPELINE_OUTLINE].data();
        vertexInputInfo.vertexAttributeDescriptionCount     = static_cast<uint32_t>(m_VertexInputAttribute[PIPELINE_OUTLINE].size());
        vertexInputInfo.pVertexAttributeDescriptions        = m_VertexInputAttribute[PIPELINE_OUTLINE].data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = VulkanHelper::PipelineInputAssemblyStateCreateInfo();
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;

    VkViewport viewport = { 0.0f, 0.0f, static_cast<float>(m_VulkanRenderer->m_swapChainExtent.width), static_cast<float>(m_VulkanRenderer->m_swapChainExtent.height), 0.0f, 1.0f };
    VkRect2D scissor    = { {0, 0}, m_VulkanRenderer->m_swapChainExtent};
    VkPipelineViewportStateCreateInfo viewportState = VulkanHelper::PipelineViewportStateCreateInfo();
        viewportState.viewportCount = 1;
        viewportState.pViewports    = &viewport;
        viewportState.scissorCount  = 1;
        viewportState.pScissors     = &scissor;

#ifdef ENABLE_DYNAMIC_STATE
    // Specify the dynamic state count and VkDynamicState enum stating which
    // dynamic state will use the values from dynamic state commands rather
    // than from the pipeline state creation info.
    dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
    dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;
#endif

    VkPipelineRasterizationStateCreateInfo rasterizer           = VulkanHelper::PipelineRasterizationStateCreateInfo();
    VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo = VulkanHelper::PipelineDepthStencilStateCreateInfo();
    VkPipelineMultisampleStateCreateInfo multisampling          = VulkanHelper::PipelineMultisampleStateCreateInfo();

    VkPipelineColorBlendAttachmentState colorBlendAttachment    = VulkanHelper::PipelineColorBlendAttachmentState();
    VkPipelineColorBlendStateCreateInfo colorBlending           = VulkanHelper::PipelineColorBlendStateCreateInfo();
        colorBlending.pAttachments                              = &colorBlendAttachment;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo   = VulkanHelper::PipelineLayoutCreateInfo();
        pipelineLayoutInfo.setLayoutCount           = static_cast<uint32_t>(m_DescriptorSet->descLayout.size());
        pipelineLayoutInfo.pSetLayouts              = m_DescriptorSet->descLayout.data();

    VkPipelineLayout graphicsPipelineLayout = VK_NULL_HANDLE;
    VkResult vkResult = vkCreatePipelineLayout(m_VulkanRenderer->m_hDevice, &pipelineLayoutInfo, nullptr, &graphicsPipelineLayout);

    if (vkResult != VK_SUCCESS)
    {
        VulkanHelper::LogError("vkCreatePipelineLayout() failed!");
        assert(false);
    }

    VkGraphicsPipelineCreateInfo pipelineInfo   = VulkanHelper::GraphicsPipelineCreateInfo();
        pipelineInfo.stageCount                 = 2;
        pipelineInfo.pStages                    = shaderStages;
        pipelineInfo.pVertexInputState          = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState        = &inputAssembly;
        pipelineInfo.pViewportState             = &viewportState;
        pipelineInfo.pRasterizationState        = &rasterizer;
        pipelineInfo.pMultisampleState          = &multisampling;
        pipelineInfo.pColorBlendState           = &colorBlending;
#ifdef ENABLE_DYNAMIC_STATE
        pipelineInfo.pDynamicState              = &dynamicState;
#endif
        pipelineInfo.layout                     = graphicsPipelineLayout;
        pipelineInfo.renderPass                 = m_VulkanRenderer->m_hRenderPass;
        pipelineInfo.subpass                    = 0;
        pipelineInfo.basePipelineHandle         = VK_NULL_HANDLE;
        pipelineInfo.pDepthStencilState         = &depthStencilStateInfo;

    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
    vkResult = vkCreateGraphicsPipelines(m_VulkanRenderer->m_hDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline);
    if (vkResult != VK_SUCCESS)
    {
        VulkanHelper::LogError("vkCreateGraphicsPipelines() failed!");
        assert(false);
    }

    QPair<void*/*VkPipeline*/, void*/*VkPipelineLayout*/>* graphicsPipelineAndLayoutPair = new QPair<void*/*VkPipeline*/, void*/*VkPipelineLayout*/>(graphicsPipeline, graphicsPipelineLayout);
    m_GraphicsPipelineMap[PIPELINE_OUTLINE] = graphicsPipelineAndLayoutPair;

    vkDestroyShaderModule(m_VulkanRenderer->m_hDevice, fragShader, nullptr);
    vkDestroyShaderModule(m_VulkanRenderer->m_hDevice, vertShader, nullptr);
}

void VulkanRectangleMultiDrawScheme::CreateRectFillPipeline()
{
    VkShaderModule vertShader = VulkanHelper::CreateShaderFromQRCResource(m_VulkanRenderer->m_hDevice, "://RectangleMultiDrawSchemeVert.spv");
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = VulkanHelper::PipelineShaderStageCreateInfo();
        vertShaderStageInfo.stage                       = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module                      = vertShader;
        vertShaderStageInfo.pName                       = "main";

    VkShaderModule fragShader                           = VulkanHelper::CreateShaderFromQRCResource(m_VulkanRenderer->m_hDevice, "://RectangleMultiDrawSchemeFrag.spv");
    VkPipelineShaderStageCreateInfo fragShaderStageInfo = VulkanHelper::PipelineShaderStageCreateInfo();
        fragShaderStageInfo.stage                       = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module                      = fragShader;
        fragShaderStageInfo.pName                       = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

#ifdef ENABLE_DYNAMIC_STATE
    VkDynamicState dynamicStateEnables[VK_DYNAMIC_STATE_RANGE_SIZE];
    memset(dynamicStateEnables, 0, sizeof dynamicStateEnables);
    VkPipelineDynamicStateCreateInfo dynamicState = VulkanHelper::PipelineDynamicStateCreateInfo();
        dynamicState.pDynamicStates    = dynamicStateEnables;
#endif

    VkPipelineVertexInputStateCreateInfo vertexInputInfo    = VulkanHelper::PipelineVertexInputStateCreateInfo();
        vertexInputInfo.vertexBindingDescriptionCount       = static_cast<uint32_t>(m_VertexInputBinding[PIPELINE_FILLED].size());
        vertexInputInfo.pVertexBindingDescriptions          = m_VertexInputBinding[PIPELINE_FILLED].data();
        vertexInputInfo.vertexAttributeDescriptionCount     = static_cast<uint32_t>(m_VertexInputAttribute[PIPELINE_FILLED].size());
        vertexInputInfo.pVertexAttributeDescriptions        = m_VertexInputAttribute[PIPELINE_FILLED].data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = VulkanHelper::PipelineInputAssemblyStateCreateInfo();

    VkViewport viewport = { 0.0f, 0.0f, static_cast<float>(m_VulkanRenderer->m_swapChainExtent.width), static_cast<float>(m_VulkanRenderer->m_swapChainExtent.height), 0.0f, 1.0f };
    VkRect2D scissor    = { {0, 0}, m_VulkanRenderer->m_swapChainExtent};
    VkPipelineViewportStateCreateInfo viewportState = VulkanHelper::PipelineViewportStateCreateInfo();
        viewportState.viewportCount = 1;
        viewportState.pViewports    = &viewport;
        viewportState.scissorCount  = 1;
        viewportState.pScissors     = &scissor;

#ifdef ENABLE_DYNAMIC_STATE
    // Specify the dynamic state count and VkDynamicState enum stating which
    // dynamic state will use the values from dynamic state commands rather
    // than from the pipeline state creation info.
    dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
    dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;
#endif

    VkPipelineRasterizationStateCreateInfo rasterizer           = VulkanHelper::PipelineRasterizationStateCreateInfo();
    VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo = VulkanHelper::PipelineDepthStencilStateCreateInfo();
    VkPipelineMultisampleStateCreateInfo multisampling          = VulkanHelper::PipelineMultisampleStateCreateInfo();

    VkPipelineColorBlendAttachmentState colorBlendAttachment    = VulkanHelper::PipelineColorBlendAttachmentState();
    VkPipelineColorBlendStateCreateInfo colorBlending           = VulkanHelper::PipelineColorBlendStateCreateInfo();
        colorBlending.pAttachments                              = &colorBlendAttachment;

    const unsigned pushConstantRangeCount = 1;
    VkPushConstantRange pushConstantRanges[pushConstantRangeCount] = {};
        pushConstantRanges[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushConstantRanges[0].offset = 0;
        pushConstantRanges[0].size = 16 + sizeof(glm::mat4);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo   = VulkanHelper::PipelineLayoutCreateInfo();
        pipelineLayoutInfo.pushConstantRangeCount   = pushConstantRangeCount;
        pipelineLayoutInfo.pPushConstantRanges      = pushConstantRanges;
        pipelineLayoutInfo.setLayoutCount           = static_cast<uint32_t>(m_DescriptorSet->descLayout.size());
        pipelineLayoutInfo.pSetLayouts              = m_DescriptorSet->descLayout.data();

    VkPipelineLayout graphicsPipelineLayout = VK_NULL_HANDLE;
    VkResult vkResult = vkCreatePipelineLayout(m_VulkanRenderer->m_hDevice, &pipelineLayoutInfo, nullptr, &graphicsPipelineLayout);

    if (vkResult != VK_SUCCESS)
    {
        VulkanHelper::LogError("vkCreatePipelineLayout() failed!");
        assert(false);
    }

    // Create graphics pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo   = VulkanHelper::GraphicsPipelineCreateInfo();
        pipelineInfo.stageCount                 = 2;
        pipelineInfo.pStages                    = shaderStages;
        pipelineInfo.pVertexInputState          = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState        = &inputAssembly;
        pipelineInfo.pViewportState             = &viewportState;
        pipelineInfo.pRasterizationState        = &rasterizer;
        pipelineInfo.pMultisampleState          = &multisampling;
        pipelineInfo.pColorBlendState           = &colorBlending;
#ifdef ENABLE_DYNAMIC_STATE
        pipelineInfo.pDynamicState              = &dynamicState;
#endif
        pipelineInfo.layout                     = graphicsPipelineLayout;
        pipelineInfo.renderPass                 = m_VulkanRenderer->m_hRenderPass;
        pipelineInfo.subpass                    = 0;
        pipelineInfo.basePipelineHandle         = VK_NULL_HANDLE;
        pipelineInfo.pDepthStencilState         = &depthStencilStateInfo;

    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
    vkResult = vkCreateGraphicsPipelines(m_VulkanRenderer->m_hDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline);
    if (vkResult != VK_SUCCESS)
    {
        VulkanHelper::LogError("vkCreateGraphicsPipelines() failed!");
        assert(false);
    }

    QPair<void*/*VkPipeline*/, void*/*VkPipelineLayout*/>* graphicsPipelineAndLayoutPair = new QPair<void*/*VkPipeline*/, void*/*VkPipelineLayout*/>(graphicsPipeline, graphicsPipelineLayout);
    m_GraphicsPipelineMap[PIPELINE_FILLED] = graphicsPipelineAndLayoutPair;

    vkDestroyShaderModule(m_VulkanRenderer->m_hDevice, fragShader, nullptr);
    vkDestroyShaderModule(m_VulkanRenderer->m_hDevice, vertShader, nullptr);
}

void VulkanRectangleMultiDrawScheme::CreateGraphicsPipeline(bool p_ClearGraphicsPipelineMap)
{
    // During the resizing the pipeline may be need to recreated(p_ClearGraphicsPipelineMap=true). For example if the dynamics state
    // are not enable then the viewport and scissoring may be updated by recreation of the pipeline.
    if (p_ClearGraphicsPipelineMap)
    {
        const VkDevice& device = m_VulkanRenderer->m_hDevice;
        QMap<int, void*>::iterator i;
        for (i = m_GraphicsPipelineMap.begin(); i != m_GraphicsPipelineMap.end(); ++i)
        {
            vkDestroyPipeline(m_VulkanRenderer->m_hDevice, static_cast<VkPipeline>(static_cast<QPair<void*/*VkPipeline*/, void*/*VkPipelineLayout*/>* >(i.value())->first), nullptr);
            vkDestroyPipelineLayout(device, static_cast<VkPipelineLayout>(static_cast<QPair<void*/*VkPipeline*/, void*/*VkPipelineLayout*/>* >(i.value())->second), nullptr);
        }

        m_GraphicsPipelineMap.clear();
    }

    if (m_GraphicsPipelineMap.contains(PIPELINE_FILLED)) return;

    CreateRectFillPipeline();
    CreateRectOutlinePipeline();
}

void VulkanRectangleMultiDrawScheme::createPushConstants()
{
    return;
    VkCommandBuffer copyCmd;
    VulkanHelper::AllocateCommandBuffer(m_VulkanRenderer->m_hDevice, m_VulkanRenderer->m_hCommandPool, &copyCmd);
    VulkanHelper::BeginCommandBuffer(copyCmd);

    enum ColorFlag {
        RED = 1,
        GREEN = 2,
        BLUE = 3,
        MIXED_COLOR = 4,
    };

    float mixerValue = 0.3f;
    unsigned constColorRGBFlag = BLUE;

    // Create push constant data, this contain a constant
    // color flag and mixer value for non-const color
    unsigned pushConstants[2] = {};
    pushConstants[0] = constColorRGBFlag;
    memcpy(&pushConstants[1], &mixerValue, sizeof(float));

    // Check if number of push constants does not exceed the allowed size
    int maxPushContantSize = m_VulkanRenderer->m_physicalDeviceInfo.prop.limits.maxPushConstantsSize;
    if (sizeof(pushConstants) > maxPushContantSize) {
        assert(0);
        printf("Push constand size is greater than expected, max allow size is %d", maxPushContantSize);
    }

    //for each (VulkanDrawable* drawableObj in drawableList)
    //{
    //    vkCmdPushConstants(cmdPushConstant, drawableObj->pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pushConstants), pushConstants);
    //}

    VulkanHelper::EndCommandBuffer(copyCmd);
    VulkanHelper::SubmitCommandBuffer(m_VulkanRenderer->m_hGraphicsQueue, copyCmd);
    vkFreeCommandBuffers(m_VulkanRenderer->m_hDevice, m_VulkanRenderer->m_hCommandPool, 1, &copyCmd);

    //CommandBufferMgr::allocCommandBuffer(&deviceObj->device, cmdPool, &cmdPushConstant);
    //CommandBufferMgr::beginCommandBuffer(cmdPushConstant);


    //CommandBufferMgr::endCommandBuffer(cmdPushConstant);
    //CommandBufferMgr::submitCommandBuffer(deviceObj->queue, &cmdPushConstant);
}

void VulkanRectangleMultiDrawScheme::CreateVertexLayoutBinding()
{
    for (int pipelineIdx = 0; pipelineIdx < RECTANGLE_GRAPHICS_PIPELINES::PIPELINE_COUNT; pipelineIdx++)
    {
        if (pipelineIdx == PIPELINE_FILLED)
        {
            m_VertexInputBinding[pipelineIdx].resize(1);   // 0 for position and 1 for color
            m_VertexInputAttribute[pipelineIdx].resize(2); // Why 2 = 2(for position and color

            // Indicates the rate at which the information will be
            // injected for vertex input.
            m_VertexInputBinding[pipelineIdx][0].binding = VERTEX_BUFFER_BIND_IDX;
            m_VertexInputBinding[pipelineIdx][0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            m_VertexInputBinding[pipelineIdx][0].stride = sizeof(Vertex);

            // The VkVertexInputAttribute interpreting the data.
            m_VertexInputAttribute[pipelineIdx][0].binding = VERTEX_BUFFER_BIND_IDX;
            m_VertexInputAttribute[pipelineIdx][0].location = 0;
            m_VertexInputAttribute[pipelineIdx][0].format = VK_FORMAT_R32G32B32_SFLOAT;
            m_VertexInputAttribute[pipelineIdx][0].offset = offsetof(struct Vertex, m_Position);

            m_VertexInputAttribute[pipelineIdx][1].binding = VERTEX_BUFFER_BIND_IDX;
            m_VertexInputAttribute[pipelineIdx][1].location = 1;
            m_VertexInputAttribute[pipelineIdx][1].format = VK_FORMAT_R32G32B32_SFLOAT;
            m_VertexInputAttribute[pipelineIdx][1].offset = offsetof(struct Vertex, m_Color);

//            m_VertexInputAttribute[pipelineIdx][2].binding = VERTEX_BUFFER_BIND_IDX;
//            m_VertexInputAttribute[pipelineIdx][2].location = 2;
//            m_VertexInputAttribute[pipelineIdx][2].format = VK_FORMAT_R32_UINT;
//            m_VertexInputAttribute[pipelineIdx][2].offset = offsetof(struct Vertex, m_DrawType);
        }
        else if (pipelineIdx == PIPELINE_OUTLINE)
        {
            m_VertexInputBinding[pipelineIdx].resize(1);   // 0 for position and 1 for color
            m_VertexInputAttribute[pipelineIdx].resize(2); // Why 2 = 2(for position and color

            // Indicates the rate at which the information will be
            // injected for vertex input.
            m_VertexInputBinding[pipelineIdx][0].binding = VERTEX_BUFFER_BIND_IDX;
            m_VertexInputBinding[pipelineIdx][0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            m_VertexInputBinding[pipelineIdx][0].stride = sizeof(Vertex);

            // The VkVertexInputAttribute interpreting the data.
            m_VertexInputAttribute[pipelineIdx][0].binding = VERTEX_BUFFER_BIND_IDX;
            m_VertexInputAttribute[pipelineIdx][0].location = 0;
            m_VertexInputAttribute[pipelineIdx][0].format = VK_FORMAT_R32G32B32_SFLOAT;
            m_VertexInputAttribute[pipelineIdx][0].offset = offsetof(struct Vertex, m_Position);

            m_VertexInputAttribute[pipelineIdx][1].binding = VERTEX_BUFFER_BIND_IDX;
            m_VertexInputAttribute[pipelineIdx][1].location = 1;
            m_VertexInputAttribute[pipelineIdx][1].format = VK_FORMAT_R32G32B32_SFLOAT;
            m_VertexInputAttribute[pipelineIdx][1].offset = offsetof(struct Vertex, m_Color);
        }
    }
}

void VulkanRectangleMultiDrawScheme::UpdatePipelineNodeList(Node *p_Item)
{
    Rectangl* rectangle = dynamic_cast<Rectangl*>(p_Item);
    assert(rectangle);

    // Note: Based on the draw type push the model in respective pipelines
    // Keep the draw type loose couple with the pipeline type,
    // they may be in one-to-one correspondence but that is not necessary.
    switch (rectangle->GetDrawType())
    {
    case Rectangl::FILLED:
        m_PipelineTypeModelVector[PIPELINE_FILLED].push_back(p_Item);
        break;

    case Rectangl::OUTLINE:
        m_PipelineTypeModelVector[PIPELINE_OUTLINE].push_back(p_Item);
        break;

    case Rectangl::ROUNDED:
        // TODO
        break;

    default:
        break;
    }

#ifdef DEPRECATE_PIPELINE_SETUP
    PrepareMultiDrawData(rectangle);
#endif
}

void VulkanRectangleMultiDrawScheme::RemovePipelineNodeList(Node* p_Model)
{
//    Rectangl* rectangle = dynamic_cast<Rectangl*>(p_Model);
//    assert(rectangle);

//    std::vector<Node*>* modelVector = &m_PipelineTypeModelVector[rectangle->GetDrawType()];
//    if (!modelVector) return;

//    bool isUpdated = false;
//    while (1)
//    {
//        auto result = std::find(std::begin(*modelVector), std::end(*modelVector), rectangle);
//        if (result == std::end(*modelVector)) break;

//        modelVector->erase(result);
//        isUpdated = true;
//    }

//    if (isUpdated)
//    {
//        RECTANGLE_GRAPHICS_PIPELINES pipeline = PIPELINE_COUNT;

//        switch (rectangle->GetDrawType())
//        {
//        case Rectangl::FILLED:
//            pipeline = PIPELINE_FILLED;
//            break;

//        case Rectangl::OUTLINE:
//            pipeline = PIPELINE_OUTLINE;
//            break;

//        case Rectangl::ROUNDED:
//            // TODO
//            break;

//        default:
//            break;
//        }

//        if (pipeline == PIPELINE_COUNT) return;

//        PrepareInstanceData(pipeline);
    //    }
}

#ifdef DEPRECATE_PIPELINE_SETUP
void VulkanRectangleMultiDrawScheme::PrepareMultiDrawData(Rectangl* p_Rectangle)
{
    if (!p_Rectangle) return;

    Vertex rectVertices[6];
    memcpy(rectVertices, s_RectFilledVertices, sizeof(Vertex) * 6);
    uint32_t dataSize = sizeof(rectVertices);
    uint32_t dataStride = sizeof(rectVertices[0]);
    const int vertexCount = dataSize / dataStride;
    glm::mat4 parentTransform = p_Rectangle->GetAbsoluteTransformationComputation();

    for (int i = 0; i < vertexCount; ++i)
    {
        glm::vec4 pos(s_RectFilledVertices[i].m_Position, 1.0);
        pos.x = pos.x * p_Rectangle->GetBoundedRegion().m_Dimension.x;
        pos.y = pos.y * p_Rectangle->GetBoundedRegion().m_Dimension.y;

        pos = parentTransform * pos;
        //std::cout << m_Name.toStdString() << "=+ x:" << pos.x << ", y:" << pos.y << ", z:" << pos.z << endl;

        rectVertices[i].m_Position.x = pos.x;
        rectVertices[i].m_Position.y = pos.y;
        rectVertices[i].m_Position.z = pos.z;
    }

    std::map<Node*, VulkanBuffer*>::iterator it = m_NodeBufferMap.find(p_Rectangle);
    VulkanBuffer* vertexBuffer = NULL;
    if (it != m_NodeBufferMap.end())
    {
        vertexBuffer = it->second;
        if (vertexBuffer->m_Buffer != VK_NULL_HANDLE)
        {
            vkDestroyBuffer(m_VulkanRenderer->m_hDevice, vertexBuffer->m_Buffer, NULL);
            vkFreeMemory(m_VulkanRenderer->m_hDevice, vertexBuffer->m_Memory, NULL);
        }
    }
    else
    {
        vertexBuffer = new VulkanBuffer();
        vertexBuffer->m_DataSize = dataSize;
        vertexBuffer->m_MemoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        m_NodeBufferMap[p_Rectangle] = vertexBuffer;
    }

    const VkPhysicalDeviceMemoryProperties& memProp = m_VulkanRenderer->m_physicalDeviceInfo.memProp;
    VulkanHelper::CreateBuffer(m_VulkanRenderer->m_hDevice, memProp, *vertexBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, rectVertices);
}
#else
void VulkanRectangleMultiDrawScheme::PrepareMultiDrawData()
{
#ifdef ENGINE_METAL
#else
    for (int pipelineIdx = 0; pipelineIdx < PIPELINE_COUNT; ++pipelineIdx)
    {
        NodeVector& modelList = m_PipelineTypeModelVector[pipelineIdx];
        const int modelSize = modelList.size();
        if (!modelSize) continue;

        Vertex rectVertices[6];
        memcpy(rectVertices, s_RectFilledVertices, sizeof(Vertex) * 6);
        uint32_t dataSize = sizeof(rectVertices);
        uint32_t dataStride = sizeof(rectVertices[0]);
        const int vertexCount = dataSize / dataStride;
        for (int j = 0; j < modelSize; j++)
        {
            glm::mat4 parentTransform = modelList.at(j)->GetAbsoluteTransformationComputation();

            for (int i = 0; i < vertexCount; ++i)
            {
                glm::vec4 pos(s_RectFilledVertices[i].m_Position, 1.0);
                pos.x = pos.x * modelList.at(j)->GetBoundedRegion().m_Dimension.x;
                pos.y = pos.y * modelList.at(j)->GetBoundedRegion().m_Dimension.y;

                pos = parentTransform * pos;
                //std::cout << m_Name.toStdString() << "=+ x:" << pos.x << ", y:" << pos.y << ", z:" << pos.z << endl;

                rectVertices[i].m_Position.x = pos.x;
                rectVertices[i].m_Position.y = pos.y;
                rectVertices[i].m_Position.z = pos.z;
            }

            std::map<Node*, VulkanBuffer*>::iterator it = m_NodeBufferMap.find(modelList.at(j));
            VulkanBuffer* vertexBuffer = NULL;
            if (it != m_NodeBufferMap.end())
            {
                vertexBuffer = it->second;
                if (vertexBuffer->m_Buffer != VK_NULL_HANDLE)
                {
                    vkDestroyBuffer(m_VulkanRenderer->m_hDevice, vertexBuffer->m_Buffer, NULL);
                    vkFreeMemory(m_VulkanRenderer->m_hDevice, vertexBuffer->m_Memory, NULL);
                }
            }
            else
            {
                vertexBuffer = new VulkanBuffer();
                vertexBuffer->m_DataSize = dataSize;
                vertexBuffer->m_MemoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
                m_NodeBufferMap[modelList.at(j)] = vertexBuffer;
            }

            const VkPhysicalDeviceMemoryProperties& memProp = m_VulkanRenderer->m_physicalDeviceInfo.memProp;
            VulkanHelper::CreateBuffer(m_VulkanRenderer->m_hDevice, memProp, *vertexBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, rectVertices);
        }
    }
#endif
}
#endif

void VulkanRectangleMultiDrawScheme::UpdateDirtyMultiDrawData()
{
#ifdef ENGINE_METAL
#else
    for (int pipelineIdx = 0; pipelineIdx < PIPELINE_COUNT; ++pipelineIdx)
    {
        NodeVector& modelList = m_PipelineTypeModelVector[pipelineIdx];
        const int modelSize = modelList.size();
        if (!modelSize) continue;

        Vertex rectVertices[6];
        memcpy(rectVertices, s_RectFilledVertices, sizeof(Vertex) * 6);
        uint32_t dataSize = sizeof(rectVertices);
        uint32_t dataStride = sizeof(rectVertices[0]);
        const int vertexCount = dataSize / dataStride;
        for (int j = 0; j < modelSize; j++)
        {
            if (modelList.at(j)->GetDirtyType() == DIRTY_TYPE::ATTRIBUTES || modelList.at(j)->GetDirtyType() == DIRTY_TYPE::POSITION)
            {
                qDebug() << "Dirty update.....";
                glm::mat4 parentTransform = modelList.at(j)->GetAbsoluteTransformationComputation();

                for (int i = 0; i < vertexCount; ++i)
                {
                    glm::vec4 pos(s_RectFilledVertices[i].m_Position, 1.0);
                    pos.x = pos.x * modelList.at(j)->GetBoundedRegion().m_Dimension.x;
                    pos.y = pos.y * modelList.at(j)->GetBoundedRegion().m_Dimension.y;

                    pos = parentTransform * pos;
                    //std::cout << m_Name.toStdString() << "=+ x:" << pos.x << ", y:" << pos.y << ", z:" << pos.z << endl;

                    rectVertices[i].m_Position.x = pos.x;
                    rectVertices[i].m_Position.y = pos.y;
                    rectVertices[i].m_Position.z = pos.z;
                }

                std::map<Node*, VulkanBuffer*>::iterator it = m_NodeBufferMap.find(modelList.at(j));
                VulkanBuffer* vertexBuffer = NULL;
                if (it != m_NodeBufferMap.end())
                {
                    vertexBuffer = it->second;
                    if (vertexBuffer->m_Buffer != VK_NULL_HANDLE)
                    {
                        vkDestroyBuffer(m_VulkanRenderer->m_hDevice, vertexBuffer->m_Buffer, NULL);
                        vkFreeMemory(m_VulkanRenderer->m_hDevice, vertexBuffer->m_Memory, NULL);
                    }
                }
                else
                {
                    assert(false);
                }
                // TODO: Don't reallocate, create command buffers and copy into device memory.
                const VkPhysicalDeviceMemoryProperties& memProp = m_VulkanRenderer->m_physicalDeviceInfo.memProp;
                VulkanHelper::CreateBuffer(m_VulkanRenderer->m_hDevice, memProp, *vertexBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, rectVertices);
            }
        }
    }
#endif
}

void* VulkanRectangleMultiDrawScheme::GetPipeline(int p_PipelineType)
{
//    if (!m_GraphicsPipelineMap.contains(p_PipelineType))
//    {
//        switch (static_cast<MESH_GRAPHICS_PIPELINES>(p_PipelineType))
//        {
//            case PIPELINE_FILLED:
//                CreateRectFillPipeline();
//                break;

//            case PIPELINE_OUTLINE:
//                CreateRectOutlinePipeline();
//                break;

//            default:
//                return NULL;
//        }
//    }

//    return reinterpret_cast<MTLRenderPipelineState*>(m_GraphicsPipelineMap[p_PipelineType]);
    return NULL;
}

void VulkanRectangleMultiDrawScheme::Render(void* p_CommandBuffer)
{
    VkCommandBuffer cmdBuffer = static_cast<VkCommandBuffer>(p_CommandBuffer);

    for (int pipelineIdx = 0; pipelineIdx < RECTANGLE_GRAPHICS_PIPELINES::PIPELINE_COUNT; pipelineIdx++)
    {
        NodeVector& m_ModelList = m_PipelineTypeModelVector[pipelineIdx];
        const int modelSize = m_ModelList.size();
        if (!modelSize) continue;

        VkPipelineLayout graphicsPipelineLayout = VK_NULL_HANDLE;
        VkPipeline       graphicsPipeline = VK_NULL_HANDLE;
        if (pipelineIdx == PIPELINE_FILLED)
        {
            if (m_GraphicsPipelineMap.contains(PIPELINE_FILLED))
            {

                graphicsPipeline = static_cast<VkPipeline>(static_cast<QPair<void*/*VkPipeline*/, void*/*VkPipelineLayout*/>*>(m_GraphicsPipelineMap[PIPELINE_FILLED])->first);
                graphicsPipelineLayout = static_cast<VkPipelineLayout>(static_cast<QPair<void*/*VkPipeline*/, void*/*VkPipelineLayout*/>*>(m_GraphicsPipelineMap[PIPELINE_FILLED])->second);
            }
        }
        else if (pipelineIdx == PIPELINE_OUTLINE)
        {
            if (m_GraphicsPipelineMap.contains(PIPELINE_OUTLINE))
            {
                graphicsPipeline = static_cast<VkPipeline>(static_cast<QPair<void*/*VkPipeline*/, void*/*VkPipelineLayout*/>*>(m_GraphicsPipelineMap[PIPELINE_OUTLINE])->first);
                graphicsPipelineLayout = static_cast<VkPipelineLayout>(static_cast<QPair<void*/*VkPipeline*/, void*/*VkPipelineLayout*/>*>(m_GraphicsPipelineMap[PIPELINE_OUTLINE])->second);
            }
        }
        else
        {
            assert(false);
        }

        vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelineLayout, 0, 1, m_DescriptorSet->descriptorSet.data(), 0, NULL);
        vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

        for (int j = 0; j < modelSize; j++)
        {
            Rectangl* model = (static_cast<Rectangl*>(m_ModelList.at(j)));
            if (!model || !model->GetVisible()) continue;

            //////////////////////////////////////////////////////////////////////////////////
            struct pushConst
            {
                glm::vec4 inColor;
                glm::mat4 modelMatrix;
            }PC;


            PC.inColor = model->GetColor();
            //PC.inColor.a = 0.5;
            //PC.modelMatrix = /*(*model->GetScene()->GetProjection()) * (*model->GetScene()->GetView()) */ model->GetAbsoluteTransformation();
            //PC.modelMatrix = (*GetProjection()) * (*GetView()) model->GetModelTransformation();// GetAbsoluteTransformation();

            // Check if number of push constants does not exceed the allowed size
            uint32_t maxPushContantSize = m_VulkanRenderer->m_physicalDeviceInfo.prop.limits.maxPushConstantsSize;
            if (static_cast<uint32_t>(sizeof(PC)) > maxPushContantSize)
            {
                printf("Push constand size is greater than expected max allow size is %d", maxPushContantSize);
                assert(0);
            }

            vkCmdPushConstants(cmdBuffer, graphicsPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PC), &PC);
            ////////////////////////////////////////////////////////////////////////////////

            int vertexCount = 0;
            if (model->GetDrawType() == Rectangl::FILLED)
            {
                vertexCount = sizeof(s_RectFilledVertices) / sizeof(Vertex);
            }
            else if (model->GetDrawType() == Rectangl::OUTLINE)
            {
                vertexCount = sizeof(s_RectOutlineVertices) / sizeof(Vertex);
            }

            std::map<Node*, VulkanBuffer*>::iterator it = m_NodeBufferMap.find(model);
            if (it == m_NodeBufferMap.end()) continue;
#ifdef ENABLE_DYNAMIC_STATE
            VkViewport viewport;
            viewport.height     = (float)m_VulkanRenderer->m_swapChainExtent.height;
            viewport.width      = (float)m_VulkanRenderer->m_swapChainExtent.width;
            viewport.minDepth   = (float) 0.0f;
            viewport.maxDepth   = (float) 1.0f;
            viewport.x          = 0;
            viewport.y          = 0;
            vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
#endif
            const VkDeviceSize offsets[1] = { 0 };
            vkCmdBindVertexBuffers(cmdBuffer, VERTEX_BUFFER_BIND_IDX, 1, &it->second->m_Buffer, offsets);
            vkCmdDraw(cmdBuffer, vertexCount, /*INSTANCE_COUNT*/1, 0, 0);

            model->SetDirtyType(DIRTY_TYPE::NONE); // Doublce check if this dirty is needed
        }

    }
}
