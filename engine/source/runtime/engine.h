#pragma once

#include <cassert>
#include <chrono>
#include <memory>
#include <string>

namespace Piccolo
{
    class PiccoloEngine
    {
    public:
        void initialize(const std::string& config_file_path);
        void shutdown();

        float calculateDeltaTime();
        bool  tickOneFrame(float delta_time);

    protected:
        bool rendererTick(float delta_time);

        std::chrono::steady_clock::time_point tick_time_point_last = std::chrono::steady_clock::now(); // 上个时间点
    };
} // namespace Piccolo