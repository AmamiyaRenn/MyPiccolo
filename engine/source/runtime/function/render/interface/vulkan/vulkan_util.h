#pragma once

#include "function/render/interface/rhi.h"
#include "vulkan/vulkan_core.h"

#include <vector>

namespace Piccolo
{
    class VulkanUtil
    {
    public:
        static VkImageView    createImageView(VkDevice           device,
                                              VkImage&           image,
                                              VkFormat           format,
                                              VkImageAspectFlags image_aspect_flags,
                                              VkImageViewType    view_type,
                                              uint32_t           layout_count,
                                              uint32_t           miplevels);
        static VkShaderModule createShaderModule(VkDevice device, const std::vector<unsigned char>& shader_code);
        static void           createBuffer(VkPhysicalDevice      physical_device,
                                           VkDevice              device,
                                           VkDeviceSize          size,
                                           VkBufferUsageFlags    usage,
                                           VkMemoryPropertyFlags properties,
                                           VkBuffer&             buffer,
                                           VkDeviceMemory&       buffer_memory);
        static uint32_t
        findMemoryType(VkPhysicalDevice physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties_flag);
        static void copyBuffer(RHI*         rhi,
                               VkBuffer     srcBuffer,
                               VkBuffer     dstBuffer,
                               VkDeviceSize srcOffset,
                               VkDeviceSize dstOffset,
                               VkDeviceSize size);
    };
} // namespace Piccolo