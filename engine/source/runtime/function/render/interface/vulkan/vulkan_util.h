#pragma once

#include "vulkan/vulkan_core.h"

namespace Piccolo
{
    class VulkanUtil
    {
    public:
        static VkImageView createImageView(VkDevice           device,
                                           VkImage&           image,
                                           VkFormat           format,
                                           VkImageAspectFlags image_aspect_flags,
                                           VkImageViewType    view_type,
                                           uint32_t           layout_count,
                                           uint32_t           miplevels);
    };
} // namespace Piccolo