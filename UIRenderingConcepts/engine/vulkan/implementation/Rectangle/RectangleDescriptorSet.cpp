#include "RectangleDescriptorSet.h"

RectangleDescriptorSet::RectangleDescriptorSet(BaseRenderer* p_VulkanApplication)
{
    m_VulkanApplication = static_cast<VulkanApp*>(p_VulkanApplication);

    UniformBuffer = new UniformBufferObj;

    CreateDescriptor();
}

RectangleDescriptorSet::~RectangleDescriptorSet()
{
    // Destroy descriptors
    for (int i = 0; i < descLayout.size(); i++)
    {
        vkDestroyDescriptorSetLayout(m_VulkanApplication->m_hDevice, descLayout[i], NULL);
    }
    descLayout.clear();

    vkFreeDescriptorSets(m_VulkanApplication->m_hDevice, descriptorPool, (uint32_t)descriptorSet.size(), &descriptorSet[0]);
    vkDestroyDescriptorPool(m_VulkanApplication->m_hDevice, descriptorPool, NULL);
}

RectangleDescriptorSet::UniformBufferObj::UniformBufferObj()
{
    memset(this, 0, sizeof(UniformBufferObj));
}

void RectangleDescriptorSet::CreateUniformBuffer()
{
    UniformBuffer->m_BufObj.m_MemoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
//    UniformBuffer->m_BufObj.m_DataSize = sizeof(glm::mat4);
    UniformBuffer->m_BufObj.m_DataSize = sizeof(UBORect);

    // Create buffer resource states using VkBufferCreateInfo
    VulkanHelper::CreateBuffer(m_VulkanApplication->m_hDevice, m_VulkanApplication->m_physicalDeviceInfo.memProp, UniformBuffer->m_BufObj, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    // Map the GPU memory on to local host
    VulkanHelper::MapMemory(m_VulkanApplication->m_hDevice, UniformBuffer->m_BufObj.m_Memory, 0, UniformBuffer->m_BufObj.m_MemRqrmnt.size, 0, UniformBuffer->m_MappedMemory);

    // We have only one Uniform buffer object to update
    UniformBuffer->m_MappedRange.resize(1);

    // Populate the VkMappedMemoryRange data structure
    UniformBuffer->m_MappedRange[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    UniformBuffer->m_MappedRange[0].memory = UniformBuffer->m_BufObj.m_Memory;
    UniformBuffer->m_MappedRange[0].offset = 0;
    UniformBuffer->m_MappedRange[0].size = UniformBuffer->m_BufObj.m_DataSize;

    // Update descriptor buffer info in order to write the descriptors
    UniformBuffer->m_DescriptorBufInfo.buffer = UniformBuffer->m_BufObj.m_Buffer;
    UniformBuffer->m_DescriptorBufInfo.offset = 0;
    UniformBuffer->m_DescriptorBufInfo.range = UniformBuffer->m_BufObj.m_DataSize;
}

void RectangleDescriptorSet::DestroyUniformBuffer()
{
    vkUnmapMemory(m_VulkanApplication->m_hDevice, UniformBuffer->m_BufObj.m_Memory);
    vkDestroyBuffer(m_VulkanApplication->m_hDevice, UniformBuffer->m_BufObj.m_Buffer, NULL);
    vkFreeMemory(m_VulkanApplication->m_hDevice, UniformBuffer->m_BufObj.m_Memory, NULL);
}

void RectangleDescriptorSet::CreateDescriptorSetLayout()
{
    // Define the layout binding information for the descriptor set(before creating it)
    // Specify binding point, shader type(like vertex shader below), count etc.
    VkDescriptorSetLayoutBinding layoutBindings[1];
    layoutBindings[0].binding = 0; // DESCRIPTOR_SET_BINDING_INDEX
    layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBindings[0].descriptorCount = 1;
    layoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    layoutBindings[0].pImmutableSamplers = NULL;

    // Specify the layout bind into the VkDescriptorSetLayoutCreateInfo
    // and use it to create a descriptor set layout
    VkDescriptorSetLayoutCreateInfo descriptorLayout = {};
    descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorLayout.pNext = NULL;
    descriptorLayout.bindingCount = 1;
    descriptorLayout.pBindings = layoutBindings;

    VkResult  result;
    // Allocate required number of descriptor layout objects and
    // create them using vkCreateDescriptorSetLayout()
    descLayout.resize(1);
    result = vkCreateDescriptorSetLayout(m_VulkanApplication->m_hDevice, &descriptorLayout, NULL, descLayout.data());
    assert(result == VK_SUCCESS);
}

void RectangleDescriptorSet::DestroyDescriptorLayout()
{
    for (int i = 0; i < descLayout.size(); i++) {
        vkDestroyDescriptorSetLayout(m_VulkanApplication->m_hDevice, descLayout[i], NULL);
    }
    descLayout.clear();
}

void RectangleDescriptorSet::CreateDescriptor()
{
    CreateDescriptorSetLayout();
    CreateUniformBuffer();
    CreateDescriptorPool();
    CreateDescriptorSet();
}

void RectangleDescriptorSet::CreateDescriptorPool()
{
    VkResult  result;
    // Define the size of descriptor pool based on the
    // type of descriptor set being used.
    std::vector<VkDescriptorPoolSize> descriptorTypePool;

    // The first descriptor pool object is of type Uniform buffer
    descriptorTypePool.push_back(VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 });

    // Populate the descriptor pool state information
    // in the create info structure.
    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.pNext = NULL;
    descriptorPoolCreateInfo.maxSets = 1;
    descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    descriptorPoolCreateInfo.poolSizeCount = (uint32_t)descriptorTypePool.size();
    descriptorPoolCreateInfo.pPoolSizes = descriptorTypePool.data();

    // Create the descriptor pool using the descriptor
    // pool create info structure
    result = vkCreateDescriptorPool(m_VulkanApplication->m_hDevice, &descriptorPoolCreateInfo, NULL, &descriptorPool);
    assert(result == VK_SUCCESS);
}

void RectangleDescriptorSet::CreateDescriptorSet()
{
    VkResult  result;

    // Create the descriptor allocation structure and specify the descriptor
    // pool and descriptor layout
    VkDescriptorSetAllocateInfo dsAllocInfo[1];
    dsAllocInfo[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    dsAllocInfo[0].pNext = NULL;
    dsAllocInfo[0].descriptorPool = descriptorPool;
    dsAllocInfo[0].descriptorSetCount = 1;
    dsAllocInfo[0].pSetLayouts = descLayout.data();

    // Allocate the number of descriptor sets needs to be produced
    descriptorSet.resize(1);

    // Allocate descriptor sets
    result = vkAllocateDescriptorSets(m_VulkanApplication->m_hDevice, dsAllocInfo, descriptorSet.data());
    assert(result == VK_SUCCESS);

    // Allocate one write descriptors for transformation (MVP)
    VkWriteDescriptorSet writes[1];
    memset(&writes, 0, sizeof(writes));

    // Specify the uniform buffer related
    // information into first write descriptor
    writes[0] = {};
    writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].pNext = NULL;
    writes[0].dstSet = descriptorSet[0];
    writes[0].descriptorCount = 1;
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writes[0].pBufferInfo = &UniformBuffer->m_DescriptorBufInfo;
    writes[0].dstArrayElement = 0;
    writes[0].dstBinding = 0; // DESCRIPTOR_SET_BINDING_INDEX

    // Update the uniform buffer into the allocated descriptor set
    vkUpdateDescriptorSets(m_VulkanApplication->m_hDevice, 1, writes, 0, NULL);
}
