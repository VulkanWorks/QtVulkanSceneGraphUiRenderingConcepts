#include "VulkanHelper.h"

#include <QFile>

std::vector<LayerProperties> VulkanHelper::m_LayerPropertyList = {};

VulkanHelper::VulkanHelper()
{
}

VulkanHelper::~VulkanHelper()
{
}

// Helper method to log error messages
void VulkanHelper::LogError(string text)
{
    string outputText;
    outputText = "Error: " + text;
    cout << outputText;
    assert(0);
}

VkResult VulkanHelper::GetInstanceLayerExtensionProperties()
{
    uint32_t                        instanceLayerCount;     // Stores number of layers supported by instance
    std::vector<VkLayerProperties>  layerProperties;        // Vector to store layer properties
    VkResult                        result;                 // Variable to check Vulkan API result status

    m_LayerPropertyList.clear();

    // Query all the layers
    result = vkEnumerateInstanceLayerProperties(&instanceLayerCount, NULL);
    layerProperties.resize(instanceLayerCount);
    result = vkEnumerateInstanceLayerProperties(&instanceLayerCount, layerProperties.data());

    // Query all the extensions for each layer and store it.
    std::cout << "\nInstanced Layers" << std::endl;
    std::cout << "===================" << std::endl;
    std::cout << "\n" << std::left << std::setw(40) << "Instance Layer Name" << " | " << std::setw(40) << "Instance Layer Description" << " | " << "Available Extensions\n";
    std::cout << "-------------------------------------------------------------------------------------------------------------------" << std::endl;
    for (auto globalLayerProp : layerProperties) {
        LayerProperties layerProps;
        layerProps.properties = globalLayerProp;

        // Get Instance level extensions for corresponding layer properties
        result = GetExtensionProperties(layerProps);
        if (result) continue;

        m_LayerPropertyList.push_back(layerProps);

        // Get extension name for each instance layer
        string extensions = "[ ";
        if (layerProps.extensions.size()) {
            for (auto j : layerProps.extensions) {
                extensions += j.extensionName;
                extensions += " ";
            }
        }
        else {
            extensions = "None";
        }

        extensions += " ]";

        // Print Instance Layer info
        std::cout << "\n" << std::left << std::setw(40) << globalLayerProp.layerName << " | " << std::setw(40) << globalLayerProp.description << " | " << extensions;
    }
    std::cout << "\n-------------------------------------------------------------------------------------------------------------------" << std::endl;
    return result;
}

VkResult VulkanHelper::GetDeviceLayerExtensionProperties(VkPhysicalDevice gpu)
{
    std::cout << "\n\nDevice layer extensions" << std::endl;
    std::cout << "==========================" << std::endl;
    VkResult result;
    std::vector<LayerProperties> instanceLayerProp = m_LayerPropertyList;

    std::cout << "\n" << std::left << std::setw(40) << "Device Layer Name" << " | " << std::setw(40) << "Device Layer Description" << " | " << "Available Extensions\n";
    std::cout << "-------------------------------------------------------------------------------------------------------------------" << std::endl;

    for (auto globalLayerProp : instanceLayerProp) {
        LayerProperties layerProps;
        layerProps.properties = globalLayerProp.properties;

        if (result = GetExtensionProperties(layerProps, gpu))
            continue;

        m_LayerPropertyList.push_back(layerProps);

        string extensions = "[ ";
        if (layerProps.extensions.size()) {
            for (auto j : layerProps.extensions) {
                extensions += j.extensionName;
                extensions += " ";
            }
        }
        else {
            extensions += "None";
        }
        extensions += " ]";

        // Print Device Layer info
        std::cout << "\n" << std::left << std::setw(40) << globalLayerProp.properties.layerName << " | " << std::setw(40) << globalLayerProp.properties.description << " | " << extensions;
    }
    std::cout << "\n-------------------------------------------------------------------------------------------------------------------" << std::endl;
    return result;
}

// This function retrieves extension and its properties at instance 
// and device level. Pass a valid physical device
// pointer to retrieve device level extensions, otherwise
// use NULL to retrieve extension specific to instance level.
VkResult VulkanHelper::GetExtensionProperties(LayerProperties &layerProps, VkPhysicalDevice gpu)
{
    uint32_t    extensionCount;                                 // Stores number of extension per layer
    VkResult    result;                                         // Variable to check Vulkan API result status
    char*       layerName = layerProps.properties.layerName;    // Name of the layer

    do {
        // Get the total number of extension in this layer
        if (gpu)
            result = vkEnumerateDeviceExtensionProperties(gpu, layerName, &extensionCount, NULL);
        else
            result = vkEnumerateInstanceExtensionProperties(layerName, &extensionCount, NULL);

        if (result || extensionCount == 0) continue;

        layerProps.extensions.resize(extensionCount);

        // Gather all extension properties
        if (gpu)
            result = vkEnumerateDeviceExtensionProperties(gpu, layerName, &extensionCount, layerProps.extensions.data());
        else
            result = vkEnumerateInstanceExtensionProperties(layerName, &extensionCount, layerProps.extensions.data());
    } while (result == VK_INCOMPLETE);

    return result;
}

