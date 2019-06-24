#include "VulkanRectangleInstancingScheme.h"

#include "../../../../UIGraphicsDemo/source/Rectangle/Rect.h"

#include "RectangleGeometry.h"
#include "RectangleShaderTypes.h"

static float s_Time = 0.0;
static bool s_EnableDirtyTest = false;

VulkanRectangleInstancingScheme::VulkanRectangleInstancingScheme(BaseRenderer* p_Renderer)
    : RenderSchemeFactory(p_Renderer)
    , m_VulkanRenderer(static_cast<VulkanApp*>(p_Renderer))
{
    assert(m_VulkanRenderer);
    m_DescriptorSet = NULL;

    memset(&m_VertexBuffer, 0, sizeof(VulkanBuffer) * PIPELINE_COUNT);
    memset(&m_InstanceBuffer, 0, sizeof(VulkanBuffer) * PIPELINE_COUNT);
    memset(&m_OldInstanceDataSize, 0, sizeof(int) * PIPELINE_COUNT);
}

VulkanRectangleInstancingScheme::~VulkanRectangleInstancingScheme()
{
    VkPipelineLayout graphicsPipelineLayout = VK_NULL_HANDLE;
    VkPipeline       graphicsPipeline = VK_NULL_HANDLE;
    if (m_GraphicsPipelineMap.contains(PIPELINE_FILLED))
    {
        graphicsPipeline = static_cast<VkPipeline>(static_cast<QPair<void*/*VkPipeline*/, void*/*VkPipelineLayout*/>*>(m_GraphicsPipelineMap[PIPELINE_FILLED])->first);
        vkDestroyPipeline(m_VulkanRenderer->m_hDevice, graphicsPipeline, nullptr);

        //// Destroy descriptors
        //for (int i = 0; i < descLayout.size(); i++) {
        //       vkDestroyDescriptorSetLayout(m_VulkanRenderer->m_hDevice, descLayout[i], NULL);
        //}
        //descLayout.clear();
        graphicsPipelineLayout = static_cast<VkPipelineLayout>(static_cast<QPair<void*/*VkPipeline*/, void*/*VkPipelineLayout*/>*>(m_GraphicsPipelineMap[PIPELINE_FILLED])->second);
        vkDestroyPipelineLayout(m_VulkanRenderer->m_hDevice, graphicsPipelineLayout, nullptr);
    }

    for (int pipelineIdx = 0; pipelineIdx < RECTANGLE_GRAPHICS_PIPELINES::PIPELINE_COUNT; pipelineIdx++)
    {
        // Destroy Vertex Buffer
        vkDestroyBuffer(m_VulkanRenderer->m_hDevice, m_VertexBuffer[pipelineIdx].m_Buffer, NULL);
        vkFreeMemory(m_VulkanRenderer->m_hDevice, m_VertexBuffer[pipelineIdx].m_Memory, NULL);
    }

    //vkFreeDescriptorSets(m_VulkanRenderer->m_hDevice, descriptorPool, (uint32_t)descriptorSet.size(), &descriptorSet[0]);
    //vkDestroyDescriptorPool(m_VulkanRenderer->m_hDevice, descriptorPool, NULL);

    vkUnmapMemory(m_VulkanRenderer->m_hDevice, m_DescriptorSet->UniformBuffer->m_BufObj.m_Memory);
    vkDestroyBuffer(m_VulkanRenderer->m_hDevice, m_DescriptorSet->UniformBuffer->m_BufObj.m_Buffer, NULL);
    vkFreeMemory(m_VulkanRenderer->m_hDevice, m_DescriptorSet->UniformBuffer->m_BufObj.m_Memory, NULL);
}

void VulkanRectangleInstancingScheme::Setup()
{
    m_DescriptorSet = std::make_shared<RectangleDescriptorSet>(m_VulkanRenderer);

    CreateVertexBuffer();

    CreateGraphicsPipeline();

    //m_VulkanRenderer->CreateCommandBuffers(); // Create command buffers

    PrepareInstanceData();  // Important: Object's GPU allocation held here

    RecordCommandBuffer();
}

void VulkanRectangleInstancingScheme::Update()
{
    if (s_EnableDirtyTest)
    {
        s_Time += 0.01;
        if (s_Time > 1.0)
            s_Time = 0.0;
    }

    RectangleDescriptorSet::UBORect ubo;
    ubo.m_ProjViewMat = m_ProjectViewMatrix;
    ubo.m_Time = s_Time;
    ubo.m_DirtyTest = s_EnableDirtyTest;

    VulkanHelper::WriteMemory(m_VulkanRenderer->m_hDevice,
        m_DescriptorSet->UniformBuffer->m_MappedMemory,
        m_DescriptorSet->UniformBuffer->m_MappedRange,
        m_DescriptorSet->UniformBuffer->m_BufObj.m_MemoryFlags,
        &ubo, sizeof(RectangleDescriptorSet::UBORect));

    UpdateDirtyInstanceData();
}

