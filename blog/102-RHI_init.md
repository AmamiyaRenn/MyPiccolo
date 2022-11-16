#! https://zhuanlan.zhihu.com/p/583980556
![Image](https://w.wallhaven.cc/full/gp/wallhaven-gpzve7.png)

本系列是笔者学习王希老师的 GAMES104 现代游戏引擎入门课程时抄写 Piccolo 源码的笔记，由于笔者连科班计算机都不是，且基本不懂图形学(也就学过 101 与 learningOpenGL)，如有错误那就请斧正
本系列最终目标是王希老师所言“完成 104 后，人手一个小引擎”“最后能做个多人网络对战游戏”
笔者代码在[此处](https://github.com/AmamiyaRenn/MyPiccolo/tree/v1.2)，代码会同步这篇文章对应的进度

# [Piccolo 引擎复刻笔记] 第一章 创世 第二节 Vulkan_RHI 初始化

既然要让引擎真正“画”出东西，就要创建对应的系统`RenderSystem`类

## RHI

既然要让引擎真正“画”出东西，就要创建对应的系统`RenderSystem`类；简单创建并写了初始化函数壳子后，遇到一个问题：我们怎么样才能显示图形。
当然，这个问题是简单的，我们需要 RHI(Render Hardware Interface)，或者说图形学 api
游戏引擎不提供图形学 api，但调用 api；主流的 cg api 有 DirectX、OpenGL、Metal、Vulkan 等，考虑到 Piccolo 跨平台，所以使用 Vulkan；相比 OpenGL，Vulkan 更加底层，对硬件的控制也更加精细
不过笔者并不会 Vulkan，现在只能边用边学，下面给出一些链接
[vulkan 编程指南](https://github.com/fangcun010/VulkanTutorialCN/blob/master/Vulkan%E7%BC%96%E7%A8%8B%E6%8C%87%E5%8D%97.pdf)
[vulkan-tutorial code](https://github.com/heitaoflower/vulkan-tutorial)
[[Piccolo 图形悦读笔记]卷 1：初始化 Vulkan RHI](https://zhuanlan.zhihu.com/p/480412509)

## 引入 Vulkan

经过简单的阅读，我们大致了解了 Vulkan 渲染需要有哪些步骤，所以现在开始准备引入 Vulkan
首先创建 interface 文件夹用于放置 rhi，创建 vulkan 文件夹放置 VulkanRHI
在 rhi 中，我们创立了 RHI 类作为 RHI 的抽象
现在我们深入完成 vulkan_rhi 的 initialize 函数，现在是抄代码 time（遇到的不懂的暂时先抄再说）！
值得注意的时，工程中存在大量的`init_info`的结构体，之所以要这样定义，是为了 ① 初始化方便 ② 更加容易共享给别的管线
为了引入 Vulkan，我们首先去安装 VulkanSDK（此处我复制了 Piccolo 源码中提供的 VulkanSDK 与 vulkanmemoryallocator 放置到 3rdparty 且去官网安装了 SDK）
如果你没有安装 SDK，则验证层将不会工作
然后分别在 engine/runtime 处加上对应的 CMakeLists 构建代码

```python
# engine
if(WIN32) # 对于不同平台拥有不同设置
    set(vulkan_lib ${THIRD_PARTY_DIR}/VulkanSDK/lib/Win32/vulkan-1.lib)
    set(glslangValidator_executable ${THIRD_PARTY_DIR}/VulkanSDK/bin/Win32/glslangValidator.exe)
    add_compile_definitions("PICCOLO_VK_LAYER_PATH=${THIRD_PARTY_DIR}/VulkanSDK/bin/Win32")
    message(NOTICE "Use Win32 Vulkan")
endif()
```

```python
# runtime
target_link_libraries(${TARGET_NAME} PUBLIC ${vulkan_lib})

target_include_directories(
    ${TARGET_NAME}
    PUBLIC $<BUILD_INTERFACE:${vulkan_include}>)

target_include_directories(
    ${TARGET_NAME}
    PUBLIC $<BUILD_INTERFACE:${THIRD_PARTY_DIR}/vulkanmemoryallocator/include>
)
```

至此我们导入了 Vulkan，之后出现类似的引入库操作都类似，不再赘述

## 引入一个简单的日志系统

从开始引入图形学 API 开始，代码出错的可能性开始大幅度增加，我们需要一些机制能够让我们能定位到这些错误；同时我们可能要获得一些运行时信息，此时，我们知道我们是时候需要引入一个简单的日志系统了，此处我简单起见使用`<iostream>`中的`std::cerr`之类的工具，之后我们考虑换 spdlog(此处先简单起见)

```c++
#include <stdexcept>
#define LOG_ERROR(...) std::cerr<<__VA_ARGS__ //...表示可变参数，__VA_ARGS__就是将...的值复制到这里
```

## Vulkan 初始化

初始化的部分需要看一下一些教程，可以看出 Piccolo 是大体遵循 vulkan*tutorial 的代码的，所以只需要简单看看 vulkan_tutorial 就可以不求甚解的搞明白 vulkan 初始化部分
现阶段我们不必特别在意这些细节内容，不然就太繁杂了
考虑到目前的任务可能只是画一个三角形，所以初始化部分我们也会进行一定程度的简化（也就是只抄需要的代码，有关 XXXKHRXXX 之类的多余功能暂时不会加入）
值得注意的是，当代码量上去后，区分成员变量/局部变量将会变得非常麻烦，比如 device/physical_device，当然我们可以用`this`来特别指明，不过 this 可能 ① 被遗忘 ②this->整整占用了 6 个字符；Piccolo 源码为了解决这个问题在（几乎）所有的成员变量前加入了`m*`，在全局变量前加入了`g\_`，所以我将代码也按照这个风格来修改
在看到《图形渲染管线》这一章时可以考虑学习/复习一下[GAMES101](https://www.bilibili.com/video/BV1X7411F744?p=1&vd_source=319cfc2457dab41418812a7cbf1411b8)中前 9 讲的内容来了解一些原理
这次我们先完成到这里

```c++
        // Vulkan初始化步骤
        // 初始化Vulkan实例：设置应用名称、版本、拓展模块等
        createInstance();
        // 启动验证层从而在debug版本中发现可能存在的错误
        initializeDebugMessenger();
        // 链接之前的glfw，使vulkan与当前运行平台窗口系统兼容
        createWindowSurface();
        // 选择物理设备，并进行一些兼容性检查（比如你的rtx2060）
        initializePhysicalDevice();
        // 创建逻辑设备与准备队列，从而抽象你的物理设备为一些接口
        createLogicDevice();
        // 创建交换链，选择最适合的属性
        createSwapchain();
        // 创建交换链图像视图
        createSwapchainImageViews();
```

接下来就是解决一堆 glsl 的问题了，限于篇幅，下篇文章再细谈
