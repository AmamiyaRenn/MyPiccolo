#include "function/render/interface/vulkan/vulkan_util.h"
#include "core/base/macro.h"

namespace Piccolo
{
    VkImageView VulkanUtil::createImageView(VkDevice           device,
                                            VkImage&           image,
                                            VkFormat           format,
                                            VkImageAspectFlags image_aspect_flags,
                                            VkImageViewType    view_type,
                                            uint32_t           layout_count,
                                            uint32_t           miplevels)
    {
        VkImageViewCreateInfo image_view_create_info {};
        image_view_create_info.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.image    = image;
        image_view_create_info.viewType = view_type; // 将图像看作1D纹理、2D纹理、3D纹理或立方体贴图
        image_view_create_info.format   = format;
        image_view_create_info.subresourceRange.aspectMask     = image_aspect_flags;
        image_view_create_info.subresourceRange.baseMipLevel   = 0; // 暂时不考虑Mipmap
        image_view_create_info.subresourceRange.levelCount     = miplevels;
        image_view_create_info.subresourceRange.baseArrayLayer = 0; // 暂时不考虑多图层
        image_view_create_info.subresourceRange.layerCount     = layout_count;

        VkImageView image_view;
        if (vkCreateImageView(device, &image_view_create_info, nullptr, &image_view) != VK_SUCCESS)
            LOG_ERROR("Failed to create image views!");

        return image_view;
    }
} // namespace Piccolo
