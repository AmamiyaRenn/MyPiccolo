#pragma once

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

        std::shared_ptr<WindowSystem> window_system;
    };
    extern RuntimeGlobalContext runtime_global_context;
} // namespace Piccolo
