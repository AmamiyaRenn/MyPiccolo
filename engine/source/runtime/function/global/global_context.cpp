#include "runtime/function/global/global_context.h"
#include "function/render/window_system.h"
#include <memory>

namespace Piccolo
{
    RuntimeGlobalContext runtime_global_context; // 运行时上下文(全局变量)

    void RuntimeGlobalContext::startSystems(const std::string& config_file_path)
    {
        // 初始化视窗子系统
        window_system = std::make_shared<WindowSystem>();
        WindowCreateInfo window_create_info;
        window_system->initialize(window_create_info);
    }
    void RuntimeGlobalContext::shutdownSystems() {}
} // namespace Piccolo