// Returns the best surface format to create the swap chain
VkSurfaceFormatKHR VulkanHelper::SelectBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    // Choose the first format as default
    VkSurfaceFormatKHR bestFormat = availableFormats[0];

    // When no formats is available then choose the following format
    if (availableFormats.size() == 1 && bestFormat.format == VK_FORMAT_UNDEFINED)
    {
        bestFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
        bestFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    }
    else
    {
        for (unsigned int i = 0; i < availableFormats.size(); i++)
        {
            // Choose a preferred format
            if (availableFormats[i].format == VK_FORMAT_B8G8R8A8_UNORM &&
                availableFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                bestFormat = availableFormats[i];
                break;
            }
        }
    }

    return bestFormat;
}

// Returns the best present mode to create the swap chain
VkPresentModeKHR VulkanHelper::SelectBestPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes)
{
    // By default set to VK_PRESENT_MODE_FIFO_KHR where the presentation engine
    // waits for the next vertical blanking period to update the current image
    VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

    for (uint32_t i = 0; i < availablePresentModes.size(); i++)
    {
        if (availablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            // Set the mode to VK_PRESENT_MODE_MAILBOX_KHR where the presentation engine
            // waits for the next vertical blanking period to update the current image.
            // The image waiting to be displayed may get overwritten.
            // This mode do not cause image tearing
            bestMode = VK_PRESENT_MODE_MAILBOX_KHR;
            break;
        }

        if (availablePresentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
        {
            // Set the mode to VK_PRESENT_MODE_IMMEDIATE_KHR where the presentation engine
            // will not wait for display vertical blank interval
            // This mode may cause image tearing
            bestMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
        }
    }

    return bestMode;
}

// Returns the best swap chain extent to create the swap chain
VkExtent2D VulkanHelper::SelectBestExtent(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D& windowDim)
{
    VkExtent2D swapChainExtent = capabilities.currentExtent;

    // If either the width or height is -1 then set to swap chain extent to window dimension
    // otherwise choose the current extent from device capabilities
    if (capabilities.currentExtent.width == (uint32_t)-1 ||
       capabilities.currentExtent.height == (uint32_t)-1)
    {
        swapChainExtent.width = windowDim.width;
        swapChainExtent.height = windowDim.height;
    }
    else if (swapChainExtent.width != windowDim.width ||
             swapChainExtent.height != windowDim.height)
    {
        swapChainExtent.width = windowDim.width;
        swapChainExtent.height = windowDim.height;
    }

    return (swapChainExtent);
}

VkShaderModule VulkanHelper::CreateShader(VkDevice device, const std::string& filename)
{
    VkShaderModule shaderModule = 0;
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (file.is_open())
    {
        // Read the shader file
        size_t fileSize = (size_t)file.tellg();
        std::vector<char> shaderCode(fileSize);

        file.seekg(0);
        file.read(shaderCode.data(), fileSize);
        file.close();

        // Now create the shader module
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = shaderCode.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

        VkResult result = vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);

        if (result != VK_SUCCESS)
        {
            LogError("Failed to create shader!");
            assert(false);
        }
    }
    else
    {
        LogError("Failed to open file!");
        assert(false);
    }

    return shaderModule;
}

VkShaderModule VulkanHelper::CreateShaderFromQRCResource(VkDevice device, const std::string& filename)
{
    QFile shadersFile(QString(filename.c_str()));
    if (!shadersFile.open(QIODevice::ReadOnly))
    {
        LogError("Failed to create shader!");
        assert(false);
    }

    QByteArray shaderCode = shadersFile.readAll();
    VkShaderModule shaderModule = 0;

    // Now create the shader module
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = shaderCode.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.constData());

    VkResult result = vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);

    if (result != VK_SUCCESS)
    {
        LogError("Failed to create shader!");
        assert(false);
    }

    return shaderModule;
}

bool VulkanHelper::MemoryTypeFromProperties(VkPhysicalDeviceMemoryProperties memoryProperties, uint32_t typeBits, 
                                            VkFlags requirementsMask, uint32_t *typeIndex)
{
    // Search memtypes to find first index with those properties
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        if ((typeBits & 1) == 1) {
            // Type is available, does it match user properties?
            if ((memoryProperties.memoryTypes[i].propertyFlags & requirementsMask) == requirementsMask) {
                *typeIndex = i;
                return true;
            }
        }
        typeBits >>= 1;
    }
    // No memory types matched, return failure
    return false;
}

