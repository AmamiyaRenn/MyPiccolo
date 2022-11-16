#pragma once

#include "function/render/interface/rhi_struct.h"
#include "vulkan/vulkan_core.h"

namespace Piccolo
{
    class VulkanQueue : public RHIQueue
    {
    public:
        void    setResource(VkQueue res) { m_resource = res; }
        VkQueue getResource() const { return m_resource; }

    private:
        VkQueue m_resource;
    };

    class VulkanImageView : public RHIImageView
    {
    public:
        void        setResource(VkImageView res) { m_resource = res; }
        VkImageView getResource() const { return m_resource; }

    private:
        VkImageView m_resource;
    };
} // namespace Piccolo