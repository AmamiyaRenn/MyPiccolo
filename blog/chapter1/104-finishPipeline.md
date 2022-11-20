#! https://zhuanlan.zhihu.com/p/584449336
![Image](https://w.wallhaven.cc/full/x8/wallhaven-x8movl.png)

# [Piccolo 引擎复刻笔记] 第一章 创世 第四节 最后的准备：渲染过程、组装管线、帧缓冲与命令缓冲

本系列是笔者学习王希老师的 GAMES104 现代游戏引擎入门课程时抄写 Piccolo 源码的笔记，由于笔者连科班计算机都不是，且基本不懂图形学(也就学过 101 与 learningOpenGL)，如有错误那就请斧正
本系列最终目标是王希老师所言“完成 104 后，人手一个小引擎”“最后能做个多人网络对战游戏”
笔者代码在[此处](https://github.com/AmamiyaRenn/MyPiccolo/tree/v1.4)，代码会同步这篇文章对应的进度（所有的文章都有对应的 git tag）

## 构建渲染过程 setupRenderPass()

在最终完成图形渲染管线前，我们还需要构建**渲染过程**，渲染过程会调用数个**渲染子过程**，而每个子过程都是一次对**帧缓冲对象**的操作（如颜色**附件**）
如果你忘记什么是帧缓冲对象了，可以看一看[LearningOpenGL 的对应部分](https://learnopengl-cn.github.io/04%20Advanced%20OpenGL/05%20Framebuffers/)
下面对部分名词进行解释
**渲染过程**(render pass)，渲染的一系列流程，将会包括数个渲染子过程，这些子过程可能依赖其他过程的输出
**渲染子过程**(subpass)，即是一些在渲染后期进行的操作，这些操作依赖于之前的过程在帧缓冲中已经绘制的内容，比如一个接一个地应用一系列后期处理效果。如果你把这些渲染操作全都集中到一个渲染过程中去的话，Vulkan 就可以对这些操作重新排序，并且节省内存带宽以得到最佳性能
**帧缓冲对象**(frame buffer object)就是颜色缓冲（写入颜色数值）、深度缓冲（写入深度信息）和模板缓冲（剔除特定片段）的结合，不过在帧缓冲这里面我们不会用颜色/深度/模板缓冲的概念，取而代之的是附件
**附件**(attachment)是一个内存位置，它能够作为帧缓冲的一个缓冲，可以将它想象为一张图片；不同于 OpenGL 默认会提供一个帧缓冲，Vulkan 并不提供，所以就算只是画一个三角形也要手动描述你的帧缓冲
所以，在这个函数中，我们将会 ① 描述一些附件 ② 创造子过程 ③ 创造渲染过程
而帧缓冲对象此时其实并没有完全建立（附件的绑定还没做），我们将在后面完成它

## 组装图形渲染管线

回到`setupPipelines()`函数的末尾，我们将会通过组合前面阶段产生的一些结构体来完成图形渲染管线的创建
下面摘自`vulkan-tutorial`

-   着色器阶段：定义了图形渲染管线中可编程阶段的功能的着色器模块
-   固定功能部分：所有定义了管线的固定功能阶段的结构体，比如顶点装配器、光栅化和颜色绑定等
-   管线布局：所有被着色器引用的 uniform 变量和推送（push）常量，它们可以在绘制时被更新
-   渲染过程：被管线中的过程引用的附件及其使用方法

根据四个部分，我们完成`RHIGraphicsPipelineCreateInfo`结构体，但事情还没结束
之前我在[这篇文章](https://zhuanlan.zhihu.com/p/584238207)中谈过了 VulkanRHI 与 RHI 的关系，现在这个关系将会导致我们抄代码的量大幅度提高——没错，我指的就是` virtual bool createGraphicsPipelines(RHIPipelineCache* pipelineCache, uint32_t createInfoCount, const RHIGraphicsPipelineCreateInfo* pCreateInfos, RHIPipeline* &pPipelines) = 0;`这个函数，500 多行，实在是恐怖；不过，仔细阅读代码发现其实很简单，无非是做了 RHI 到 VulkanRHI 转化的过程，所以我就很不要脸的直接复制过来了（当然，不是完全复制，有些会之后回过头来再复制）
值得一提的是，直到现在我依然没选择开始写任何的`destroy`相关的函数，因为目前来看并不是必须的，但这个终究要写，不然内存泄漏的问题将会很严重

## 创建帧缓冲 setupFramebuffer()

现在到创建帧缓冲的时候了，我们此时会把在创建交换链阶段创造的图像视图一一绑定到对应的帧缓冲上；同时由于帧缓冲在 render 时被使用，所以我们会把它绑定到之前设定过的 render pass 上；同样的，帧缓冲需要确定宽高

## 创建命令池与命令缓冲

创建帧缓冲后，我们回到`VulkanRHI::initialize()`，接下来我们要创建**命令池**与**命令缓冲**(` createCommandPool``createCommandBuffers `)；Vulkan 中的命令，如绘制操作、内存传递，需要在命令缓冲对象中记录（你想要进行的所有操作）。这么可以提前完成设置图形命令中的所有困难的工作，并且可以使用多线程
命令缓冲：Vulkan 命令存储的位置
命令池：管理命令缓冲，命令缓冲将会在命令池中分配
此处我对源码做了点调整——不使用`m_rhi_command_pool`而是把`k_max_frames_in_flight`设置为 1，然后使用`m_command_pools[0]`来标记命令池，这是为了让代码尽量保证既贴合源码又贴合 vulkan-tutorial 的表述

完成上述所说的一切后，我们就已经解决了`while(true) drawTriangle();`的上面发生的事情了，下一篇文章将会解决 drawTriangle()里面发生的事情
最后的准备已经结束，三角形预备向你问好！
