#pragma once
#include <vulkan/vulkan.h>

#include <assert.h>
#include <fstream>
#include <iostream>
#include <string>
#include <set>
#include <vector>
#include <iomanip>

#include "../common/SceneGraph/Common.h"

// A structure to store physical device information
struct PhysicalDeviceInfo
{
    VkPhysicalDeviceProperties              prop = {};
    VkPhysicalDeviceFeatures                features = {};
    VkPhysicalDeviceMemoryProperties        memProp = {};
    std::vector<VkQueueFamilyProperties>    familyPropList;
    int                                     graphicsFamilyIndex = -1;
    int                                     presentFamilyIndex = -1;
    std::vector<VkExtensionProperties>      extensionList;
    VkSurfaceCapabilitiesKHR                capabilities = {};
    std::vector<VkSurfaceFormatKHR>         formatList;
    std::vector<VkPresentModeKHR>           presentModeList;
};

struct LayerProperties
{
    VkLayerProperties                       properties;
    std::vector<VkExtensionProperties>      extensions;
};

// TODO: remove the m_ prefix from the structures
struct VulkanBuffer
{
    VulkanBuffer() { memset(this, 0, sizeof(VulkanBuffer)); }
    VkBuffer				m_Buffer;			// Buffer resource object
    uint64_t				m_DataSize;			// Actual data size request for, use m_MemRqrmnt.size for actual backing size
    VkDeviceMemory			m_Memory;			// Buffer resource object's allocated device memory
    VkMemoryRequirements	m_MemRqrmnt;		// Memory requirement for the allocation buffer, useful in mapping/unmapping
    VkMemoryPropertyFlags	m_MemoryFlags;		// Memory properties flags
};

struct VulkanImage
{
    struct
    {
        uint64_t				dataSize;		// Data size for linear buffer, to be used for staging or copying data between buffer and images.
        VkExtent3D				extent;         // Image extent
        VkMemoryPropertyFlags   memoryFlags;	// Memory properties flags
        VkImageAspectFlagBits   imageAspectType;// Color, Depth, Stencil
    } in;
    struct
    {
        VkImage					image;          // Image resource object
        VkDeviceMemory			deviceMemory;   // Image resource object's allocated device memory
        VkMemoryRequirements	memRqrmnt;      // Retrived Memory requirement for the image allocation
    } out;
};

struct VulkanImageView
{
    VkImageView				imageView;
    VkImage*				pImage = NULL;	// Image resource object
};

class VulkanHelper
{
public:
    VulkanHelper();
    ~VulkanHelper();

    // Create Swap chain helper functions
    static VkSurfaceFormatKHR SelectBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    static VkPresentModeKHR SelectBestPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
    static VkExtent2D SelectBestExtent(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D& windowDim);

    // Layer and Extension helper functions
    static VkResult GetInstanceLayerExtensionProperties();
    static VkResult GetDeviceLayerExtensionProperties(VkPhysicalDevice gpu);
    static VkResult GetExtensionProperties(LayerProperties &layerProps, VkPhysicalDevice gpu = nullptr);

    // General helper functions
    static void LogError(string text);

    // Shader helper funcitons
    static VkShaderModule CreateShader(VkDevice device, const std::string& filename);
    static VkShaderModule CreateShaderFromQRCResource(VkDevice device, const std::string& filename);

    static bool MemoryTypeFromProperties(VkPhysicalDeviceMemoryProperties memoryProperties, uint32_t typeBits, VkFlags requirementsMask, uint32_t *typeIndex);

    static void CreateCommandPool(const VkDevice& device, VkCommandPool& cmdPool, const PhysicalDeviceInfo& deviceInfo, const VkCommandPoolCreateInfo* commandBufferInfo = NULL);
    static void AllocateCommandBuffer(const VkDevice device, const VkCommandPool cmdPool, VkCommandBuffer* cmdBuf, const VkCommandBufferAllocateInfo* commandBufferInfo = NULL);
    static void BeginCommandBuffer(VkCommandBuffer cmdBuf, VkCommandBufferBeginInfo* inCmdBufInfo = NULL);
    static void EndCommandBuffer(VkCommandBuffer cmdBuf);
    static void SubmitCommandBuffer(const VkQueue& queue, const VkCommandBuffer cmdBufList, const VkSubmitInfo* submitInfo = NULL, const VkFence& fence = VK_NULL_HANDLE);
    static void SetImageLayout(VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkAccessFlagBits srcAccessMask, const VkCommandBuffer& commandBuffer);

