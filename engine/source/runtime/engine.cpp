﻿#include "runtime/engine.h"

#include "function/render/window_system.h"
#include "runtime/function/global/global_context.h"
#include <string>

namespace Piccolo
{
    void PiccoloEngine::initialize(const std::string& config_file_path)
    {
        runtime_global_context.startSystems(config_file_path); // 初始化子系统
    }

    void PiccoloEngine::shutdown() {}

    float PiccoloEngine::calculateDeltaTime()
    {
        using namespace std::chrono;                                    // 作用域using
        steady_clock::time_point tick_time_point = steady_clock::now(); // 用于计量时间点的时钟类型
        duration<float>          time_span =
            duration_cast<duration<float>>(tick_time_point - tick_time_point_last); // 用于计量时间间隔类型
        tick_time_point_last = tick_time_point;                                     // 更新时间
        return time_span.count();                                                   // 返回时间差
    }

    /**
     * @brief 引擎计算一帧
     * @param delta_time
     * @return true 可以更新一帧
     */
    bool PiccoloEngine::tickOneFrame(float delta_time)
    {
        runtime_global_context.window_system->setTitle(
            std::string("Piccolo - " /*+std::to_string(getFPS())+" FPS"*/).c_str());
        // 检查有没有触发什么事件（比如键盘输入、鼠标移动等）、更新窗口状态，并调用对应的回调函数
        runtime_global_context.window_system->pollEvents();
        return !runtime_global_context.window_system->shouldClose();
    }
} // namespace Piccolo