#pragma once
#include "../Rectangle/RectangleDescriptorSet.h"

struct CircleDescriptorSet : public RectangleDescriptorSet
{
    CircleDescriptorSet(BaseRenderer* p_VulkanApplication);
};