void VulkanRectangleInstancingScheme::ResizeWindow(/*void* p_CommandBuffer*/)
{
#ifdef RECREATE_PSO
    CreateGraphicsPipeline(true);
#endif
}

void VulkanRectangleInstancingScheme::CreateRectOutlinePipeline()
{
    // Compile the vertex shader
//#ifdef _WIN32
//    VkShaderModule vertShader = VulkanHelper::CreateShader(m_VulkanRenderer->m_hDevice, "F:/Development/GithubRepos/RenderConcept/UIRenderingConcepts/engine/vulkan/implementation/Shaders/RectangleInstancingSchemeVert.spv"); // Relative path to binary output dir
//                                                                                                                              // Setup the vertex shader stage create info structures
//#elif __APPLE__
    VkShaderModule vertShader = VulkanHelper::CreateShaderFromQRCResource(m_VulkanRenderer->m_hDevice, "://RectangleInstancingSchemeOutlineVert.spv");
//#endif

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShader;
    vertShaderStageInfo.pName = "main";

    // Compile the fragment shader
//#ifdef _WIN32
//    VkShaderModule fragShader = VulkanHelper::CreateShader(m_VulkanRenderer->m_hDevice, "F:/Development/GithubRepos/RenderConcept/UIRenderingConcepts/engine/vulkan/implementation/Shaders/RectangleInstancingSchemeFrag.spv"); // Relative path to binary output dir
//                                                                                                                              // Setup the fragment shader stage create info structures
//#elif __APPLE__
    VkShaderModule fragShader = VulkanHelper::CreateShaderFromQRCResource(m_VulkanRenderer->m_hDevice, "://RectangleInstancingSchemeOutlineFrag.spv");
//#endif
    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShader;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

#ifdef ENABLE_DYNAMIC_STATE
    VkDynamicState dynamicStateEnables[VK_DYNAMIC_STATE_RANGE_SIZE];
    memset(dynamicStateEnables, 0, sizeof dynamicStateEnables);

    // Specify the dynamic state information to pipeline through
    // VkPipelineDynamicStateCreateInfo control structure.
    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pNext             = NULL;
    dynamicState.pDynamicStates    = dynamicStateEnables;
    dynamicState.dynamicStateCount = 0;
#endif

    // Setup the vertex input
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    // Check the size where every necessrry
    vertexInputInfo.vertexBindingDescriptionCount = (uint32_t)m_VertexInputBinding[PIPELINE_OUTLINE].size();
    vertexInputInfo.pVertexBindingDescriptions = m_VertexInputBinding[PIPELINE_OUTLINE].data();
    vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)m_VertexInputAttribute[PIPELINE_OUTLINE].size();
    vertexInputInfo.pVertexAttributeDescriptions = m_VertexInputAttribute[PIPELINE_OUTLINE].data();

    // Setup input assembly
    // We will be rendering 1 triangle using triangle strip topology
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Setup viewport to the maximum widht and height of the window
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)m_VulkanRenderer->m_swapChainExtent.width;
    viewport.height = (float)m_VulkanRenderer->m_swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Setup scissor rect
    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = m_VulkanRenderer->m_swapChainExtent;

    // Setup view port state
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

#ifdef ENABLE_DYNAMIC_STATE
    // Specify the dynamic state count and VkDynamicState enum stating which
    // dynamic state will use the values from dynamic state commands rather
    // than from the pipeline state creation info.
    dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
    dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;
#endif

    // Setup the rasterizer state
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE/*VK_CULL_MODE_BACK_BIT*/;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthClampEnable = true;

    VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo = {};
    depthStencilStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilStateInfo.pNext = NULL;
    depthStencilStateInfo.flags = 0;
    depthStencilStateInfo.depthTestEnable = true;
    depthStencilStateInfo.depthWriteEnable = true;
    depthStencilStateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencilStateInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilStateInfo.stencilTestEnable = VK_FALSE;
    depthStencilStateInfo.back.failOp = VK_STENCIL_OP_KEEP;
    depthStencilStateInfo.back.passOp = VK_STENCIL_OP_KEEP;
    depthStencilStateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
    depthStencilStateInfo.back.compareMask = 0;
    depthStencilStateInfo.back.reference = 0;
    depthStencilStateInfo.back.depthFailOp = VK_STENCIL_OP_KEEP;
    depthStencilStateInfo.back.writeMask = 0;
    depthStencilStateInfo.minDepthBounds = 0;
    depthStencilStateInfo.maxDepthBounds = 0;
    depthStencilStateInfo.stencilTestEnable = VK_FALSE;
    depthStencilStateInfo.front = depthStencilStateInfo.back;

    // Setup multi sampling. In our first example we will be using single sampling mode
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = NUM_SAMPLES;

    // Setup color output masks.
    // Set to write out RGBA components
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    // Setup color blending
    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    // Create pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
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
    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
