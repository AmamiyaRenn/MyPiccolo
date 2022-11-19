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
    class VulkanRenderPass : public RHIRenderPass
    {
    public:
        void         setResource(VkRenderPass res) { m_resource = res; }
        VkRenderPass getResource() const { return m_resource; }

    private:
        VkRenderPass m_resource;
    };
    class VulkanPipeline : public RHIPipeline
    {
    public:
        void       setResource(VkPipeline res) { m_resource = res; }
        VkPipeline getResource() const { return m_resource; }

    private:
        VkPipeline m_resource;
    };
    class VulkanPipelineCache : public RHIPipelineCache
    {
    public:
        void            setResource(VkPipelineCache res) { m_resource = res; }
        VkPipelineCache getResource() const { return m_resource; }

    private:
        VkPipelineCache m_resource;
    };
    class VulkanFramebuffer : public RHIFramebuffer
    {
    public:
        void          setResource(VkFramebuffer res) { m_resource = res; }
        VkFramebuffer getResource() const { return m_resource; }

    private:
        VkFramebuffer m_resource;
    };
    class VulkanCommandPool : public RHICommandPool
    {
    public:
        void          setResource(VkCommandPool res) { m_resource = res; }
        VkCommandPool getResource() const { return m_resource; }

    private:
        VkCommandPool m_resource;
    };
    class VulkanCommandBuffer : public RHICommandBuffer
    {
    public:
        void            setResource(VkCommandBuffer res) { m_resource = res; }
        VkCommandBuffer getResource() const { return m_resource; }

    private:
        VkCommandBuffer m_resource;
    };
    class VulkanSemaphore : public RHISemaphore
    {
    public:
        void         setResource(VkSemaphore res) { m_resource = res; }
        VkSemaphore& getResource() { return m_resource; }

    private:
        VkSemaphore m_resource;
    };
} // namespace Piccolo