#include "function/render/window_system.h"
#include "GLFW/glfw3.h"
#include "core/base/macro.h"

namespace Piccolo
{
    void WindowSystem::initialize(WindowCreateInfo& create_info)
    {
        if (!glfwInit()) // 尝试初始化glfw
        {
            LOG_FATAL("failed to initialize GLFW"); // TODO: 日志系统
            return;                                 // 失败后终止
        }

        m_width  = create_info.width;
        m_height = create_info.height;

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // 不产生OpenGL上下文
        if (!(m_window = glfwCreateWindow(m_width, m_height, create_info.title, nullptr, nullptr))) // 创建一个窗口对象
        {
            LOG_FATAL("failed to create window");
            glfwTerminate();
            return; // 失败后终止
        }
    }
} // namespace Piccolo
