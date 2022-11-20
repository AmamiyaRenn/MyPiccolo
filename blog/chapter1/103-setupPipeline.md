#! https://zhuanlan.zhihu.com/p/584238207
![Image](https://w.wallhaven.cc/full/4o/wallhaven-4o8mjm.png)
本系列是笔者学习王希老师的 GAMES104 现代游戏引擎入门课程时抄写 Piccolo 源码的笔记，由于笔者连科班计算机都不是，且基本不懂图形学(也就学过 101 与 learningOpenGL)，如有错误那就请斧正
本系列最终目标是王希老师所言“完成 104 后，人手一个小引擎”“最后能做个多人网络对战游戏”
笔者代码在[此处](https://github.com/AmamiyaRenn/MyPiccolo/tree/v1.3)，代码会同步这篇文章对应的进度（所有的文章都有对应的 git tag）

# [Piccolo 引擎复刻笔记] 第一章 创世 第三节 设置图形管线 setupPipelines()

## 构建 glsl shader

glsl 需要通过 glslangValidator 编译为 SPIR-V 字节码，再被调用；piccolo 通过写 cmake 来进行命令调用，具体来说就是`ShaderCompile.cmake`与`GenerateShaderCPPFile.cmake`这两个文件；Piccolo 通过 ShaderCompile 来编译，生成字节码后通过 GenerateShaderCPPFile 来生成对应的.h 文件，从而创造 c++与 glsl 的接口
vulkan 通过`vkCreateShaderModule`函数来使用 glsl，代码中简单的在 vulkan_util 与 vulkan_rhi 处对这个函数进行了封装
下面给出一些值得注意的部分

```python
# runtime
add_dependencies(${TARGET_NAME} ${SHADER_COMPILE_TARGET}) # 显式要求cmake先编译${SHADER_COMPILE_TARGET}
target_include_directories(
    ${TARGET_NAME}
    PUBLIC $<BUILD_INTERFACE:${ENGINE_ROOT_DIR}/shader/generated/cpp>)

# shader
compile_shader( # 调用.cmake中的函数通过glslangValidator编译shader
    "${SHADER_FILES}"
    "${TARGET_NAME}"
    "${SHADER_INCLUDE_FOLDER}"
    "${GENERATED_SHADER_FOLDER}"
    "${glslangValidator_executable}")

# ShaderCompile
function(compile_shader SHADERS TARGET_NAME SHADER_INCLUDE_FOLDER GENERATED_DIR GLSLANG_BIN)
    set(working_dir "${CMAKE_CURRENT_SOURCE_DIR}")

    set(ALL_GENERATED_SPV_FILES "")
    set(ALL_GENERATED_CPP_FILES "")

    if(UNIX) # 对于unix系统，使用命令行执行命令从而使对应应用可以执行
        execute_process(COMMAND chmod a+x ${GLSLANG_BIN})
    endif()

    # For each shader, we create a header file
    foreach(SHADER ${SHADERS})
        # Prepare a header name and a global variable for this shader
        get_filename_component(SHADER_NAME ${SHADER} NAME)
        string(REPLACE "." "_" HEADER_NAME ${SHADER_NAME})
        string(TOUPPER ${HEADER_NAME} GLOBAL_SHADER_VAR)

        set(SPV_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${GENERATED_DIR}/spv/${SHADER_NAME}.spv")
        set(CPP_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${GENERATED_DIR}/cpp/${HEADER_NAME}.h")

        add_custom_command( # 定义构建命令：glslangValidator编译shader文件为spir-v文件到指定目录
            OUTPUT ${SPV_FILE}
            COMMAND ${GLSLANG_BIN} -I${SHADER_INCLUDE_FOLDER} -V100 -o ${SPV_FILE} ${SHADER} # -g -Od should only be used in debug version
            DEPENDS ${SHADER}
            WORKING_DIRECTORY "${working_dir}")

        list(APPEND ALL_GENERATED_SPV_FILES ${SPV_FILE})

        add_custom_command( # 定义构建命令：Make a C header file with the SPIR-V shader(using GenerateShaderCPPFile.cmake)
            OUTPUT ${CPP_FILE}
            COMMAND ${CMAKE_COMMAND} -DPATH=${SPV_FILE} -DHEADER="${CPP_FILE}"
            -DGLOBAL="${GLOBAL_SHADER_VAR}" -P "${PICCOLO_ROOT_DIR}/cmake/GenerateShaderCPPFile.cmake"
            DEPENDS ${SPV_FILE}
            WORKING_DIRECTORY "${working_dir}")

        list(APPEND ALL_GENERATED_CPP_FILES ${CPP_FILE})
    endforeach()

    add_custom_target(${TARGET_NAME}
        DEPENDS ${ALL_GENERATED_SPV_FILES} ${ALL_GENERATED_CPP_FILES} SOURCES ${SHADERS})
endfunction()

# GenerateShaderCPPFile
function(embed_resource resource_file_name source_file_name variable_name)
    if(EXISTS "${source_file_name}")
        if("${source_file_name}" IS_NEWER_THAN "${resource_file_name}") # 当已经存在source且比resource更新时不需要进行操作
            return()
        endif()
    endif()

    message(NOTICE embed_resource)

    if(EXISTS "${resource_file_name}")
        file(READ "${resource_file_name}" hex_content HEX) # hex读入

        # 正则表达式替换，读入32个十六进制数，转换为16个0x??
        string(REPEAT "[0-9a-f]" 32 pattern)
        string(REGEX REPLACE "(${pattern})" "\\1\n" content "${hex_content}")
        string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1, " content "${content}")
        string(REGEX REPLACE ", $" "" content "${content}")

        # 向量定义
        set(array_definition "static const std::vector<unsigned char> ${variable_name} =\n{\n${content}\n};")

        # 添加注释、#include与定义
        get_filename_component(file_name ${source_file_name} NAME)
        set(source "/**\n * @file ${file_name}\n * @brief Auto generated file.\n */\n#include <vector>\n${array_definition}\n")

        # 文件写入
        file(WRITE "${source_file_name}" "${source}")
    else()
        message("ERROR: ${resource_file_name} doesn't exist")
        return()
    endif()
endfunction()

# let's use it as a script
if(EXISTS "${PATH}")
    embed_resource("${PATH}" "${HEADER}" "${GLOBAL}") # 函数于此处被调用
endif()
```

## RHI 与 VulkanRHI

这里可以稍微谈一谈 RHI 与 VulkanRHI 的关系：鉴于 Vulkan 只是 RHI 的一种，所以我们自然想到了继承——为了之后可能会使用诸如 DirectX 之类的 RHI 做准备；这正是设计模式中讲的基本原则——**依赖倒置原则**（详见[C++设计模式入门](https://www.bilibili.com/video/BV1Yr4y157Ci/?p=2&share_source=copy_web&vd_source=78fca262a252b90390c3caa57c3e6f1bhttps://www.bilibili.com/video/BV1Yr4y157Ci/?share_source=copy_web&vd_source=78fca262a252b90390c3caa57c3e6f1b)）：外围使用 RHI 会直接用 RHI 的接口而不会管里面的实现（用什么 RHI），RHI 作为相对不变的东西存在，外围依赖这个不变的东西；同时各种特殊的 RHI 也通过 override 来进行自己的特殊的实现，也就是说特殊 RHI 正是变化的东西，这样两个变化的东西通过不变的`class RHI`来交互，从而让调用者不会修改特别多的代码（不必`else if(rhi==openGL) then`）。这个原则通过多态来实现
抄写代码的过程中用到了大量的`rhi_struct.h`与`render_type.h`里的东西，而这里面的东西只是对应 vulkan 部分的简单替换，所以我直接进行了复制（这几千行确实没变要抄）（同时 core 中的 hash.h 也复制过来了为了能通过编译，此处先不管这个东西），当然，这么激进的直接复制也会导致一堆编译的问题，只能慢慢修了；此处记得不要把之前已经写过的部分覆盖（比如 QueueFamilyIndices），不过覆盖了也没关系，我们有 git，不是吗
当你遇到类似与`glfwCreateWindowSurface 未定义的行为`之类的错误时，可以考虑加入

```c++
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
```

## 创建一个 用于 Debug 的 DrawPipeline

我们的目的是有一个简单的渲染的画面，到现在可以明确为：**画个三角形**
既然是画个三角形，那就不存在什么复杂的 phong shader 光线追踪之类的，只会用最基础的代码，或者说，我们要完成一个测试图形管线的代码
所以我们在 render 文件夹下开一个 debugdraw 文件夹，用于放置`DebugDrawPipeline`类与`DebugDrawManager`(渲染调试管理器)类，通过后者来管理前者，前者则是实现我们的目的的类——它将会通过调用 VulkanRHI 创造一个渲染管线，并完成渲染
我们依然是按照`vulkan-tutorial`来编写代码，这次我们着重抄写的是`DebugDrawPipeline`的` setupPipelines()``setupRenderPass `的相关部分
我们在 shader/glsl 中加入 debugdraw.vert 与 debugdraw.frag，并加入`vulkan-tutorial`中提供的 glsl 代码，然后编译代码，看看能不能通过编译，当通过编译后，再 generated 文件夹中便会编译生成 spv 与 cpp，如果你编译生成成功，则代表着你成功引入了 glsl
题外话：鉴于 piccolo 本身注释大部分是英语 ② 六级马上要考了，得补一点英语水平，接下的代码注释尽量会是英语

## setupPipelines()

在 VulkanRHI 的 initialize()中，我们已经成功打开了 vulakn debug，抽象好了我们的设备、配置了图形交换、显示的方式，现在正式到了建立图形管线的时候（对管线不是很了解的建议看一下 games101 的对应部分），具体而言，就是`setupPipelines()`这个函数做的事情
下面是对代码主干的讲解（细节请看`vulkan-tutorial`）

1. 取出我们的 shader 代码(glsl)模型

```c++
RHIShader* vert_shader_module = m_rhi->createShaderModule(DEBUGDRAW_VERT);// DEBUGDRA_VERT为`构建 glsl shader`阶段产生的CPP文件夹中对应`.h`文件中记录有SPIR-V字节码的数组
RHIShader* frag_shader_module = m_rhi->createShaderModule(DEBUGDRAW_FRAG);
```

2. 创建顶点着色(vertex shader)阶段所需信息，顶点顶点着色器处理每个输入进来的顶点。它需要输入每个顶点的属性，比如世界位置（position）、颜色、法线以及纹理坐标。它的输出是在裁剪坐标中的最终位置，以及需要被传递给片段着色器的属性，比如颜色和纹理坐标。这些值会在光栅化的时候被插入到片段上以获得平滑的渐变
3. 创建片段着色(fragment shader)阶段所需信息，片段着色器接受光栅化后的顶点着色器输出，并对生成一些颜色、深度缓冲
4. 设定输入顶点的格式，之后会详细研究
5. 设定输入顶点的组织方式，即把顶点组织成什么（线段？三角形？）与是否图元重启（比如要求相邻顶点连成线，允许隔 n 个顶点，就有 n 与 n+1 不连线）
6. 设定视口变换（将视域进行缩放）与裁剪变换（缩放后的视域进行裁剪(1920\*1080)，裁剪后会映射到标准化设备坐标(-1, 1)）的信息
7. 设定光栅化阶段(rasterization)所需信息：光栅化，即对顶点着色后形成的图元(primitive，顶点着色后产生的东西的称呼)进行片段化（离散成光栅（可以简单理解为一个个像素小格子）），这个阶段还会进行深度测试（产生深度缓冲）、面剔除（三角形有两个面（正/背面），其中一个面可能不会被看见，则可以不用渲染）、裁剪测试，其中还会选择多边形渲染方式如只渲染点，只渲染线框（则三角形内部颜色就没有了）
8. 设定多重采样反走样(Muti-sampling Anti-Aliasing)信息：MSAA 不同于 SSAA 的一点是：同样是为了反走样，SSAA 是简单的先渲染更高分辨率的画面再降到显示的分辨率从而反走样，MSAA 不多渲染分辨率，但对每个 fragment 进行了多采样(N\*N)，然后通过统计采样点在三角形中的个数来确定颜色，比如一个 fragment 进行 2\*2MSAA，则有四个采样点，其中只有 3 个在三角形内部，假设三角形颜色为(1,0,0)，则这个 fragment 的颜色为(0.75,0,0)
9. 深度/模板测试，之后再谈
10. 设定颜色混合方式：这个有两种方式：① 把旧的颜色缓冲和新的颜色缓冲混合之后产生最终的颜色 ② 通过位运算来混合旧颜色和新颜色；有两个结构体用于设置颜色混合：① 每个缓冲如何混合 ② 在 ① 的前提下，允许你设置混合的系数（混合因子）与是否采用位运算混合
11. 设定动态设置：在上面已经设定好的信息中，有部分允许你不需要重设管线就能动态变化的变量，如视口的大小、线宽和混合因子等，可于此处设定
12. 设定管线布局：glsl 中存在着一些`uniform`全局变量，这些变量允许你在外部调用函数中对其进行修改从而改变着色器行为，要使用这些变量，需要创建管线布局

至此，管线的固定功能就设定结束了，但没有完全结束，我们还需向 Vulkan 告知我们怎么样完成一次渲染，也就是说，我们需要设定渲染过程`renderPass`；限于篇幅，我们将会在下一篇中进行讨论
Tips: 保证你的代码始终能够编译运行是很重要的，这将会大大降低之后发现 bug 导致的 debug 时间成本