void VulkanHelper::CreateCommandPool(const VkDevice& device, VkCommandPool& cmdPool, const PhysicalDeviceInfo& deviceInfo, const VkCommandPoolCreateInfo* commandPoolInfo)
{
    VkResult vkResult;
    if (commandPoolInfo)
    {
        vkResult = vkCreateCommandPool(device, commandPoolInfo, nullptr, &cmdPool);
        assert(!vkResult);
    }
    else
    {
        // Create the command buffer pool object
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = 0;
        poolInfo.queueFamilyIndex = deviceInfo.graphicsFamilyIndex;
        vkResult = vkCreateCommandPool(device, &poolInfo, nullptr, &cmdPool);
        assert(!vkResult);
    }
}

void VulkanHelper::AllocateCommandBuffer(const VkDevice device, const VkCommandPool cmdPool, VkCommandBuffer* cmdBuf, const VkCommandBufferAllocateInfo* commandBufferInfo)
{
    // Dependency on the intialize SwapChain Extensions and initialize CommandPool
    VkResult result;

    // If command information is available use it as it is.
    if (commandBufferInfo) {
        result = vkAllocateCommandBuffers(device, commandBufferInfo, cmdBuf);
        assert(!result);
        return;
    }

    // Default implementation, create the command buffer
    // allocation info and use the supplied parameter into it
    VkCommandBufferAllocateInfo cmdInfo = {};
    cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdInfo.pNext = NULL;
    cmdInfo.commandPool = cmdPool;
    cmdInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdInfo.commandBufferCount = (uint32_t) sizeof(cmdBuf) / sizeof(VkCommandBuffer);;

    result = vkAllocateCommandBuffers(device, &cmdInfo, cmdBuf);
    assert(!result);
}

void VulkanHelper::BeginCommandBuffer(VkCommandBuffer cmdBuf, VkCommandBufferBeginInfo* inCmdBufInfo)
{
    // Dependency on  the initialieCommandBuffer()
    VkResult  result;
    // If the user has specified the custom command buffer use it
    if (inCmdBufInfo) {
        result = vkBeginCommandBuffer(cmdBuf, inCmdBufInfo);
        assert(result == VK_SUCCESS);
        return;
    }

    // Otherwise, use the default implementation.
    VkCommandBufferInheritanceInfo cmdBufInheritInfo = {};
    cmdBufInheritInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    cmdBufInheritInfo.pNext = NULL;
    cmdBufInheritInfo.renderPass = VK_NULL_HANDLE;
    cmdBufInheritInfo.subpass = 0;
    cmdBufInheritInfo.framebuffer = VK_NULL_HANDLE;
    cmdBufInheritInfo.occlusionQueryEnable = VK_FALSE;
    cmdBufInheritInfo.queryFlags = 0;
    cmdBufInheritInfo.pipelineStatistics = 0;

    VkCommandBufferBeginInfo cmdBufInfo = {};
    cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBufInfo.pNext = NULL;
    cmdBufInfo.flags = 0;
    cmdBufInfo.pInheritanceInfo = &cmdBufInheritInfo;

    result = vkBeginCommandBuffer(cmdBuf, &cmdBufInfo);

    assert(result == VK_SUCCESS);
}

void VulkanHelper::EndCommandBuffer(VkCommandBuffer commandBuffer)
{
    VkResult  result;
    result = vkEndCommandBuffer(commandBuffer);
    assert(result == VK_SUCCESS);
}

void VulkanHelper::SubmitCommandBuffer(const VkQueue& queue, const VkCommandBuffer commandBuffer, const VkSubmitInfo* inSubmitInfo, const VkFence& fence)
{
    VkResult result;

    // If Subimt information is avialable use it as it is, this assumes that
    // the commands are already specified in the structure, hence ignore command buffer
    if (inSubmitInfo) {
        result = vkQueueSubmit(queue, 1, inSubmitInfo, fence);
        assert(!result);

        result = vkQueueWaitIdle(queue);
        assert(!result);
        return;
    }

    // Otherwise, create the submit information with specified buffer commands
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = NULL;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = NULL;
    submitInfo.pWaitDstStageMask = NULL;
    submitInfo.commandBufferCount = (uint32_t)sizeof(commandBuffer) / sizeof(VkCommandBuffer);
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = NULL;

    result = vkQueueSubmit(queue, 1, &submitInfo, fence);
    assert(!result);

    result = vkQueueWaitIdle(queue); // May be put this in a flag to wait for vkQueue idle
    assert(!result);
}

bool VulkanHelper::SetImageLayoutEx(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, const VkCommandBuffer& commandBuffer)
{
    bool result = true;

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        result = false;
        VulkanHelper::LogError("Unsupported layout transition!");
    }

    if (result)
    {
        vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage,
            0, 0, nullptr, 0, nullptr, 1, &barrier);
    }

    return (result);
}

