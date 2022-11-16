#pragma once

#include "function/render/render_system.h"
#include "function/render/window_system.h"
#include <memory>
#include <string>

namespace Piccolo
{
    class RuntimeGlobalContext
    {
    public:
        // create all global systems and initialize these systems
        void startSystems(const std::string& config_file_path);
        // destroy all global systems
        void shutdownSystems();

        std::shared_ptr<WindowSystem> m_window_system;
        std::shared_ptr<RenderSystem> m_render_system;
    };
    extern RuntimeGlobalContext g_runtime_global_context;
} // namespace Piccolo
