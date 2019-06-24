#pragma once
#include "../engine/vulkan/core/VulkanApp.h"

#include "../common/SceneGraph/Transformation.h"
#include "../common/SceneGraph/Scene.h"
#include "../common/SceneGraph/Node.h"

struct RectangleDescriptorSet
{
    struct UniformBufferObj {
        UniformBufferObj();

        VulkanBuffer                    m_BufObj;
        VkDescriptorBufferInfo          m_DescriptorBufInfo;// Descriptor buffer info that need to supplied into write descriptor set (VkWriteDescriptorSet)
        std::vector<VkMappedMemoryRange>m_MappedRange;      // Metadata of memory mapped objects
        uint8_t*                        m_MappedMemory;     // Host pointer containing the mapped device address which is used to write data into.
        size_t                          m_DataSize;         // Data size.
    };

    // Used in instacing, moved into derived class
    struct UBORect {
        glm::mat4 m_ProjViewMat;
        float m_Time;
        int m_DirtyTest; // 1: true, 0: false
    } uboRectParameters;

    RectangleDescriptorSet(BaseRenderer* p_VulkanApplication);

    ~RectangleDescriptorSet();

    void CreateUniformBuffer();
    void DestroyUniformBuffer();

    void CreateDescriptorSetLayout();
    void DestroyDescriptorLayout();

    UniformBufferObj* UniformBuffer;

    // List of all the VkDescriptorSetLayouts
    std::vector<VkDescriptorSetLayout> descLayout;

    // List of all created VkDescriptorSet
    std::vector<VkDescriptorSet> descriptorSet;

private:
    void CreateDescriptor();

    // Creates the descriptor pool, this function depends on -
    void CreateDescriptorPool();
    // Creates the descriptor sets using descriptor pool.
    // This function depend on the createDescriptorPool() and createUniformBuffer().
    void CreateDescriptorSet();

    // Decriptor pool object that will be used for allocating VkDescriptorSet object
    VkDescriptorPool descriptorPool;

    VulkanApp* m_VulkanApplication;
};
