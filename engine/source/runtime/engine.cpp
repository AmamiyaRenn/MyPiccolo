#include "runtime/engine.h"

#include "runtime/function/global/global_context.h"

namespace Piccolo
{
    PiccoloEngine::PiccoloEngine(const std::string& config_file_path)
    {
        runtime_global_context.startSystems(config_file_path); // 初始化子系统
    }
    void PiccoloEngine::initialize() {}
    void PiccoloEngine::run() {}
    void PiccoloEngine::shutdown() {}
} // namespace Piccolo