void VulkanHelper::SetImageLayout(VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkAccessFlagBits srcAccessMask, const VkCommandBuffer& commandBuffer)
{
    // Dependency on commandBuffer
    assert(commandBuffer != VK_NULL_HANDLE);

    VkImageMemoryBarrier imgMemoryBarrier = {};
    imgMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imgMemoryBarrier.pNext = NULL;
    imgMemoryBarrier.srcAccessMask = srcAccessMask;
    imgMemoryBarrier.dstAccessMask = 0;
    imgMemoryBarrier.oldLayout = oldImageLayout;
    imgMemoryBarrier.newLayout = newImageLayout;
    imgMemoryBarrier.image = image;
    imgMemoryBarrier.subresourceRange.aspectMask = aspectMask;
    imgMemoryBarrier.subresourceRange.baseMipLevel = 0;
    imgMemoryBarrier.subresourceRange.levelCount = 1;
    imgMemoryBarrier.subresourceRange.layerCount = 1;

    if (oldImageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        imgMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    }

    switch (newImageLayout)
    {
        // Ensure that anything that was copying from this image has completed
        // An image in this layout can only be used as the destination operand of the commands
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
        imgMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        break;

        // Ensure any Copy or CPU writes to image are flushed
        // An image in this layout can only be used as a read-only shader resource
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        imgMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imgMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        break;

        // An image in this layout can only be used as a framebuffer color attachment
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        imgMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        break;

        // An image in this layout can only be used as a framebuffer depth/stencil attachment
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        imgMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;
    }

    VkPipelineStageFlags srcStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags destStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    vkCmdPipelineBarrier(commandBuffer, srcStages, destStages, 0, 0, NULL, 0, NULL, 1, &imgMemoryBarrier);
}

void VulkanHelper::CreateBuffer(const VkDevice p_Device, VkPhysicalDeviceMemoryProperties p_DeviceMemProp, VulkanBuffer& p_VulkanBuffer, VkBufferUsageFlags p_UsageFlags, const void* p_Data, VkBufferCreateInfo* p_pBufInfo)
{
    if (p_VulkanBuffer.m_DataSize <= 0)
    {
        LogError("Error: Trying to create buffer with invalid size");
        assert(0);
    }

    VkResult  result;
    // 1. Create the Buffer resource - Default usage - vertex buffer
    /*******************************/
    if (p_pBufInfo){
        result = vkCreateBuffer(p_Device, p_pBufInfo, NULL, &p_VulkanBuffer.m_Buffer);
    }
    else {
        VkBufferCreateInfo bufInfo = {};
        bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufInfo.pNext = NULL;
        bufInfo.usage = p_UsageFlags;
        bufInfo.size = p_VulkanBuffer.m_DataSize;
        bufInfo.queueFamilyIndexCount = 0;
        bufInfo.pQueueFamilyIndices = NULL;
        bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufInfo.flags = 0;
        result = vkCreateBuffer(p_Device, &bufInfo, NULL, &p_VulkanBuffer.m_Buffer);
    }
    assert(result == VK_SUCCESS);

    // 2. Get memory specific requirements
    /**************************************************************/

    // 2a. Get the Buffer resource requirements
    vkGetBufferMemoryRequirements(p_Device, p_VulkanBuffer.m_Buffer, &p_VulkanBuffer.m_MemRqrmnt);

    // 2b. Get the compatible type of memory
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = NULL;
    allocInfo.memoryTypeIndex = 0;
    allocInfo.allocationSize = p_VulkanBuffer.m_MemRqrmnt.size;

    if (!VulkanHelper::MemoryTypeFromProperties(p_DeviceMemProp, p_VulkanBuffer.m_MemRqrmnt.memoryTypeBits,
        p_VulkanBuffer.m_MemoryFlags, &allocInfo.memoryTypeIndex))
    {
        LogError("Failed to match compatible memory!");
        assert(0);
    }

    // 3. Allocate the physical backing
    /******************************************************/
    result = vkAllocateMemory(p_Device, &allocInfo, NULL, &p_VulkanBuffer.m_Memory);
    assert(result == VK_SUCCESS);

    // 4. Bind the allocated buffer resource to the device memory
    result = vkBindBufferMemory(p_Device, p_VulkanBuffer.m_Buffer, p_VulkanBuffer.m_Memory, 0);
    assert(result == VK_SUCCESS);

    if (p_Data)
    {
        WriteBuffer(p_Device, p_Data, p_VulkanBuffer);
    }
}

