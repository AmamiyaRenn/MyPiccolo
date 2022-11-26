#include "runtime/function/global/global_context.h"
#include "core/base/macro.h"
#include "function/render/render_system.h"
#include "function/render/window_system.h"

#include <memory>

namespace Piccolo
{
    RuntimeGlobalContext g_runtime_global_context; // 运行时上下文(全局变量)

    void RuntimeGlobalContext::startSystems(const std::string& config_file_path)
    {
        // 初始化日志系统
        m_logger_system = std::make_shared<LogSystem>();

        // 初始化默认设置管理器
        m_config_manager = std::make_shared<ConfigManager>();
        m_config_manager->initialize(config_file_path);

        // 初始化资产管理器
        m_asset_manager = std::make_shared<AssetManager>();

        // 初始化视窗子系统
        m_window_system = std::make_shared<WindowSystem>();
        WindowCreateInfo window_create_info;
        m_window_system->initialize(window_create_info);

        // 初始化渲染子系统
        m_render_system = std::make_shared<RenderSystem>();
        RenderSystemInitInfo render_init_info;
        render_init_info.m_window_system = m_window_system;
        m_render_system->initialize(render_init_info);

        // 初始化调试渲染管理器
        m_debugdraw_manager = std::make_shared<DebugDrawManager>();
        m_debugdraw_manager->initialize();
    }
    void RuntimeGlobalContext::shutdownSystems() {}
} // namespace Piccolo