#ifdef ENABLE_DYNAMIC_STATE
    pipelineInfo.pDynamicState = &dynamicState;
#endif
    pipelineInfo.layout = graphicsPipelineLayout;
    pipelineInfo.renderPass = m_VulkanRenderer->m_hRenderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.pDepthStencilState = &depthStencilStateInfo;

    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
    vkResult = vkCreateGraphicsPipelines(m_VulkanRenderer->m_hDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline);
    if (vkResult != VK_SUCCESS)
    {
        VulkanHelper::LogError("vkCreateGraphicsPipelines() failed!");
        assert(false);
    }

    QPair<void*/*VkPipeline*/, void*/*VkPipelineLayout*/>* graphicsPipelineAndLayoutPair = new QPair<void*/*VkPipeline*/, void*/*VkPipelineLayout*/>(graphicsPipeline, graphicsPipelineLayout);
    m_GraphicsPipelineMap[PIPELINE_OUTLINE] = graphicsPipelineAndLayoutPair;
//    m_GraphicsPipelineMap[PIPELINE_OUTLINE] = qMakePair(graphicsPipeline, graphicsPipelineLayout);

    // Cleanup
    vkDestroyShaderModule(m_VulkanRenderer->m_hDevice, fragShader, nullptr);
    vkDestroyShaderModule(m_VulkanRenderer->m_hDevice, vertShader, nullptr);
}

void VulkanRectangleInstancingScheme::CreateRectFillPipeline()
{
    // Compile the vertex shader
#ifdef _WIN32
//    VkShaderModule vertShader = VulkanHelper::CreateShader(m_VulkanRenderer->m_hDevice, "F:/Development/GithubRepos/RenderConcept/UIRenderingConcepts/engine/vulkan/implementation/Shaders/RectangleInstancingSchemeVert.spv"); // Relative path to binary output dir
    VkShaderModule vertShader = VulkanHelper::CreateShaderFromQRCResource(m_VulkanRenderer->m_hDevice, "://RectangleInstancingSchemeVert.spv");

#elif __APPLE__
    VkShaderModule vertShader = VulkanHelper::CreateShaderFromQRCResource(m_VulkanRenderer->m_hDevice, "://RectangleInstancingSchemeVert.spv");
#endif

    // Setup the vertex shader stage create info structures
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShader;
    vertShaderStageInfo.pName = "main";

    // Compile the fragment shader
#ifdef _WIN32
//    VkShaderModule fragShader = VulkanHelper::CreateShader(m_VulkanRenderer->m_hDevice, "F:/Development/GithubRepos/RenderConcept/UIRenderingConcepts/engine/vulkan/implementation/Shaders/RectangleInstancingSchemeFrag.spv"); // Relative path to binary output dir
    VkShaderModule fragShader = VulkanHelper::CreateShaderFromQRCResource(m_VulkanRenderer->m_hDevice, "://RectangleInstancingSchemeFrag.spv");
#elif __APPLE__
    VkShaderModule fragShader = VulkanHelper::CreateShaderFromQRCResource(m_VulkanRenderer->m_hDevice, "://RectangleInstancingSchemeFrag.spv");
#endif
    // Setup the fragment shader stage create info structures
    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShader;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

#ifdef ENABLE_DYNAMIC_STATE
    VkDynamicState dynamicStateEnables[VK_DYNAMIC_STATE_RANGE_SIZE];
    memset(dynamicStateEnables, 0, sizeof dynamicStateEnables);

    // Specify the dynamic state information to pipeline through
    // VkPipelineDynamicStateCreateInfo control structure.
    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pNext             = NULL;
    dynamicState.pDynamicStates    = dynamicStateEnables;
    dynamicState.dynamicStateCount = 0;
#endif

    // Setup the vertex input
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = m_VertexInputBinding[PIPELINE_FILLED].size();// sizeof(m_VertexInputBinding[PIPELINE_FILLED]) / sizeof(VkVertexInputBindingDescription);
    vertexInputInfo.pVertexBindingDescriptions = m_VertexInputBinding[PIPELINE_FILLED].data();
    vertexInputInfo.vertexAttributeDescriptionCount = m_VertexInputAttribute[PIPELINE_FILLED].size();// sizeof(m_VertexInputAttribute[PIPELINE_FILLED]) / sizeof(VkVertexInputAttributeDescription);
    vertexInputInfo.pVertexAttributeDescriptions = m_VertexInputAttribute[PIPELINE_FILLED].data();

    // Setup input assembly
    // We will be rendering 1 triangle using triangle strip topology
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Setup viewport to the maximum widht and height of the window
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)m_VulkanRenderer->m_swapChainExtent.width;
    viewport.height = (float)m_VulkanRenderer->m_swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Setup scissor rect
    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = m_VulkanRenderer->m_swapChainExtent;

    // Setup view port state
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

