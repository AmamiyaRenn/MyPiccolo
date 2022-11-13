#pragma once

namespace Piccolo
{
    // 窗口创建时信息
    struct WindowCreateInfo
    {
        int         width         = 1280;
        int         height        = 720;
        const char* title         = "Piccolo";
        bool        is_fullscreen = false;
    };
    class WindowSystem
    {
    public:
        WindowSystem() = default;
        void initialize(WindowCreateInfo& create_info);

    private:
        int width  = 0;
        int height = 0;
    };
} // namespace Piccolo
