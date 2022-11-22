#include "function/render/interface/vulkan/vulkan_util.h"
#include "core/base/macro.h"
#include "function/render/interface/vulkan/vulkan_rhi.h"
#include "vulkan/vulkan_core.h"

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
        {
            LOG_ERROR("Failed to create image views!");
            return VK_NULL_HANDLE;
        }

        return image_view;
    }

    VkShaderModule VulkanUtil::createShaderModule(VkDevice device, const std::vector<unsigned char>& shader_code)
    {
        VkShaderModuleCreateInfo shader_module_create_info {};
        shader_module_create_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shader_module_create_info.codeSize = shader_code.size();
        shader_module_create_info.pCode    = reinterpret_cast<const uint32_t*>(shader_code.data());

        VkShaderModule shader_module;
        if (vkCreateShaderModule(device, &shader_module_create_info, nullptr, &shader_module) != VK_SUCCESS)
        {
            LOG_ERROR("Failed to create shader module!");
            return VK_NULL_HANDLE;
        }
        return shader_module;
    }

    void VulkanUtil::createBuffer(VkPhysicalDevice      physical_device,
                                  VkDevice              device,
                                  VkDeviceSize          size,
                                  VkBufferUsageFlags    usage,
                                  VkMemoryPropertyFlags properties,
                                  VkBuffer&             buffer,
                                  VkDeviceMemory&       buffer_memory)
    {
        // create buffer
        VkBufferCreateInfo buffer_create_info {};
        buffer_create_info.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_create_info.size        = size;
        buffer_create_info.usage       = usage;                     // use as a vertex/staging/index buffer
        buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // not sharing among queue families
        if (vkCreateBuffer(device, &buffer_create_info, nullptr, &buffer) != VK_SUCCESS)
        {
            LOG_ERROR("vkCreateBuffer failed!");
            return;
        }

        // get buffer memory requirements(for allocate_info.allocationSize and allocate_info.memoryTypeIndex)
        VkMemoryRequirements buffer_memory_requirements;
        vkGetBufferMemoryRequirements(device, buffer, &buffer_memory_requirements);

        // find the right memory type
        VkMemoryAllocateInfo buffer_memory_allocate_info {};
        buffer_memory_allocate_info.sType          = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        buffer_memory_allocate_info.allocationSize = buffer_memory_requirements.size;
        buffer_memory_allocate_info.memoryTypeIndex =
            VulkanUtil::findMemoryType(physical_device, buffer_memory_requirements.memoryTypeBits, properties);

        // allocate the device memory
        if (vkAllocateMemory(device, &buffer_memory_allocate_info, nullptr, &buffer_memory) != VK_SUCCESS)
        {
            LOG_ERROR("vkAllocateMemory failed!");
            return;
        }

        // bind buffer with device memory
        vkBindBufferMemory(device, buffer, buffer_memory, 0); // offset = 0
    }

    uint32_t VulkanUtil::findMemoryType(VkPhysicalDevice      physical_device,
                                        uint32_t              type_filter,
                                        VkMemoryPropertyFlags properties_flag)
    {
        VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
        vkGetPhysicalDeviceMemoryProperties(physical_device, &physical_device_memory_properties);

        for (uint32_t i = 0; i < physical_device_memory_properties.memoryTypeCount; i++)
            if (type_filter & (1 << i) &&
                (physical_device_memory_properties.memoryTypes[i].propertyFlags & properties_flag) == properties_flag)
                return i;

        LOG_ERROR("findMemoryType error");
        return 0;
    }

    void VulkanUtil::copyBuffer(RHI*         rhi,
                                VkBuffer     srcBuffer,
                                VkBuffer     dstBuffer,
                                VkDeviceSize srcOffset,
                                VkDeviceSize dstOffset,
                                VkDeviceSize size)
    {
        if (rhi == nullptr)
        {
            LOG_ERROR("rhi is nullptr");
            return;
        }

        RHICommandBuffer* rhi_command_buffer = static_cast<VulkanRHI*>(rhi)->beginSingleTimeCommands();
        VkCommandBuffer   command_buffer     = static_cast<VulkanCommandBuffer*>(rhi_command_buffer)->getResource();

        VkBufferCopy copy_region = {srcOffset, dstOffset, size};
        vkCmdCopyBuffer(command_buffer, srcBuffer, dstBuffer, 1, &copy_region);

        static_cast<VulkanRHI*>(rhi)->endSingleTimeCommands(rhi_command_buffer);
    }
} // namespace Piccolo
