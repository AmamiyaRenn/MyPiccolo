#pragma once

#define GLFW_INCLUDE_VULKAN

#include <memory>
#include <vector>

#include "function/render/interface/rhi_struct.h"
#include "function/render/window_system.h"

namespace Piccolo
{
    struct RHIInitInfo
    {
        std::shared_ptr<WindowSystem> m_window_system;
    };

    class RHI
    {
    public:
        // initialize
        virtual void initialize(RHIInitInfo init_info) = 0;
        virtual void prepareContext()                  = 0;

        // allocate and create
        virtual void       createSwapchain()                                                                       = 0;
        virtual void       createSwapchainImageViews()                                                             = 0;
        virtual RHIShader* createShaderModule(const std::vector<unsigned char>& shader_code)                       = 0;
        virtual bool       createGraphicsPipelines(RHIPipelineCache*                    pipelineCache,
                                                   uint32_t                             createInfoCount,
                                                   const RHIGraphicsPipelineCreateInfo* pCreateInfos,
                                                   RHIPipeline*&                        pPipelines)                                       = 0;
        virtual bool       createPipelineLayout(const RHIPipelineLayoutCreateInfo* pCreateInfo,
                                                RHIPipelineLayout*&                pPipelineLayout)                               = 0;
        virtual bool createRenderPass(const RHIRenderPassCreateInfo* pCreateInfo, RHIRenderPass*& pRenderPass)     = 0;
        virtual bool createFramebuffer(const RHIFramebufferCreateInfo* pCreateInfo, RHIFramebuffer*& pFramebuffer) = 0;
        virtual void recreateSwapchain()                                                                           = 0;
        virtual void createBuffer(RHIDeviceSize          size,
                                  RHIBufferUsageFlags    usage,
                                  RHIMemoryPropertyFlags properties,
                                  RHIBuffer*&            buffer,
                                  RHIDeviceMemory*&      buffer_memory)                                                 = 0;

        // command and command write
        virtual void cmdBindVertexBuffersPFN(RHICommandBuffer*    commandBuffer,
                                             uint32_t             firstBinding,
                                             uint32_t             bindingCount,
                                             RHIBuffer* const*    pBuffers,
                                             const RHIDeviceSize* pOffsets) = 0;
        virtual void cmdBeginRenderPassPFN(RHICommandBuffer*             commandBuffer,
                                           const RHIRenderPassBeginInfo* pRenderPassBegin,
                                           RHISubpassContents            contents)     = 0;
        virtual void cmdBindPipelinePFN(RHICommandBuffer*    commandBuffer,
                                        RHIPipelineBindPoint pipelineBindPoint,
                                        RHIPipeline*         pipeline)              = 0;
        virtual void cmdDraw(RHICommandBuffer* commandBuffer,
                             uint32_t          vertexCount,
                             uint32_t          instanceCount,
                             uint32_t          firstVertex,
                             uint32_t          firstInstance)                        = 0;
        virtual void cmdEndRenderPassPFN(RHICommandBuffer* commandBuffer)   = 0;
        virtual void cmdSetViewportPFN(RHICommandBuffer*  commandBuffer,
                                       uint32_t           firstViewport,
                                       uint32_t           viewportCount,
                                       const RHIViewport* pViewports)       = 0;
        virtual void cmdSetScissorPFN(RHICommandBuffer* commandBuffer,
                                      uint32_t          firstScissor,
                                      uint32_t          scissorCount,
                                      const RHIRect2D*  pScissors)           = 0;
        virtual void waitForFences()                                        = 0;

        // query
        virtual RHISwapChainDesc  getSwapchainInfo()              = 0;
        virtual RHICommandBuffer* getCurrentCommandBuffer() const = 0;

        // command write
        virtual bool prepareBeforePass(std::function<void()> passUpdateAfterRecreateSwapchain) = 0;
        virtual void submitRendering(std::function<void()> passUpdateAfterRecreateSwapchain)   = 0;

        // destroy
        virtual void destroyFramebuffer(RHIFramebuffer* framebuffer) = 0;

        // memory
        virtual bool mapMemory(RHIDeviceMemory*  memory,
                               RHIDeviceSize     offset,
                               RHIDeviceSize     size,
                               RHIMemoryMapFlags flags,
                               void**            ppData)             = 0;
        virtual void unmapMemory(RHIDeviceMemory* memory) = 0;
    };
}; // namespace Piccolo