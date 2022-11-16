#pragma once

#define GLFW_INCLUDE_VULKAN
#include "function/render/window_system.h"
#include <memory>

namespace Piccolo
{
    struct RHIInitInfo
    {
        std::shared_ptr<WindowSystem> window_system;
    };

    class RHI
    {
    public:
        // initialize
        virtual void initialize(RHIInitInfo init_info) = 0;

        // allocate and create
        virtual void createSwapchain()           = 0;
        virtual void createSwapchainImageViews() = 0;
    };
}; // namespace Piccolo