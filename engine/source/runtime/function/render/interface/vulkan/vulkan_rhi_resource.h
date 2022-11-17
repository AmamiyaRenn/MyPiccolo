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
    class VulkanShader : public RHIShader
    {
    public:
        void           setResource(VkShaderModule res) { m_resource = res; }
        VkShaderModule getResource() const { return m_resource; }

    private:
        VkShaderModule m_resource;
    };

    class VulkanPipelineLayout : public RHIPipelineLayout
    {
    public:
        void             setResource(VkPipelineLayout res) { m_resource = res; }
        VkPipelineLayout getResource() const { return m_resource; }

    private:
        VkPipelineLayout m_resource;
    };
} // namespace Piccolo