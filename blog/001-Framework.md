#! https://zhuanlan.zhihu.com/p/583024686
![Image](https://w.wallhaven.cc/full/8x/wallhaven-8xvwvo.jpg)

# [Piccolo 引擎复刻笔记] 第一章 创世 第一节 从 0 开始创造一个窗口

本系列是笔者学习王希老师的 GAMES104 现代游戏引擎入门课程时抄写 Piccolo 源码的笔记，由于笔者连科班计算机都不是，且基本不懂图形学(也就学过 101 与 learningOpenGL)，如有错误那就真是私密马赛
本系列最终目标是王希老师所言“完成 104 后，人手一个小引擎”“最后能做个多人网络对战游戏”
笔者代码在[此处](https://github.com/AmamiyaRenn/MyPiccolo.git)

## 程序入口

千里之行，始于足下。抄写源码总是要从 main 开始，所以我们找到位于 engine->source->editor->source 的 main.cpp。可以看到此处做了两件事情 ① 初始化并运行引擎 ② 初始化并运行编辑器
所以现在可以创建一个 main.cpp 了，但是立马就遇到了项目建构的问题，于是装模作样开抄 cmakelist 与文件目录结构
![Image](https://pic4.zhimg.com/80/v2-7ac128688b17e4f4ed4d9e95b4269152.png)
![Image](https://pic4.zhimg.com/80/v2-6774ce15754baa6bac02cb9c9a9ffdae.png)

```python
# Editor CMakeLists
set(TARGET_NAME PiccoloEditor) # 增加构建目标
# 收集对应目录的文件到对应变量
# If the CONFIGURE_DEPENDS flag is specified,
# CMake will add logic to the main build system check target to rerun the flagged GLOB commands at build time. If any of the outputs change, CMake will regenerate the build system.
file(GLOB EDITOR_HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
file(GLOB EDITOR_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
add_executable(${TARGET_NAME} ${EDITOR_HEADERS} ${EDITOR_SOURCES}) # 给建构目标添加文件
target_link_libraries(${TARGET_NAME} PiccoloRuntime)# Runtime已经作为静态库被构建了，所以要链接过来
```

此处为部分，详细请看代码及其注释部分，

顺藤摸瓜，创建 editor 与 engine 的壳子部分，完成 main 的框架
![Image](https://pic4.zhimg.com/80/v2-beef46753180b631a5c02814f7f43338.png)

## 完成窗口函数框架

有了壳子，编译再通过后（中间又遇到了只写函数定义忘写函数实现导致的符号解析错误了），终于完成了“程序入口”这一步，可以深入去完成这个引擎了，**首先订个小目标：运行引擎后拥有一个有简单渲染的界面**
要完成这个小目标，**首先得解决视窗系统和渲染系统**，即拥有一个可以简单互动的界面与一个简单的软件光栅化渲染器，而这一部分都位于源码的 EditorUI 中，所以我们从此处入手
要完成这个目标，首先要定位这些系统所在的层次：根据 [Games104 第二课](https://www.bilibili.com/video/BV12Z4y1B7th/?share_source=copy_web)，游戏引擎中让游戏能被我们看见、玩到的层次是功能层
![Image](https://pic4.zhimg.com/80/v2-68ecedf8f91cee67f0185d1ad512218f.png)
所以我们在 runtime（游戏运行时源文件夹）中开出我们的 function 文件夹，往其中加入 ui 和 render（渲染系统与视窗系统）的文件夹，然后我们继续探索 ui 部分
ui 主体就是 WindowUI 这个类，前面代码中我们看到过它，就是在 EditorUI 中——EditorUI 继承自 WindowUI，至于为什么要这么做，目前还不是非常明朗，所以我们暂且只是抄下不表
此处有我们更加关心的的东西——WindowSystem&RenderSystem，这两个系统只要完成，我们的小目标就算完成了，所以我们深入看这两部分代码
在拥有一个漂亮的界面前，我们首先想要一个黑乎乎的窗口，不然搞半天一点反馈都没有也挺无聊的；要达成这个目标，我们就深入 WindowSystem，这个类通过 glfw 给我们提供了一个视窗系统，glfw 库 对于学过[LearningOpenGL](https://learnopengl-cn.github.io/01%20Getting%20started/02%20Creating%20a%20window/)的朋友来说并不是很陌生，如果没学过，稍微看一看就好了也不难；视窗系统类的其余部分可以看出是一些回调函数的声明和使用，此处先不去理睬，我们目前的目标是创建一个黑乎乎的窗口，暂时不需要窗口交互——而要完成这一点，我们只需要初始化 glfw 就行了，所以下面的重点放在 Piccolo 在何处初始化 glfw 上
通过[RTFSC](http://acronymsandslang.com/definition/192433/RTFSC-meaning.html)，我们知道视窗系统通过 initialize 来初始化 glfw，而这个函数被`RuntimeGlobalContext::startSystems`调用，阅读这个函数，我们了解到这是在对引擎各个部分进行初始化，再 RTFSC 我们发现这个函数正是在引擎初始化阶段被调用的，于是我们兜兜转转又回到了初始化引擎部分；初始化引擎三句话分别是反射信息注册，初始化子模块（RuntimeGlobalContext(运行时全局上下文)可以看出是对各个子系统的统一管理）与记录日志，目前不用关心 1 和 3，接下来就是抄代码的时间（glfw 导入部分抄完再说）
值得注意的是代码中大量用到了智能指针（shared_ptr unique_ptr）来管理内存，此处是学习 C++时间
抄完了除 glfw 部分代码后，现在准备导入 glfw 库并完成 glfw 初始化

## 导入 glfw

首先，glfw 作为第三方库内容，我们在 engine 目录下创建 3rdparty 目录
glfw 作为自己编译的库，我们此处先简单的把它放到 3rdparty 目录下
接下来要解决 cmake 的问题
我们在 engine 的 cmakelist 中加入子目录，并在 3rdparty 的 CMakelist 中加入以下代码

```python
set(third_party_folder "ThirdParty")
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE) # 设置一个bool值：编译为静态库
if(NOT TARGET glfw) # 判断 target-name 是否存在
    option(GLFW_BUILD_EXAMPLES "" OFF) # 关闭一些不必要的生成
    option(GLFW_BUILD_TESTS "" OFF)
    option(GLFW_BUILD_DOCS "" OFF)
    option(GLFW_INSTALL "" OFF)
    add_subdirectory(glfw)
    set_target_properties(glfw PROPERTIES FOLDER ${third_party_folder}/glfw) # 导入外部库，设置外部库文件路径
    set_target_properties(update_mappings PROPERTIES FOLDER ${third_party_folder}/glfw) # gamepad mapping
endif()
```

当然，runtime 目录下也要记得要链接刚加入的库
接下来是加入 initialize 代码

```C++
    void WindowSystem::initialize(WindowCreateInfo& create_info)
    {
        if (!glfwInit()) // 尝试初始化glfw
        {                // TODO: 日志系统
            return;      // 失败后终止
        }

        width  = create_info.width;
        height = create_info.height;

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // 不使用诸如OpenGL ES之类的API
        if (!(window = glfwCreateWindow(width, height, create_info.title, nullptr, nullptr))) // 创建一个窗口对象
        {
            glfwTerminate();
            return; // 失败后终止
        }
    }
```

## 你好，窗口！

好，编译运行，出现了一个小黑框，但闪退了；成功了，但只成功了一半，这是为什么呢？
学过 learningOpenGL 的应该知道，上文只是创建了一个窗口，接下来我们还需要用一个`while(true)`来保持它([你好，窗口！](https://learnopengl-cn.github.io/01%20Getting%20started/03%20Hello%20Window/))，所以我们 RTFSC 后知道在 editor->run()中确实有这样的东西（engine->run()并没有实际使用）；

```c++
    void PiccoloEditor::run()
    {
        assert(engine_runtime);
        assert(editor_ui);
        while (true)
        {
            float delta_time = engine_runtime->calculateDeltaTime();
            if (!engine_runtime->tickOneFrame(delta_time))
                return;
        }
    }
```

这里就是我们游戏引擎的主循环，引擎不断的隔一段时间 tick 一下，计算各类实时数据，然后显示画面，此处我们暂且不去深究里面的东西，而是简单完成一下计算时间部分与 tickOneFrame 的 function return 部分
完成后我们运行程序，得到如下画面（居然不是黑乎乎而且纯白）![Image](https://pic4.zhimg.com/80/v2-b890cba66edbe15109f04be13a004d64.png)
不过这样的结果也足够振奋人心了，南大蒋炎岩老师说“有了代码你就有了全世界”，现在我们已经拥有了世界的外壳（或者说拥有一个虚空）了
下面，我们尝试当个上帝，让世界拥有光
