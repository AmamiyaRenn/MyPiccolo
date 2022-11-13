#pragma once

#include <GLFW/glfw3.h>

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
        void        initialize(WindowCreateInfo& create_info);
        void        setTitle(const char* title) { glfwSetWindowTitle(window, title); }
        static void pollEvents() { glfwPollEvents(); }
        bool        shouldClose() const { return glfwWindowShouldClose(window); };

    private:
        GLFWwindow* window = nullptr;
        int         width  = 0;
        int         height = 0;
    };
} // namespace Piccolo