    static void CreateBuffer(const VkDevice p_Device, VkPhysicalDeviceMemoryProperties p_DeviceMemProp, VulkanBuffer& p_VulkanBuffer, VkBufferUsageFlags p_UsageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, const void* p_Data = NULL, VkBufferCreateInfo* p_pBufInfo = NULL); // Please use this Create Buffer currently begin used
    static void CreateStagingBuffer(const VkDevice p_Device, VkPhysicalDeviceMemoryProperties p_DeviceMemProp, VkCommandPool& p_CmdPool, const VkQueue& p_Queue, VulkanBuffer& p_VulkanBuffer, VkBufferUsageFlags p_UsageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, const void* p_Data = NULL, VkBufferCreateInfo* p_pBufInfo = NULL); // Please use this Create Buffer currently begin used
    static void CreateStagingBufferCopyRegion(const VkDevice p_Device, VkPhysicalDeviceMemoryProperties p_DeviceMemProp, VkCommandPool& p_CmdPool, const VkQueue& p_Queue, VulkanBuffer& p_VulkanBuffer, VkBufferUsageFlags p_UsageFlags, const void* p_Data, size_t p_DataSize, VkBufferCreateInfo* p_pBufInfo, const std::vector<VkBufferCopy>& p_CopyRegion);
    static void MapMemory(const VkDevice p_Device, const VkDeviceMemory& p_Memory, VkDeviceSize p_Offset, VkDeviceSize p_Size, VkMemoryMapFlags flags, uint8_t*& p_MappedMemory);
    static void WriteMemory(const VkDevice p_Device, void* p_MappedMemory, const std::vector<VkMappedMemoryRange>& p_MappedRange, VkMemoryMapFlags p_Flags, void const* p_Src, size_t p_Size);

    static void CreateImage(const VkDevice device, VkPhysicalDeviceMemoryProperties deviceMemProp, VkMemoryPropertyFlags imageMemProp, VkImageCreateInfo* pImageInfo, VkImage* pTextureImage, VkDeviceMemory* pTextureImageMemory);// Parminder: This function marked as obsolete
    static VkImageView CreateImageView(const VkDevice device, VkImage image, VkImageViewType type = VK_IMAGE_VIEW_TYPE_2D, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM);
    static bool SetImageLayoutEx(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, const VkCommandBuffer& commandBuffer);

    static void CreateImage(const VkDevice p_Device, VkPhysicalDeviceMemoryProperties p_DeviceMemProp, VulkanImage& p_VulkanImage, VkImageCreateInfo* p_pImageInfo = NULL);
    static void CreateImageView(const VkDevice p_Device, VulkanImageView& p_ImageView, const VkImageViewCreateInfo* p_pImageViewCreateInfo = NULL);
    static void CreateStagingImage(const VkDevice p_Device, VkPhysicalDeviceMemoryProperties p_DeviceMemProp, VkCommandPool& p_CmdPool, const VkQueue& p_Queue, VulkanImage& p_VulkanImage, const void* p_Data, VkImageCreateInfo* p_ImageInfo, VkBufferImageCopy* p_Region);

    // Pipeline State
    static VkPipelineDepthStencilStateCreateInfo    PipelineDepthStencilStateCreateInfo();
    static VkPipelineRasterizationStateCreateInfo   PipelineRasterizationStateCreateInfo();
    static VkPipelineMultisampleStateCreateInfo     PipelineMultisampleStateCreateInfo();
    static VkPipelineColorBlendAttachmentState      PipelineColorBlendAttachmentState();
    static VkPipelineColorBlendStateCreateInfo      PipelineColorBlendStateCreateInfo();
    static VkPipelineVertexInputStateCreateInfo     PipelineVertexInputStateCreateInfo();
    static VkPipelineInputAssemblyStateCreateInfo   PipelineInputAssemblyStateCreateInfo();
    static VkPipelineViewportStateCreateInfo        PipelineViewportStateCreateInfo();
    static VkPipelineLayoutCreateInfo               PipelineLayoutCreateInfo();
    static VkGraphicsPipelineCreateInfo             GraphicsPipelineCreateInfo();
    static VkPipelineDynamicStateCreateInfo         PipelineDynamicStateCreateInfo();
    static VkPipelineShaderStageCreateInfo          PipelineShaderStageCreateInfo();

    // Data Structs APIs
//    static VkViewport Viewport(float p_X, float p_Y, float p_Width, float p_Height, float p_MinDepth, float p_MaxDepth);

private:
    static bool WriteBuffer(const VkDevice p_Device, const void* p_VertexData, const VulkanBuffer& p_VulkanBuffer);

public:
    // Layer property list containing Layers and respective extensions
    static std::vector<LayerProperties> m_LayerPropertyList;
};