void VulkanHelper::CreateStagingBuffer(const VkDevice p_Device, VkPhysicalDeviceMemoryProperties p_DeviceMemProp, VkCommandPool& p_CmdPool, const VkQueue& p_Queue, VulkanBuffer& p_VulkanBuffer, VkBufferUsageFlags p_UsageFlags, const void* p_Data, VkBufferCreateInfo* p_pBufInfo)
{
    VulkanBuffer stageBuffer;

    // Create staging buffer
    stageBuffer.m_DataSize = p_VulkanBuffer.m_DataSize;
    stageBuffer.m_MemoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    VulkanHelper::CreateBuffer(p_Device, p_DeviceMemProp, stageBuffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, p_Data, p_pBufInfo);

    // Create Device Local Buffers
    p_VulkanBuffer.m_MemoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT; // Ensure, it must be device local
    if (p_VulkanBuffer.m_Buffer == VK_NULL_HANDLE)
    {
        VulkanHelper::CreateBuffer(p_Device, p_DeviceMemProp, p_VulkanBuffer, p_UsageFlags, p_Data, p_pBufInfo);
    }

    // Copy staging buffers in device local buffer
    {
        VkCommandBuffer copyCmd;
        VulkanHelper::AllocateCommandBuffer(p_Device, p_CmdPool, &copyCmd);
        VulkanHelper::BeginCommandBuffer(copyCmd);

        VkBufferCopy copyRegion = {};
        copyRegion.size = p_VulkanBuffer.m_DataSize;
        vkCmdCopyBuffer(copyCmd, stageBuffer.m_Buffer, p_VulkanBuffer.m_Buffer, 1, &copyRegion);

        VulkanHelper::EndCommandBuffer(copyCmd);
        VulkanHelper::SubmitCommandBuffer(p_Queue, copyCmd);
        vkFreeCommandBuffers(p_Device, p_CmdPool, 1, &copyCmd);
    }

    vkDestroyBuffer(p_Device, stageBuffer.m_Buffer, nullptr);
    vkFreeMemory(p_Device, stageBuffer.m_Memory, nullptr);
}

void VulkanHelper::CreateStagingBufferCopyRegion(const VkDevice p_Device, VkPhysicalDeviceMemoryProperties p_DeviceMemProp, VkCommandPool& p_CmdPool,
                                                 const VkQueue& p_Queue, VulkanBuffer& p_VulkanBuffer, VkBufferUsageFlags p_UsageFlags, const void* p_Data,
                                                 size_t p_DataSize, VkBufferCreateInfo* p_pBufInfo, const std::vector<VkBufferCopy>& p_CopyRegion)
{
    VulkanBuffer stageBuffer;

    // Create staging buffer
    stageBuffer.m_DataSize = p_DataSize;
    stageBuffer.m_MemoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    VulkanHelper::CreateBuffer(p_Device, p_DeviceMemProp, stageBuffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, p_Data, p_pBufInfo);

    // Create Device Local Buffers
    p_VulkanBuffer.m_MemoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT; // Ensure, it must be device local
    if (p_VulkanBuffer.m_Buffer == VK_NULL_HANDLE)
    {
        VulkanHelper::CreateBuffer(p_Device, p_DeviceMemProp, p_VulkanBuffer, p_UsageFlags, p_Data, p_pBufInfo);
    }

    // Copy staging buffers in device local buffer
    {
        VkCommandBuffer copyCmd;
        VulkanHelper::AllocateCommandBuffer(p_Device, p_CmdPool, &copyCmd);
        VulkanHelper::BeginCommandBuffer(copyCmd);

        vkCmdCopyBuffer(copyCmd, stageBuffer.m_Buffer, p_VulkanBuffer.m_Buffer, p_CopyRegion.size(), p_CopyRegion.data());

        VulkanHelper::EndCommandBuffer(copyCmd);
        VulkanHelper::SubmitCommandBuffer(p_Queue, copyCmd);
        vkFreeCommandBuffers(p_Device, p_CmdPool, 1, &copyCmd);
    }

    vkDestroyBuffer(p_Device, stageBuffer.m_Buffer, nullptr);
    vkFreeMemory(p_Device, stageBuffer.m_Memory, nullptr);
}