#ifdef ENABLE_DYNAMIC_STATE
    // Specify the dynamic state count and VkDynamicState enum stating which
    // dynamic state will use the values from dynamic state commands rather
    // than from the pipeline state creation info.
    dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
    dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;
#endif

    // Setup the rasterizer state
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE/*VK_CULL_MODE_BACK_BIT*/;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthClampEnable = true;

    VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo = {};
    depthStencilStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilStateInfo.pNext = NULL;
    depthStencilStateInfo.flags = 0;
    depthStencilStateInfo.depthTestEnable = true;
    depthStencilStateInfo.depthWriteEnable = true;
    depthStencilStateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencilStateInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilStateInfo.stencilTestEnable = VK_FALSE;
    depthStencilStateInfo.back.failOp = VK_STENCIL_OP_KEEP;
    depthStencilStateInfo.back.passOp = VK_STENCIL_OP_KEEP;
    depthStencilStateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
    depthStencilStateInfo.back.compareMask = 0;
    depthStencilStateInfo.back.reference = 0;
    depthStencilStateInfo.back.depthFailOp = VK_STENCIL_OP_KEEP;
    depthStencilStateInfo.back.writeMask = 0;
    depthStencilStateInfo.minDepthBounds = 0;
    depthStencilStateInfo.maxDepthBounds = 0;
    depthStencilStateInfo.stencilTestEnable = VK_FALSE;
    depthStencilStateInfo.front = depthStencilStateInfo.back;

    // Setup multi sampling. In our first example we will be using single sampling mode
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = NUM_SAMPLES;

    // Setup color output masks.
    // Set to write out RGBA components
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;

    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    // Setup color blending
    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    // Create pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
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
    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
#ifdef ENABLE_DYNAMIC_STATE
    pipelineInfo.pDynamicState = &dynamicState;
#endif
    pipelineInfo.layout = graphicsPipelineLayout;
    pipelineInfo.renderPass = m_VulkanRenderer->m_hRenderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.pDepthStencilState = &depthStencilStateInfo;

    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
    vkResult = vkCreateGraphicsPipelines(m_VulkanRenderer->m_hDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline);
    if (vkResult != VK_SUCCESS)
    {
        VulkanHelper::LogError("vkCreateGraphicsPipelines() failed!");
        assert(false);
    }

//    m_GraphicsPipelineMap[PIPELINE_FILLED] = qMakePair(graphicsPipeline, graphicsPipelineLayout);
    QPair<void*/*VkPipeline*/, void*/*VkPipelineLayout*/>* graphicsPipelineAndLayoutPair = new QPair<void*/*VkPipeline*/, void*/*VkPipelineLayout*/>(graphicsPipeline, graphicsPipelineLayout);
    m_GraphicsPipelineMap[PIPELINE_FILLED] = graphicsPipelineAndLayoutPair;

    // Cleanup
    vkDestroyShaderModule(m_VulkanRenderer->m_hDevice, fragShader, nullptr);
    vkDestroyShaderModule(m_VulkanRenderer->m_hDevice, vertShader, nullptr);
}

void VulkanRectangleInstancingScheme::CreateGraphicsPipeline(bool p_ClearGraphicsPipelineMap)
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

    if (m_GraphicsPipelineMap.contains(PIPELINE_FILLED)) return;

    CreateRectFillPipeline();
    CreateRectOutlinePipeline();
}

