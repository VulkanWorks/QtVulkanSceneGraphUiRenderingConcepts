#include "VulkanCircleMultiDrawScheme.h"

#include "../../../../UIGraphicsDemo/source/Circle/Circle.h"

#include "CircleDescriptorSet.h"
#include "CircleGeometry.h"
#include "CircleShaderTypes.h"

VulkanCircleMultiDrawScheme::VulkanCircleMultiDrawScheme(BaseRenderer* p_Renderer)
    : RenderSchemeFactory(p_Renderer)
    , m_VulkanRenderer(static_cast<VulkanApp*>(p_Renderer))
{
    assert(p_Renderer);

    /////////////////

#ifdef DEPRECATE_PIPELINE_SETUP
    m_DescriptorSet = std::make_shared<CircleDescriptorSet>(m_Renderer);

    CreateVertexLayoutBinding();

    CreateGraphicsPipeline();

    // Build the push constants
    createPushConstants();
#else
    m_DescriptorSet = NULL;
#endif
}

VulkanCircleMultiDrawScheme::~VulkanCircleMultiDrawScheme()
{
    for (int pipelineIdx = 0; pipelineIdx < CIRCLE_GRAPHICS_PIPELINES::PIPELINE_COUNT; pipelineIdx++)
    {
        NodeVector& modelList = m_PipelineTypeModelVector[pipelineIdx];
        const int modelSize = modelList.size();
        if (!modelSize) continue;

        // Destroy Vertex Buffer
        for (int j = 0; j < modelSize; j++)
        {
            if (modelList.at(j)->GetRefShapeType() == SHAPE::SHAPE_CIRCLE_MULTIDRAW)
            {
                Circle* model = (static_cast<Circle*>(modelList.at(j)));
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

    CircleDescriptorSet::UniformBufferObj* UniformBuffer = m_DescriptorSet->UniformBuffer;
    vkUnmapMemory(m_VulkanRenderer->m_hDevice, UniformBuffer->m_BufObj.m_Memory);
    vkDestroyBuffer(m_VulkanRenderer->m_hDevice, UniformBuffer->m_BufObj.m_Buffer, NULL);
    vkFreeMemory(m_VulkanRenderer->m_hDevice, UniformBuffer->m_BufObj.m_Memory, NULL);
}

//void VulkanCircleMultiDrawScheme::Setup()
//{
//#ifndef DEPRECATE_PIPELINE_SETUP
//    m_DescriptorSet = std::make_shared<CircleDescriptorSet>(m_Renderer);

//    CreateVertexLayoutBinding();

//    CreateGraphicsPipeline();

//    PrepareMultiDrawData();

//    // Build the push constants
//    createPushConstants();
//#endif
//}

//void VulkanCircleMultiDrawScheme::Update()
//{
//    CircleDescriptorSet::UniformBufferObj* UniformBuffer = m_DescriptorSet->UniformBuffer;
//    VulkanHelper::WriteMemory(m_VulkanRenderer->m_hDevice,
//        UniformBuffer->m_MappedMemory,
//        UniformBuffer->m_MappedRange,
//        UniformBuffer->m_BufObj.m_MemoryFlags,
//        &m_ProjectViewMatrix, sizeof(m_ProjectViewMatrix));
//}

void VulkanCircleMultiDrawScheme::Update()
{
    RectangleDescriptorSet::UniformBufferObj* UniformBuffer = m_DescriptorSet->UniformBuffer;
    VulkanHelper::WriteMemory(m_VulkanRenderer->m_hDevice,
        UniformBuffer->m_MappedMemory,
        UniformBuffer->m_MappedRange,
        UniformBuffer->m_BufObj.m_MemoryFlags,
                  &m_ProjectViewMatrix, sizeof(m_ProjectViewMatrix));

    UpdateDirtyMultiDrawData();
}

void VulkanCircleMultiDrawScheme::ResizeWindow(/*void* p_CommandBuffer*/)
{
#ifdef RECREATE_PSO
    CreateGraphicsPipeline(true);
//    Render(p_CommandBuffer);
#endif
}

void VulkanCircleMultiDrawScheme::CreateCircleOutlinePipeline()
{
    VkShaderModule vertShader                           = VulkanHelper::CreateShaderFromQRCResource(m_VulkanRenderer->m_hDevice, "://CircleMultiDrawSchemeVert.spv");
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = VulkanHelper::PipelineShaderStageCreateInfo();
        vertShaderStageInfo.stage                       = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module                      = vertShader;
        vertShaderStageInfo.pName                       = "main";

    VkShaderModule fragShader                           = VulkanHelper::CreateShaderFromQRCResource(m_VulkanRenderer->m_hDevice, "://CircleMultiDrawSchemeFrag.spv");
    VkPipelineShaderStageCreateInfo fragShaderStageInfo = VulkanHelper::PipelineShaderStageCreateInfo();
        fragShaderStageInfo.stage                       = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module                      = fragShader;
        fragShaderStageInfo.pName                       = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

//#ifdef ENABLE_DYNAMIC_STATE
//    VkDynamicState dynamicStateEnables[VK_DYNAMIC_STATE_RANGE_SIZE];
//    memset(dynamicStateEnables, 0, sizeof dynamicStateEnables);

//    // Specify the dynamic state information to pipeline through
//    // VkPipelineDynamicStateCreateInfo control structure.
//    VkPipelineDynamicStateCreateInfo dynamicState = {};
//    dynamicState.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
//    dynamicState.pNext             = NULL;
//    dynamicState.pDynamicStates    = dynamicStateEnables;
//    dynamicState.dynamicStateCount = 0;
//#endif

#ifdef ENABLE_DYNAMIC_STATE
    VkDynamicState dynamicStateEnables[VK_DYNAMIC_STATE_RANGE_SIZE];
    memset(dynamicStateEnables, 0, sizeof dynamicStateEnables);
    VkPipelineDynamicStateCreateInfo dynamicState = VulkanHelper::PipelineDynamicStateCreateInfo();
        dynamicState.pDynamicStates    = dynamicStateEnables;
#endif

    VkPipelineVertexInputStateCreateInfo vertexInputInfo    = VulkanHelper::PipelineVertexInputStateCreateInfo();
        vertexInputInfo.vertexBindingDescriptionCount       = (uint32_t)m_VertexInputBinding[PIPELINE_OUTLINE].size();
        vertexInputInfo.pVertexBindingDescriptions          = m_VertexInputBinding[PIPELINE_OUTLINE].data();
        vertexInputInfo.vertexAttributeDescriptionCount     = (uint32_t)m_VertexInputAttribute[PIPELINE_OUTLINE].size();
        vertexInputInfo.pVertexAttributeDescriptions        = m_VertexInputAttribute[PIPELINE_OUTLINE].data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly    = VulkanHelper::PipelineInputAssemblyStateCreateInfo();
        inputAssembly.topology                              = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;

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
        colorBlending.pAttachments                               = &colorBlendAttachment;

    // Create pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo   = VulkanHelper::PipelineLayoutCreateInfo();
        pipelineLayoutInfo.setLayoutCount = (uint32_t)m_DescriptorSet->descLayout.size();
        pipelineLayoutInfo.pSetLayouts = m_DescriptorSet->descLayout.data();

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
    m_GraphicsPipelineMap[PIPELINE_OUTLINE] = graphicsPipelineAndLayoutPair;

    vkDestroyShaderModule(m_VulkanRenderer->m_hDevice, fragShader, nullptr);
    vkDestroyShaderModule(m_VulkanRenderer->m_hDevice, vertShader, nullptr);
}

void VulkanCircleMultiDrawScheme::CreateCircleFillPipeline()
{
    VkShaderModule vertShader                           = VulkanHelper::CreateShaderFromQRCResource(m_VulkanRenderer->m_hDevice, "://CircleMultiDrawSchemeVert.spv");
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = VulkanHelper::PipelineShaderStageCreateInfo();
        vertShaderStageInfo.stage                       = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module                      = vertShader;
        vertShaderStageInfo.pName                       = "main";

    VkShaderModule fragShader                           = VulkanHelper::CreateShaderFromQRCResource(m_VulkanRenderer->m_hDevice, "://CircleMultiDrawSchemeFrag.spv");
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
        vertexInputInfo.vertexBindingDescriptionCount       = (uint32_t) m_VertexInputBinding[PIPELINE_FILLED].size();
        vertexInputInfo.pVertexBindingDescriptions          = m_VertexInputBinding[PIPELINE_FILLED].data();
        vertexInputInfo.vertexAttributeDescriptionCount     = (uint32_t) m_VertexInputAttribute[PIPELINE_FILLED].size();
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

    // Setup the push constant range
    const unsigned pushConstantRangeCount = 1;
    VkPushConstantRange pushConstantRanges[pushConstantRangeCount] = {};
        pushConstantRanges[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushConstantRanges[0].offset = 0;
        pushConstantRanges[0].size = 16 + sizeof(glm::mat4);

    // Create pipeline layout
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
        pipelineInfo.stageCount                     = 2;
        pipelineInfo.pStages                        = shaderStages;
        pipelineInfo.pVertexInputState              = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState            = &inputAssembly;
        pipelineInfo.pViewportState                 = &viewportState;
        pipelineInfo.pRasterizationState            = &rasterizer;
        pipelineInfo.pMultisampleState              = &multisampling;
        pipelineInfo.pColorBlendState               = &colorBlending;
#ifdef ENABLE_DYNAMIC_STATE
        pipelineInfo.pDynamicState                  = &dynamicState;
#endif
        pipelineInfo.layout                         = graphicsPipelineLayout;
        pipelineInfo.renderPass                     = m_VulkanRenderer->m_hRenderPass;
        pipelineInfo.subpass                        = 0;
        pipelineInfo.basePipelineHandle             = VK_NULL_HANDLE;
        pipelineInfo.pDepthStencilState             = &depthStencilStateInfo;

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

void VulkanCircleMultiDrawScheme::CreateGraphicsPipeline(bool p_ClearGraphicsPipelineMap)
{
    if (p_ClearGraphicsPipelineMap)
    {
        const VkDevice& device = m_VulkanRenderer->m_hDevice;
        QMap<int, void*>::iterator i;
        for (i = m_GraphicsPipelineMap.begin(); i != m_GraphicsPipelineMap.end(); ++i)
        {
            vkDestroyPipeline(m_VulkanRenderer->m_hDevice, static_cast<VkPipeline>(static_cast<QPair<void*/*VkPipeline*/, void*/*VkPipelineLayout*/>*>(i.value())->first), nullptr);
            vkDestroyPipelineLayout(device, static_cast<VkPipelineLayout>(static_cast<QPair<void*/*VkPipeline*/, void*/*VkPipelineLayout*/>*>(i.value())->second), nullptr);
        }

        m_GraphicsPipelineMap.clear();
    }

    VkPipelineLayout graphicsPipelineLayout = VK_NULL_HANDLE;
    VkPipeline       graphicsPipeline = VK_NULL_HANDLE;
    if (m_GraphicsPipelineMap.contains(PIPELINE_FILLED))
    {
        graphicsPipeline = static_cast<VkPipeline>(static_cast<QPair<void*/*VkPipeline*/, void*/*VkPipelineLayout*/>*>(m_GraphicsPipelineMap[PIPELINE_FILLED])->first);
        graphicsPipelineLayout = static_cast<VkPipelineLayout>(static_cast<QPair<void*/*VkPipeline*/, void*/*VkPipelineLayout*/>*>(m_GraphicsPipelineMap[PIPELINE_FILLED])->second);
        return;
    }

    CreateCircleFillPipeline();
    CreateCircleOutlinePipeline();
}

void VulkanCircleMultiDrawScheme::createPushConstants()
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

void VulkanCircleMultiDrawScheme::CreateVertexLayoutBinding()
{
    for (int pipelineIdx = 0; pipelineIdx < CIRCLE_GRAPHICS_PIPELINES::PIPELINE_COUNT; pipelineIdx++)
    {
        if (pipelineIdx == PIPELINE_FILLED)
        {
            m_VertexInputBinding[pipelineIdx].resize(1);   // 0 for position and 1 for color
            m_VertexInputAttribute[pipelineIdx].resize(3); // Why 2 = 2(for position and color

            // Indicates the rate at which the information will be
            // injected for vertex input.
            m_VertexInputBinding[pipelineIdx][0].binding = VERTEX_BUFFER_BIND_IDX;
            m_VertexInputBinding[pipelineIdx][0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            m_VertexInputBinding[pipelineIdx][0].stride = sizeof(CircleVertex);

            // The VkVertexInputAttribute interpreting the data.
            m_VertexInputAttribute[pipelineIdx][0].binding = VERTEX_BUFFER_BIND_IDX;
            m_VertexInputAttribute[pipelineIdx][0].location = 0;
            m_VertexInputAttribute[pipelineIdx][0].format = VK_FORMAT_R32G32B32_SFLOAT;
            m_VertexInputAttribute[pipelineIdx][0].offset = offsetof(CircleVertex, m_Position);

            m_VertexInputAttribute[pipelineIdx][1].binding = VERTEX_BUFFER_BIND_IDX;
            m_VertexInputAttribute[pipelineIdx][1].location = 1;
            m_VertexInputAttribute[pipelineIdx][1].format = VK_FORMAT_R32G32_SFLOAT;
            m_VertexInputAttribute[pipelineIdx][1].offset = offsetof(CircleVertex, m_TexCoord);

            m_VertexInputAttribute[pipelineIdx][2].binding = VERTEX_BUFFER_BIND_IDX;
            m_VertexInputAttribute[pipelineIdx][2].location = 2;
            m_VertexInputAttribute[pipelineIdx][2].format = VK_FORMAT_R32_UINT;
            m_VertexInputAttribute[pipelineIdx][2].offset = offsetof(CircleVertex, m_DrawType);
        }
        else if (pipelineIdx == PIPELINE_OUTLINE)
        {
            m_VertexInputBinding[pipelineIdx].resize(1);   // 0 for position and 1 for color
            m_VertexInputAttribute[pipelineIdx].resize(2); // Why 2 = 2(for position and color

            // Indicates the rate at which the information will be
            // injected for vertex input.
            m_VertexInputBinding[pipelineIdx][0].binding = VERTEX_BUFFER_BIND_IDX;
            m_VertexInputBinding[pipelineIdx][0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            m_VertexInputBinding[pipelineIdx][0].stride = sizeof(CircleVertex);

            // The VkVertexInputAttribute interpreting the data.
            m_VertexInputAttribute[pipelineIdx][0].binding = VERTEX_BUFFER_BIND_IDX;
            m_VertexInputAttribute[pipelineIdx][0].location = 0;
            m_VertexInputAttribute[pipelineIdx][0].format = VK_FORMAT_R32G32B32_SFLOAT;
            m_VertexInputAttribute[pipelineIdx][0].offset = offsetof(CircleVertex, m_Position);

            m_VertexInputAttribute[pipelineIdx][1].binding = VERTEX_BUFFER_BIND_IDX;
            m_VertexInputAttribute[pipelineIdx][1].location = 1;
            m_VertexInputAttribute[pipelineIdx][1].format = VK_FORMAT_R32G32_SFLOAT;
            m_VertexInputAttribute[pipelineIdx][1].offset = offsetof(CircleVertex, m_TexCoord);
        }
    }
}

void VulkanCircleMultiDrawScheme::UpdatePipelineNodeList(Node *p_Item)
{
    Circle* circle = dynamic_cast<Circle*>(p_Item);
    assert(circle);

    // Note: Based on the draw type push the model in respective pipelines
    // Keep the draw type loose couple with the pipeline type,
    // they may be in one-to-one correspondence but that is not necessary.
    switch (circle->GetDrawType())
    {
    case Circle::FILLED:
        m_PipelineTypeModelVector[PIPELINE_FILLED].push_back(p_Item);
        break;

    case Circle::OUTLINE:
        m_PipelineTypeModelVector[PIPELINE_OUTLINE].push_back(p_Item);
        break;

    case Circle::ROUNDED:
        // TODO
        break;

    default:
        break;
    }

#ifdef DEPRECATE_PIPELINE_SETUP
    PrepareMultiDrawData(circle);
#endif
}

void VulkanCircleMultiDrawScheme::RemovePipelineNodeList(Node* p_Model)
{
    Q_UNUSED(p_Model);
}

#ifdef DEPRECATE_PIPELINE_SETUP
void VulkanCircleMultiDrawScheme::PrepareMultiDrawData(Circle* p_Circle)
{
    if (!p_Circle) return;

#ifdef ENGINE_METAL
#else
    CircleVertex rectVertices[6];
    memcpy(rectVertices, circleFilledVertices, sizeof(CircleVertex) * 6);
    uint32_t dataSize = sizeof(rectVertices);
    uint32_t dataStride = sizeof(rectVertices[0]);
    const int vertexCount = dataSize / dataStride;
    glm::mat4 parentTransform = p_Circle->GetAbsoluteTransformationComputation();

    for (int i = 0; i < vertexCount; ++i)
    {
        glm::vec4 pos(circleFilledVertices[i].m_Position, 1.0);
        pos.x = pos.x * p_Circle->GetBoundedRegion().m_Dimension.x;
        pos.y = pos.y * p_Circle->GetBoundedRegion().m_Dimension.y;

        pos = parentTransform * pos;
        //std::cout << m_Name.toStdString() << "=+ x:" << pos.x << ", y:" << pos.y << ", z:" << pos.z << endl;

        rectVertices[i].m_Position.x = pos.x;
        rectVertices[i].m_Position.y = pos.y;
        rectVertices[i].m_Position.z = pos.z;
    }

    std::map<Node*, VulkanBuffer*>::iterator it = m_NodeBufferMap.find(p_Circle);
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
        m_NodeBufferMap[p_Circle] = vertexBuffer;
    }

    const VkPhysicalDeviceMemoryProperties& memProp = m_VulkanRenderer->m_physicalDeviceInfo.memProp;
    VulkanHelper::CreateBuffer(m_VulkanRenderer->m_hDevice, memProp, *vertexBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, rectVertices);
#endif
}
#else
void VulkanCircleMultiDrawScheme::PrepareMultiDrawData()
{
#ifdef ENGINE_METAL
#else
    for (int pipelineIdx = 0; pipelineIdx < PIPELINE_COUNT; ++pipelineIdx)
    {
        NodeVector& modelList = m_PipelineTypeModelVector[pipelineIdx];
        const int modelSize = modelList.size();
        if (!modelSize) continue;

        CircleVertex rectVertices[6];
        memcpy(rectVertices, circleFilledVertices, sizeof(CircleVertex) * 6);
        uint32_t dataSize = sizeof(rectVertices);
        uint32_t dataStride = sizeof(rectVertices[0]);
        const int vertexCount = dataSize / dataStride;
        for (int j = 0; j < modelSize; j++)
        {
            glm::mat4 parentTransform = modelList.at(j)->GetAbsoluteTransformationComputation();

            for (int i = 0; i < vertexCount; ++i)
            {
                glm::vec4 pos(circleFilledVertices[i].m_Position, 1.0);
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

void VulkanCircleMultiDrawScheme::UpdateDirtyMultiDrawData()
{
#ifdef ENGINE_METAL
#else
    for (int pipelineIdx = 0; pipelineIdx < PIPELINE_COUNT; ++pipelineIdx)
    {
        NodeVector& modelList = m_PipelineTypeModelVector[pipelineIdx];
        const int modelSize = modelList.size();
        if (!modelSize) continue;

        CircleVertex rectVertices[6];
        memcpy(rectVertices, circleFilledVertices, sizeof(CircleVertex) * 6);
        uint32_t dataSize = sizeof(rectVertices);
        uint32_t dataStride = sizeof(rectVertices[0]);
        const int vertexCount = dataSize / dataStride;
        for (int j = 0; j < modelSize; j++)
        {
            if (modelList.at(j)->GetDirtyType() == DIRTY_TYPE::ATTRIBUTES || modelList.at(j)->GetDirtyType() == DIRTY_TYPE::POSITION)
            {
                glm::mat4 parentTransform = modelList.at(j)->GetAbsoluteTransformationComputation();

                for (int i = 0; i < vertexCount; ++i)
                {
                    glm::vec4 pos(circleFilledVertices[i].m_Position, 1.0);
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

void VulkanCircleMultiDrawScheme::Render(void* p_CommandBuffer)
{
    VkCommandBuffer cmdBuffer = static_cast<VkCommandBuffer>(p_CommandBuffer);
    for (int pipelineIdx = 0; pipelineIdx < CIRCLE_GRAPHICS_PIPELINES::PIPELINE_COUNT; pipelineIdx++)
    {
        NodeVector& m_ModelList = m_PipelineTypeModelVector[pipelineIdx];
        const int modelSize = m_ModelList.size();
        if (!modelSize) continue;

        VkPipelineLayout graphicsPipelineLayout = VK_NULL_HANDLE;
        VkPipeline       graphicsPipeline = VK_NULL_HANDLE;
        // Parminder: m_GraphicsPipelineMap first is changed from QString to int, reconsider the below if else check. we don't need double if checks any more
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
            Circle* model = (static_cast<Circle*>(m_ModelList.at(j)));
            if (!model || !model->GetVisible()) continue;
            //////////////////////////////////////////////////////////////////////////////////
            struct pushConst
            {
                glm::vec4 inColor;
                glm::mat4 modelMatrix;
            }PC;

            PC.inColor = model->GetColor();
            //PC.modelMatrix = /*(*model->GetScene()->GetProjection()) * (*model->GetScene()->GetView()) */ model->GetAbsoluteTransformation();
            //PC.modelMatrix = (*GetProjection()) * (*GetView()) model->GetModelTransformation();// GetAbsoluteTransformation();

            // Check if number of push constants does not exceed the allowed size
            uint32_t maxPushContantSize = m_VulkanRenderer->m_physicalDeviceInfo.prop.limits.maxPushConstantsSize;
            if (static_cast<uint32_t>(sizeof(PC)) > maxPushContantSize)
            {
                printf("Push constand size is greater than expected, max allow size is %d", maxPushContantSize);
                assert(0);
            }

            vkCmdPushConstants(cmdBuffer, graphicsPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PC), &PC);
            ////////////////////////////////////////////////////////////////////////////////

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

            if (model->GetDrawType() == Circle::FILLED)
            {
                // Specify vertex buffer information
                const VkDeviceSize offsets[1] = { 0 };
                std::map<Node*, VulkanBuffer*>::iterator it = m_NodeBufferMap.find(model);
                if (it == m_NodeBufferMap.end()) continue;
                vkCmdBindVertexBuffers(cmdBuffer, VERTEX_BUFFER_BIND_IDX, 1, &it->second->m_Buffer, offsets);

                // Draw the Cube
                const int vertexCount = sizeof(circleFilledVertices) / sizeof(CircleVertex);
                vkCmdDraw(cmdBuffer, vertexCount, /*INSTANCE_COUNT*/1, 0, 0);
            }
            else if (model->GetDrawType() == Circle::OUTLINE)
            {
                // Specify vertex buffer information
                std::map<Node*, VulkanBuffer*>::iterator it = m_NodeBufferMap.find(model);
                if (it == m_NodeBufferMap.end()) continue;
                const VkDeviceSize offsets[1] = { 0 };
                vkCmdBindVertexBuffers(cmdBuffer, VERTEX_BUFFER_BIND_IDX, 1, &it->second->m_Buffer, offsets);

                // Draw the Cube
                const int vertexCount = sizeof(rectOutlineVertices) / sizeof(CircleVertex);
                vkCmdDraw(cmdBuffer, vertexCount, /*INSTANCE_COUNT*/1, 0, 0);

                model->SetDirtyType(DIRTY_TYPE::NONE); // Doublce check if this dirty is needed
            }
        }

    }
}
