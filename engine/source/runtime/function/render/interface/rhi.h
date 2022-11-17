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

        // allocate and create
        virtual void       createSwapchain()                                                 = 0;
        virtual void       createSwapchainImageViews()                                       = 0;
        virtual RHIShader* createShaderModule(const std::vector<unsigned char>& shader_code) = 0;
        virtual bool       createGraphicsPipelines(RHIPipelineCache*                    pipelineCache,
                                                   uint32_t                             createInfoCount,
                                                   const RHIGraphicsPipelineCreateInfo* pCreateInfos,
                                                   RHIPipeline*&                        pPipelines)                 = 0;
        virtual bool       createPipelineLayout(const RHIPipelineLayoutCreateInfo* pCreateInfo,
                                                RHIPipelineLayout*&                pPipelineLayout)         = 0;

        // query
        virtual RHISwapChainDesc getSwapchainInfo() = 0;
    };
}; // namespace Piccolo