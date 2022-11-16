#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <array>

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
        void               initialize(WindowCreateInfo& create_info);
        void               setTitle(const char* title) { glfwSetWindowTitle(window, title); }
        GLFWwindow*        getWindow() const { return window; }
        std::array<int, 2> getWindowSize() const { return std::array<int, 2>({width, height}); }
        static void        pollEvents() { glfwPollEvents(); }
        bool               shouldClose() const { return glfwWindowShouldClose(window); };

    private:
        GLFWwindow* window = nullptr;
        int         width  = 0;
        int         height = 0;
    };
} // namespace Piccolo
