#pragma once

#include "vulkan/vulkan_core.h"

#include <vector>

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

        static VkShaderModule createShaderModule(VkDevice device, const std::vector<unsigned char>& shader_code);
    };
} // namespace Piccolo