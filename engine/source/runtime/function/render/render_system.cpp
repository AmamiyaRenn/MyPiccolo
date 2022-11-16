#include "function/render/render_system.h"
#include "function/render/interface/rhi.h"
#include "function/render/interface/vulkan/vulkan_rhi.h"

namespace Piccolo
{
    void RenderSystem::initialize(RenderSystemInitInfo init_info)
    {
        RHIInitInfo rhi_init_info;
        rhi_init_info.window_system = init_info.window_system;

        rhi = std::make_shared<VulkanRHI>();
        rhi->initialize(rhi_init_info);
    }
} // namespace Piccolo
