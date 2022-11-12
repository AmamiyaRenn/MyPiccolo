#pragma once

#include <string>

namespace Piccolo
{
    class PiccoloEngine
    {
    public:
        PiccoloEngine() = delete;
        explicit PiccoloEngine(const std::string& config_file_path);
        void initialize();
        void run();
        void shutdown();
    };
} // namespace Piccolo