void VulkanRectangleInstancingScheme::RecordCommandBuffer()
{
    // Specify the clear color value
    VkClearValue clearColor[2];
    clearColor[0].color.float32[0] = 0.0f;
    clearColor[0].color.float32[1] = 0.0f;
    clearColor[0].color.float32[2] = 0.0f;
    clearColor[0].color.float32[3] = 0.0f;

    // Specify the depth/stencil clear value
    clearColor[1].depthStencil.depth = 1.0f;
    clearColor[1].depthStencil.stencil = 0;

    // Offset to render in the frame buffer
    VkOffset2D   renderOffset = { 0, 0 };
    // Width & Height to render in the frame buffer
    VkExtent2D   renderExtent = m_VulkanRenderer->m_swapChainExtent;

    // For each command buffers in the command buffer list
    for (size_t i = 0; i < m_VulkanRenderer->m_hCommandBufferList.size(); i++)
    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        // Indicate that the command buffer can be resubmitted to the queue
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

        // Begin command buffer
        vkBeginCommandBuffer(m_VulkanRenderer->m_hCommandBufferList[i], &beginInfo);

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_VulkanRenderer->m_hRenderPass;
        renderPassInfo.framebuffer = m_VulkanRenderer->m_hFramebuffers[i];
        renderPassInfo.renderArea.offset = renderOffset;
        renderPassInfo.renderArea.extent = renderExtent;
        renderPassInfo.clearValueCount = 2;
        renderPassInfo.pClearValues = clearColor;

        // Begin render pass
        vkCmdBeginRenderPass(m_VulkanRenderer->m_hCommandBufferList[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        Render(m_VulkanRenderer->m_hCommandBufferList[i]); // consider using shared ptr/smart pointers

        // End the Render pass
        vkCmdEndRenderPass(m_VulkanRenderer->m_hCommandBufferList[i]);

        // End the Command buffer
        VkResult vkResult = vkEndCommandBuffer(m_VulkanRenderer->m_hCommandBufferList[i]);
        if (vkResult != VK_SUCCESS)
        {
            VulkanHelper::LogError("vkEndCommandBuffer() failed!");
            assert(false);
        }
    }
}

void VulkanRectangleInstancingScheme::CreateVertexBuffer()
{
    for (int pipelineIdx = 0; pipelineIdx < RECTANGLE_GRAPHICS_PIPELINES::PIPELINE_COUNT; pipelineIdx++)
    {
        m_VertexBuffer[pipelineIdx].m_MemoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

        const VkPhysicalDeviceMemoryProperties& memProp = m_VulkanRenderer->m_physicalDeviceInfo.memProp;
        const VkDevice& device = m_VulkanRenderer->m_hDevice;
        if (pipelineIdx == PIPELINE_FILLED)
        {
            m_VertexBuffer[pipelineIdx].m_DataSize = sizeof(s_RectFilledVertices);
            m_VertexCount[PIPELINE_FILLED] = sizeof(s_RectFilledVertices) / sizeof(Vertex);

            uint32_t dataStride = sizeof(s_RectFilledVertices[0]);
            VulkanHelper::CreateBuffer(device, memProp, m_VertexBuffer[pipelineIdx], VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, s_RectFilledVertices);

            m_VertexInputBinding[pipelineIdx].resize(2);   // 0 for position and 1 for color
            m_VertexInputAttribute[pipelineIdx].resize(9); // Why 7 = 2(for position and color) + 5 (transform and rotation) + Color

            // Indicates the rate at which the information will be
            // injected for vertex input.
            m_VertexInputBinding[pipelineIdx][0].binding = VERTEX_BUFFER_BIND_IDX;
            m_VertexInputBinding[pipelineIdx][0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            m_VertexInputBinding[pipelineIdx][0].stride = dataStride;

            m_VertexInputBinding[pipelineIdx][1].binding = INSTANCE_BUFFER_BIND_IDX;
            m_VertexInputBinding[pipelineIdx][1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
            m_VertexInputBinding[pipelineIdx][1].stride = sizeof(InstanceData);

            // The VkVertexInputAttribute interpreting the data.
            m_VertexInputAttribute[pipelineIdx][0].binding = 0;
            m_VertexInputAttribute[pipelineIdx][0].location = 0;
            m_VertexInputAttribute[pipelineIdx][0].format = VK_FORMAT_R32G32B32_SFLOAT;
            m_VertexInputAttribute[pipelineIdx][0].offset = offsetof(struct Vertex, m_Position);

            m_VertexInputAttribute[pipelineIdx][1].binding = 0;
            m_VertexInputAttribute[pipelineIdx][1].location = 1;
            m_VertexInputAttribute[pipelineIdx][1].format = VK_FORMAT_R32G32B32_SFLOAT;
            m_VertexInputAttribute[pipelineIdx][1].offset = offsetof(struct Vertex, m_Color);
            ////////////////////////////////////////////////////////////////////////////////////

            // Model Matrix
            m_VertexInputAttribute[pipelineIdx][2].binding = INSTANCE_BUFFER_BIND_IDX;
            m_VertexInputAttribute[pipelineIdx][2].location = 2;
            m_VertexInputAttribute[pipelineIdx][2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
            m_VertexInputAttribute[pipelineIdx][2].offset = 0;

            m_VertexInputAttribute[pipelineIdx][3].binding = INSTANCE_BUFFER_BIND_IDX;
            m_VertexInputAttribute[pipelineIdx][3].location = 3;
            m_VertexInputAttribute[pipelineIdx][3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
            m_VertexInputAttribute[pipelineIdx][3].offset = 16 * 1;

            m_VertexInputAttribute[pipelineIdx][4].binding = INSTANCE_BUFFER_BIND_IDX;
            m_VertexInputAttribute[pipelineIdx][4].location = 4;
            m_VertexInputAttribute[pipelineIdx][4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
            m_VertexInputAttribute[pipelineIdx][4].offset = 16 * 2;

            m_VertexInputAttribute[pipelineIdx][5].binding = INSTANCE_BUFFER_BIND_IDX;
            m_VertexInputAttribute[pipelineIdx][5].location = 5;
            m_VertexInputAttribute[pipelineIdx][5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
            m_VertexInputAttribute[pipelineIdx][5].offset = 16 * 3;

            // Dimension
            m_VertexInputAttribute[pipelineIdx][6].binding = INSTANCE_BUFFER_BIND_IDX;
            m_VertexInputAttribute[pipelineIdx][6].location = 6;
            m_VertexInputAttribute[pipelineIdx][6].format = VK_FORMAT_R32G32B32_SFLOAT;
            m_VertexInputAttribute[pipelineIdx][6].offset = 16 * 4;

            // Color
            m_VertexInputAttribute[pipelineIdx][7].binding = INSTANCE_BUFFER_BIND_IDX;
            m_VertexInputAttribute[pipelineIdx][7].location = 7;
            m_VertexInputAttribute[pipelineIdx][7].format = VK_FORMAT_R32G32B32_SFLOAT;
            m_VertexInputAttribute[pipelineIdx][7].offset = 16 * 5;

            // BoolFlags
            m_VertexInputAttribute[pipelineIdx][8].binding = INSTANCE_BUFFER_BIND_IDX;
            m_VertexInputAttribute[pipelineIdx][8].location = 8;
            m_VertexInputAttribute[pipelineIdx][8].format = VK_FORMAT_R32_UINT;
            m_VertexInputAttribute[pipelineIdx][8].offset = /*16 * 5*/offsetof(struct InstanceData, m_BoolFlags);

        }
        else if (pipelineIdx == PIPELINE_OUTLINE)
        {
            m_VertexBuffer[pipelineIdx].m_DataSize = sizeof(s_RectOutlineVertices);
            m_VertexCount[PIPELINE_OUTLINE] = sizeof(s_RectOutlineVertices) / sizeof(Vertex);
            const uint32_t dataStride = sizeof(s_RectOutlineVertices[0]);
            VulkanHelper::CreateBuffer(device, memProp, m_VertexBuffer[pipelineIdx], VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, s_RectOutlineVertices);

            m_VertexInputBinding[pipelineIdx].resize(2);   // 0 for position and 1 for color
            m_VertexInputAttribute[pipelineIdx].resize(9); // Why 7 = 2(for position and color) + 5 (transform and rotation) + Color

            // Indicates the rate at which the information will be
            // injected for vertex input.
            m_VertexInputBinding[pipelineIdx][0].binding = VERTEX_BUFFER_BIND_IDX;
            m_VertexInputBinding[pipelineIdx][0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            m_VertexInputBinding[pipelineIdx][0].stride = dataStride;

            m_VertexInputBinding[pipelineIdx][1].binding = INSTANCE_BUFFER_BIND_IDX;
            m_VertexInputBinding[pipelineIdx][1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
            m_VertexInputBinding[pipelineIdx][1].stride = sizeof(InstanceData);

            // The VkVertexInputAttribute interpreting the data.
            m_VertexInputAttribute[pipelineIdx][0].binding = VERTEX_BUFFER_BIND_IDX;
            m_VertexInputAttribute[pipelineIdx][0].location = 0;
            m_VertexInputAttribute[pipelineIdx][0].format = VK_FORMAT_R32G32B32_SFLOAT;
            m_VertexInputAttribute[pipelineIdx][0].offset = offsetof(struct Vertex, m_Position);

            m_VertexInputAttribute[pipelineIdx][1].binding = VERTEX_BUFFER_BIND_IDX;
            m_VertexInputAttribute[pipelineIdx][1].location = 1;
            m_VertexInputAttribute[pipelineIdx][1].format = VK_FORMAT_R32G32B32_SFLOAT;
            m_VertexInputAttribute[pipelineIdx][1].offset = offsetof(struct Vertex, m_Color);
            ////////////////////////////////////////////////////////////////////////////////////

            // Model Matrix
            m_VertexInputAttribute[pipelineIdx][2].binding = INSTANCE_BUFFER_BIND_IDX;
            m_VertexInputAttribute[pipelineIdx][2].location = 2;
            m_VertexInputAttribute[pipelineIdx][2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
            m_VertexInputAttribute[pipelineIdx][2].offset = 0;

            m_VertexInputAttribute[pipelineIdx][3].binding = INSTANCE_BUFFER_BIND_IDX;
            m_VertexInputAttribute[pipelineIdx][3].location = 3;
            m_VertexInputAttribute[pipelineIdx][3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
            m_VertexInputAttribute[pipelineIdx][3].offset = 16 * 1;

            m_VertexInputAttribute[pipelineIdx][4].binding = INSTANCE_BUFFER_BIND_IDX;
            m_VertexInputAttribute[pipelineIdx][4].location = 4;
            m_VertexInputAttribute[pipelineIdx][4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
            m_VertexInputAttribute[pipelineIdx][4].offset = 16 * 2;

            m_VertexInputAttribute[pipelineIdx][5].binding = INSTANCE_BUFFER_BIND_IDX;
            m_VertexInputAttribute[pipelineIdx][5].location = 5;
            m_VertexInputAttribute[pipelineIdx][5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
            m_VertexInputAttribute[pipelineIdx][5].offset = 16 * 3;

            // Dimension
            m_VertexInputAttribute[pipelineIdx][6].binding = INSTANCE_BUFFER_BIND_IDX;
            m_VertexInputAttribute[pipelineIdx][6].location = 6;
            m_VertexInputAttribute[pipelineIdx][6].format = VK_FORMAT_R32G32B32_SFLOAT;
            m_VertexInputAttribute[pipelineIdx][6].offset = /*16 * 4*/offsetof(struct InstanceData, m_Rect);

            // Color
            m_VertexInputAttribute[pipelineIdx][7].binding = INSTANCE_BUFFER_BIND_IDX;
            m_VertexInputAttribute[pipelineIdx][7].location = 7;
            m_VertexInputAttribute[pipelineIdx][7].format = VK_FORMAT_R32G32B32_SFLOAT;
            m_VertexInputAttribute[pipelineIdx][7].offset = /*16 * 5*/offsetof(struct InstanceData, m_Color);

            // BoolFlags
            m_VertexInputAttribute[pipelineIdx][8].binding = INSTANCE_BUFFER_BIND_IDX;
            m_VertexInputAttribute[pipelineIdx][8].location = 8;
            m_VertexInputAttribute[pipelineIdx][8].format = VK_FORMAT_R32_UINT;
            m_VertexInputAttribute[pipelineIdx][8].offset = /*16 * 5*/offsetof(struct InstanceData, m_BoolFlags);
        }
    }
}

void VulkanRectangleInstancingScheme::UpdatePipelineNodeList(Node *p_Item)
{
    //m_ModelList.push_back(p_Item);

    Rectangl* rectangle = dynamic_cast<Rectangl*>(p_Item);
    assert(rectangle);

    p_Item->SetRenderSchemeObject(this);

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
}

void VulkanRectangleInstancingScheme::RemovePipelineNodeList(Node* p_Model)
{
    Rectangl* rectangle = dynamic_cast<Rectangl*>(p_Model);
    assert(rectangle);

    std::vector<Node*>* modelVector = &m_PipelineTypeModelVector[rectangle->GetDrawType()];
    if (!modelVector) return;

    bool isUpdated = false;
    while (1)
    {
        auto result = std::find(std::begin(*modelVector), std::end(*modelVector), rectangle);
        if (result == std::end(*modelVector)) break;

        modelVector->erase(result);
        isUpdated = true;
    }

    if (isUpdated)
    {
        RECTANGLE_GRAPHICS_PIPELINES pipeline = PIPELINE_COUNT;

        switch (rectangle->GetDrawType())
        {
        case Rectangl::FILLED:
            pipeline = PIPELINE_FILLED;
            break;

        case Rectangl::OUTLINE:
            pipeline = PIPELINE_OUTLINE;
            break;

        case Rectangl::ROUNDED:
            // TODO
            break;

        default:
            break;
        }

        if (pipeline == PIPELINE_COUNT) return;

        PrepareInstanceData(pipeline);
    }
}

void VulkanRectangleInstancingScheme::PrepareInstanceData(RECTANGLE_GRAPHICS_PIPELINES p_Pipeline /*=PIPELINE_COUNT*/)
{
    bool update = false;
    bool isSinglePipelinePrepareRequest = (p_Pipeline != PIPELINE_COUNT);
    for (int pipelineIdx = (isSinglePipelinePrepareRequest ? p_Pipeline : 0); (pipelineIdx < PIPELINE_COUNT); pipelineIdx++)
    {
        NodeVector& m_ModelList = m_PipelineTypeModelVector[pipelineIdx];
        const int modelSize = m_ModelList.size();
        if (!modelSize) continue;

        //int oldInstanceDataSize = m_InstanceData.size();
        std::vector<InstanceData> instanceData;
        //m_InstanceData.clear();
        instanceData.resize(modelSize);

        for (int i = 0; i < modelSize; i++)
        {
            Node* rectangle = m_ModelList.at(i);

            instanceData[i].m_Model = rectangle->GetAbsoluteTransformation();
            instanceData[i].m_Rect.x = rectangle->GetBoundedRegion().m_Dimension.x;
            instanceData[i].m_Rect.y = rectangle->GetBoundedRegion().m_Dimension.y;
            instanceData[i].m_Color = rectangle->GetColor();
            instanceData[i].m_BoolFlags = rectangle->GetVisible() ? 1 : 0;
            rectangle->SetGpuMemOffset(i * sizeof(InstanceData));
        }

        if (modelSize != 0)
        {
            VkMemoryPropertyFlags memoryProperty = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            m_InstanceBuffer[pipelineIdx].m_MemoryFlags = memoryProperty;
            m_InstanceBuffer[pipelineIdx].m_DataSize = modelSize * sizeof(InstanceData);
            // Re-Create instance buffer if size not same.

            VulkanHelper::CreateStagingBuffer(m_VulkanRenderer->m_hDevice,
                m_VulkanRenderer->m_physicalDeviceInfo.memProp,
                m_VulkanRenderer->m_hCommandPool,
                m_VulkanRenderer->m_hGraphicsQueue,
                m_InstanceBuffer[pipelineIdx],
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                instanceData.data());

            if (modelSize != m_OldInstanceDataSize[pipelineIdx] && modelSize != 0)
            {
                update = true;
                //RecordCommandBuffer();
            }
        }

        m_OldInstanceDataSize[pipelineIdx] = modelSize;

        if (isSinglePipelinePrepareRequest)
        {
             // Single pipeline prepare only loop once.
             break;
        }
    }

    if (update)
    {
        RecordCommandBuffer();
    }
}

void VulkanRectangleInstancingScheme::UpdateDirtyInstanceData()
{
    bool update = false;
    for (int pipelineIdx = 0; pipelineIdx < PIPELINE_COUNT; pipelineIdx++)
    {
        NodeVector& nodeVector = m_PipelineTypeModelVector[pipelineIdx];
        const int modelSize = static_cast<int>(nodeVector.size());
        if (!modelSize) continue;

        std::vector<InstanceData> instanceData;
        std::vector<unsigned int> destOffset;
        std::vector<VkBufferCopy> copyRegion;
        const uint64_t instanceDataSize = static_cast<uint64_t>(sizeof(InstanceData));
        DIRTY_TYPE dirtyType = DIRTY_TYPE::NONE;
        for (int i = 0; i < modelSize; i++)
        {
            Node* nodeItem = nodeVector.at(i);
            dirtyType = nodeItem->GetDirtyType();
            // Parminder: TODO, this loop is really expensive because for N number of items it iterating through N times
            // Gather dirty node nad loop through them pls.

            if ((dirtyType == DIRTY_TYPE::ATTRIBUTES) || (dirtyType == DIRTY_TYPE::POSITION))
            {
                InstanceData data;
                data.m_Model = nodeItem->GetAbsoluteTransformationComputation();
                data.m_Rect.x = nodeItem->GetBoundedRegion().m_Dimension.x;
                data.m_Rect.y = nodeItem->GetBoundedRegion().m_Dimension.y;
                data.m_Color = nodeItem->GetColor();
                data.m_BoolFlags = nodeItem->GetVisible() ? 1 : 0;

                instanceData.push_back(data);
                destOffset.push_back(nodeItem->GetGpuMemOffset());

                VkBufferCopy copyRgnItem;
                copyRgnItem.size = static_cast<VkDeviceSize>(instanceDataSize);
                copyRgnItem.srcOffset = static_cast<VkDeviceSize>(copyRegion.size() * instanceDataSize);
                copyRgnItem.dstOffset = static_cast<VkDeviceSize>(nodeItem->GetGpuMemOffset());
                copyRegion.push_back(copyRgnItem);

                nodeItem->SetDirtyType(DIRTY_TYPE::NONE);
            }
        }

        const size_t dirtyItemSize = instanceData.size();
        if (dirtyItemSize != 0)
        {
            VulkanHelper::CreateStagingBufferCopyRegion(m_VulkanRenderer->m_hDevice,
                m_VulkanRenderer->m_physicalDeviceInfo.memProp,
                m_VulkanRenderer->m_hCommandPool,
                m_VulkanRenderer->m_hGraphicsQueue,
                m_InstanceBuffer[pipelineIdx],
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                instanceData.data(),
                dirtyItemSize * sizeof(InstanceData),
                NULL,
                copyRegion);
        }

        if (dirtyItemSize != 0)
        {
            update = true;
        }

        //m_OldInstanceDataSize[pipelineIdx] = modelSize;
    }

    if (!update) return;

    // TODO: Check the reason why do we need to record the command buffer?
    RecordCommandBuffer();
}

void VulkanRectangleInstancingScheme::Render(void* p_CommandBuffer)
{
    VkCommandBuffer cmdBuffer = static_cast<VkCommandBuffer>(p_CommandBuffer);
    for (int pipelineIdx = 0; pipelineIdx < RECTANGLE_GRAPHICS_PIPELINES::PIPELINE_COUNT; pipelineIdx++)
    {
        NodeVector& m_ModelList = m_PipelineTypeModelVector[pipelineIdx];
        if (!m_ModelList.size()) continue;

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

        // Bind graphics pipeline
        vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
        vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelineLayout,
            0, 1, m_DescriptorSet->descriptorSet.data(), 0, NULL);

        // Specify vertex buffer information
        const VkDeviceSize offsets[1] = { 0 };
        vkCmdBindVertexBuffers(cmdBuffer, VERTEX_BUFFER_BIND_IDX, 1, &m_VertexBuffer[pipelineIdx].m_Buffer, offsets);
        vkCmdBindVertexBuffers(cmdBuffer, INSTANCE_BUFFER_BIND_IDX, 1, &m_InstanceBuffer[pipelineIdx].m_Buffer, offsets);

        // Draw the Rectangle
        vkCmdDraw(cmdBuffer, m_VertexCount[pipelineIdx], m_ModelList.size(), 0, 0);
    }
}