bool VulkanHelper::WriteBuffer(const VkDevice p_Device, const void* p_VertexData, const VulkanBuffer& p_VulkanBuffer)
{
    if (!p_VertexData) return false;

    // NOTE: For the existing cases this function needs to grow overtime to accound for memoryTypeFlag.
    // The below code preassume the buffer to be host visible

    // 1. Copy data into buffer
    /**************************/
    if (p_VulkanBuffer.m_MemoryFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
    {
        // 1a. Map the physical device memory region to the host
        uint8_t *pData;
        VkResult  result = vkMapMemory(p_Device, p_VulkanBuffer.m_Memory, 0, p_VulkanBuffer.m_MemRqrmnt.size, 0, (void **)&pData);
        assert(result == VK_SUCCESS);

        // 1b. Copy the data in the mapped memory
        memcpy(pData, p_VertexData, p_VulkanBuffer.m_DataSize);

        // 1c. Unmap the device memory
        vkUnmapMemory(p_Device, p_VulkanBuffer.m_Memory);

        return true;
    }

    return false;
}

void VulkanHelper::MapMemory(const VkDevice p_Device, const VkDeviceMemory& p_Memory, VkDeviceSize p_Offset, VkDeviceSize p_Size, VkMemoryMapFlags p_Flags, uint8_t*& p_MappedMemory)
{
    VkResult  result = vkMapMemory(p_Device, p_Memory, p_Offset, p_Size, p_Flags, (void **)&p_MappedMemory);
    assert(result == VK_SUCCESS);
}

void VulkanHelper::WriteMemory(const VkDevice p_Device, void* p_MappedMemory, const std::vector<VkMappedMemoryRange>& p_MappedRange, VkMemoryMapFlags p_Flags, void const* p_Src, size_t p_Size)
{
    VkResult result = vkInvalidateMappedMemoryRanges(p_Device, static_cast<uint32_t>(p_MappedRange.size()), &p_MappedRange[0]);
    assert(result == VK_SUCCESS);

    // Copy updated data into the mapped memory
    memcpy(p_MappedMemory, p_Src, p_Size);

    result = vkFlushMappedMemoryRanges(p_Device, 1, &p_MappedRange[0]);
    assert(result == VK_SUCCESS);
}

void VulkanHelper::CreateImage(const VkDevice device, VkPhysicalDeviceMemoryProperties deviceMemProp, VulkanImage& p_VulkanImage, VkImageCreateInfo* pImageInfo)
{
    VkResult result;
    if (pImageInfo)
    {
        result = vkCreateImage(device, pImageInfo, nullptr, &p_VulkanImage.out.image);
    }
    else
    {
        VkImageCreateInfo imageInfo;
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.pNext = NULL;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        imageInfo.extent = p_VulkanImage.in.extent;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.queueFamilyIndexCount = 0;
        imageInfo.pQueueFamilyIndices = NULL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.usage = (VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
        result = vkCreateImage(device, &imageInfo, nullptr, &p_VulkanImage.out.image);
    }

    assert(result == VK_SUCCESS);

    vkGetImageMemoryRequirements(device, p_VulkanImage.out.image, &p_VulkanImage.out.memRqrmnt);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = p_VulkanImage.out.memRqrmnt.size;

    VulkanHelper::MemoryTypeFromProperties(deviceMemProp, p_VulkanImage.out.memRqrmnt.memoryTypeBits,
                                           p_VulkanImage.in.memoryFlags, &allocInfo.memoryTypeIndex);

    result = vkAllocateMemory(device, &allocInfo, nullptr, &p_VulkanImage.out.deviceMemory);
    assert(result == VK_SUCCESS);

    vkBindImageMemory(device, p_VulkanImage.out.image, p_VulkanImage.out.deviceMemory, 0);
}

void VulkanHelper::CreateImage(const VkDevice device, VkPhysicalDeviceMemoryProperties deviceMemProp,
    VkMemoryPropertyFlags imageMemProp, VkImageCreateInfo* pImageInfo, VkImage* pImage, VkDeviceMemory* pImageMemory)
{
    VkResult result = vkCreateImage(device, pImageInfo, nullptr, pImage);
    assert(result == VK_SUCCESS);

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, *pImage, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;

    VulkanHelper::MemoryTypeFromProperties(deviceMemProp, memRequirements.memoryTypeBits,
        imageMemProp, &allocInfo.memoryTypeIndex);

    result = vkAllocateMemory(device, &allocInfo, nullptr, pImageMemory);
    assert(result == VK_SUCCESS);

    vkBindImageMemory(device, *pImage, *pImageMemory, 0);
}

VkImageView VulkanHelper::CreateImageView(const VkDevice device, VkImage image, VkImageViewType type /*= VK_IMAGE_VIEW_TYPE_2D*/, VkFormat format /*= VK_FORMAT_R8G8B8A8_UNORM*/)
{
    VkImageView imageView = nullptr;

    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = type; //  VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format; // VK_FORMAT_R8G8B8A8_UNORM;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkResult result = vkCreateImageView(device, &viewInfo, nullptr, &imageView);

    assert(result == VK_SUCCESS);

    return imageView;
}

void VulkanHelper::CreateImageView(const VkDevice p_Device, VulkanImageView& p_ImageView, const VkImageViewCreateInfo* p_pImageViewCreateInfo)
{
    VkResult result;
    if (p_pImageViewCreateInfo)
    {
        result = vkCreateImageView(p_Device, p_pImageViewCreateInfo, nullptr, &p_ImageView.imageView);
    }
    else
    {
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = *p_ImageView.pImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;
        result = vkCreateImageView(p_Device, &viewInfo, nullptr, &p_ImageView.imageView);
    }

    assert(result == VK_SUCCESS);
}

void VulkanHelper::CreateStagingImage(const VkDevice p_Device, VkPhysicalDeviceMemoryProperties p_DeviceMemProp, VkCommandPool& p_CmdPool, const VkQueue& p_Queue, VulkanImage& p_VulkanImage, const void* p_Data, VkImageCreateInfo* p_ImageInfo, VkBufferImageCopy* p_Region)
{
    VulkanBuffer stageBuffer;

    // Create staging buffer
    stageBuffer.m_DataSize = p_VulkanImage.in.dataSize;
    stageBuffer.m_MemoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    VulkanHelper::CreateBuffer(p_Device, p_DeviceMemProp, stageBuffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, p_Data);

    // Create Device Local Buffers
    p_VulkanImage.in.memoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT; // Ensure, it must be device local
    VulkanHelper::CreateImage(p_Device, p_DeviceMemProp, p_VulkanImage, p_ImageInfo);

    // Copy staging buffers in device local buffer
    {
        VkCommandBuffer copyCmd;
        VulkanHelper::AllocateCommandBuffer(p_Device, p_CmdPool, &copyCmd);
        VulkanHelper::BeginCommandBuffer(copyCmd);

        VulkanHelper::SetImageLayout(p_VulkanImage.out.image, p_VulkanImage.in.imageAspectType,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, (VkAccessFlagBits)0, copyCmd);

        // Copy buffer to image
        vkCmdCopyBufferToImage(copyCmd, stageBuffer.m_Buffer, p_VulkanImage.out.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, p_Region);

        VulkanHelper::SetImageLayout(p_VulkanImage.out.image, p_VulkanImage.in.imageAspectType,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, (VkAccessFlagBits)0, copyCmd);

        VulkanHelper::EndCommandBuffer(copyCmd);
        VulkanHelper::SubmitCommandBuffer(p_Queue, copyCmd);
        vkFreeCommandBuffers(p_Device, p_CmdPool, 1, &copyCmd);
    }

    vkDestroyBuffer(p_Device, stageBuffer.m_Buffer, nullptr);
    vkFreeMemory(p_Device, stageBuffer.m_Memory, nullptr);
}

VkPipelineDepthStencilStateCreateInfo VulkanHelper::PipelineDepthStencilStateCreateInfo()
{
    VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo = {};
    depthStencilStateInfo.sType                                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilStateInfo.flags                                 = 0;
    depthStencilStateInfo.depthTestEnable                       = true;
    depthStencilStateInfo.depthWriteEnable                      = true;
    depthStencilStateInfo.depthCompareOp                        = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencilStateInfo.depthBoundsTestEnable                 = VK_FALSE;
    depthStencilStateInfo.stencilTestEnable                     = VK_FALSE;
    depthStencilStateInfo.back.failOp                           = VK_STENCIL_OP_KEEP;
    depthStencilStateInfo.back.passOp                           = VK_STENCIL_OP_KEEP;
    depthStencilStateInfo.back.compareOp                        = VK_COMPARE_OP_ALWAYS;
    depthStencilStateInfo.back.compareMask                      = 0;
    depthStencilStateInfo.back.reference                        = 0;
    depthStencilStateInfo.back.depthFailOp                      = VK_STENCIL_OP_KEEP;
    depthStencilStateInfo.back.writeMask                        = 0;
    depthStencilStateInfo.minDepthBounds                        = 0;
    depthStencilStateInfo.maxDepthBounds                        = 0;
    depthStencilStateInfo.stencilTestEnable                     = VK_FALSE;
    depthStencilStateInfo.front                                 = depthStencilStateInfo.back;
    depthStencilStateInfo.pNext                                 = NULL;

    return depthStencilStateInfo;
}

VkPipelineRasterizationStateCreateInfo VulkanHelper::PipelineRasterizationStateCreateInfo()
{
    VkPipelineRasterizationStateCreateInfo rasterizer           = {};
    rasterizer.sType                                            = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable                                 = VK_FALSE;
    rasterizer.rasterizerDiscardEnable                          = VK_FALSE;
    rasterizer.polygonMode                                      = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth                                        = 1.0f;
    rasterizer.cullMode                                         = VK_CULL_MODE_NONE;
    rasterizer.frontFace                                        = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable                                  = VK_FALSE;
    rasterizer.depthClampEnable                                 = true;
    rasterizer.pNext                                            = NULL;

    return rasterizer;
}

VkPipelineMultisampleStateCreateInfo VulkanHelper::PipelineMultisampleStateCreateInfo()
{
    VkPipelineMultisampleStateCreateInfo multisampling          = {};
    multisampling.sType                                         = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable                           = VK_FALSE;
    multisampling.rasterizationSamples                          = VK_SAMPLE_COUNT_1_BIT;
    multisampling.pNext                                         = NULL;

    return multisampling;
}

VkPipelineColorBlendAttachmentState VulkanHelper::PipelineColorBlendAttachmentState()
{
    VkPipelineColorBlendAttachmentState colorBlendAttachment    = {};
    colorBlendAttachment.colorWriteMask                         = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable                            = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor                    = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor                    = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp                           = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor                    = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstAlphaBlendFactor                    = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.alphaBlendOp                           = VK_BLEND_OP_ADD;

    return colorBlendAttachment;
}

VkPipelineColorBlendStateCreateInfo VulkanHelper::PipelineColorBlendStateCreateInfo()
{
    VkPipelineColorBlendStateCreateInfo colorBlending           = {};
    colorBlending.sType                                         = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable                                 = VK_FALSE;
    colorBlending.logicOp                                       = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount                               = 1;
    colorBlending.pAttachments                                  = NULL;
    colorBlending.blendConstants[0]                             = 0.0f;
    colorBlending.blendConstants[1]                             = 0.0f;
    colorBlending.blendConstants[2]                             = 0.0f;
    colorBlending.blendConstants[3]                             = 0.0f;
    colorBlending.pNext                                         = NULL;

    return colorBlending;
}

VkPipelineVertexInputStateCreateInfo VulkanHelper::PipelineVertexInputStateCreateInfo()
{
    VkPipelineVertexInputStateCreateInfo vertexInputInfo        = {};
    vertexInputInfo.sType                                       = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount               = 0;
    vertexInputInfo.pVertexBindingDescriptions                  = NULL;
    vertexInputInfo.vertexAttributeDescriptionCount             = 0;
    vertexInputInfo.pVertexAttributeDescriptions                = NULL;
    vertexInputInfo.pNext                                       = NULL;

    return vertexInputInfo;
}

VkPipelineInputAssemblyStateCreateInfo VulkanHelper::PipelineInputAssemblyStateCreateInfo()
{
    VkPipelineInputAssemblyStateCreateInfo inputAssembly        = {};
    inputAssembly.sType                                         = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology                                      = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable                        = VK_FALSE;
    inputAssembly.pNext                                         = NULL;

    return inputAssembly;
}

VkPipelineViewportStateCreateInfo VulkanHelper::PipelineViewportStateCreateInfo()
{
    VkPipelineViewportStateCreateInfo viewportState             = {};
    viewportState.sType                                         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount                                 = 0;
    viewportState.pViewports                                    = NULL;
    viewportState.scissorCount                                  = 0;
    viewportState.pScissors                                     = NULL;
    viewportState.pNext                                         = NULL;

    return viewportState;
}

VkPipelineLayoutCreateInfo VulkanHelper::PipelineLayoutCreateInfo()
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo               = {};
    pipelineLayoutInfo.sType                                    = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount                           = 0;
    pipelineLayoutInfo.pushConstantRangeCount                   = 0;
    pipelineLayoutInfo.pPushConstantRanges                      = 0;
    pipelineLayoutInfo.setLayoutCount                           = 0;
    pipelineLayoutInfo.pSetLayouts                              = NULL;
    pipelineLayoutInfo.pNext                                    = NULL;

    return pipelineLayoutInfo;
}

VkGraphicsPipelineCreateInfo VulkanHelper::GraphicsPipelineCreateInfo()
{
    VkGraphicsPipelineCreateInfo pipelineInfo                   = {};
    pipelineInfo.sType                                          = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount                                     = 0;
    pipelineInfo.pStages                                        = VK_NULL_HANDLE;
    pipelineInfo.pVertexInputState                              = VK_NULL_HANDLE;
    pipelineInfo.pInputAssemblyState                            = VK_NULL_HANDLE;
    pipelineInfo.pTessellationState                             = VK_NULL_HANDLE;
    pipelineInfo.pViewportState                                 = VK_NULL_HANDLE;
    pipelineInfo.pRasterizationState                            = VK_NULL_HANDLE;
    pipelineInfo.pMultisampleState                              = VK_NULL_HANDLE;
    pipelineInfo.pColorBlendState                               = VK_NULL_HANDLE;
    pipelineInfo.pDynamicState                                  = VK_NULL_HANDLE;
    pipelineInfo.layout                                         = VK_NULL_HANDLE;
    pipelineInfo.renderPass                                     = VK_NULL_HANDLE;
    pipelineInfo.subpass                                        = 0;
    pipelineInfo.basePipelineHandle                             = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex                              = 0;
    pipelineInfo.pDepthStencilState                             = VK_NULL_HANDLE;
    pipelineInfo.pNext                                          = NULL;

    return pipelineInfo;
}

VkPipelineDynamicStateCreateInfo VulkanHelper::PipelineDynamicStateCreateInfo()
{
    VkPipelineDynamicStateCreateInfo dynamicState               = {};
    dynamicState.sType                                          = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pDynamicStates                                 = VK_NULL_HANDLE;
    dynamicState.dynamicStateCount                              = 0;
    dynamicState.pNext                                          = NULL;

    return dynamicState;
}

VkPipelineShaderStageCreateInfo VulkanHelper::PipelineShaderStageCreateInfo()
{
    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType                       = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage                       = VK_SHADER_STAGE_VERTEX_BIT;
    fragShaderStageInfo.module                      = VK_NULL_HANDLE;
    fragShaderStageInfo.pName                       = "DummyDefaultMain";

    return fragShaderStageInfo;
}
