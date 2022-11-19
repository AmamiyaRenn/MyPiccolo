#! https://zhuanlan.zhihu.com/p/584897287
![Image](https://pic4.zhimg.com/80/v2-758e89ae69315ac3b8366b1d425d066a.png)

# [Piccolo 引擎复刻笔记] 第一章 创世 第五节 你好，三角形！

本系列是笔者学习王希老师的 GAMES104 现代游戏引擎入门课程时抄写 Piccolo 源码的笔记，由于笔者连科班计算机都不是，且基本不懂图形学(也就学过 101 与 learningOpenGL)，如有错误那就请斧正
本系列最终目标是王希老师所言“完成 104 后，人手一个小引擎”“最后能做个多人网络对战游戏”
笔者代码在[此处](https://github.com/AmamiyaRenn/MyPiccolo/tree/v1.5)，代码会同步这篇文章对应的进度（所有的文章都有对应的 git tag）

-   鉴于 Vulkan-tutorial 的翻译有点过时了，推荐直接打开[vulkan-tutorial 的官网](https://vulkan-tutorial.com/Introduction)进行学习（不想学英语就用网页翻译软件）

## 最简渲染管线命令 loop

### 还有些工作要先做

1. 在此前，记得先绑定我们设定的视口`vkCmdSetViewport`和裁剪`vkCmdSetScissor`，具体来说，在`draw`前绑定，因为这些参数是动态(还记得之前设定的 dynamic state 吗)，所以需要在一次渲染中动态重新绑定
2. 之前我们定义过了 subpass，subpass 之间存在依赖关系——有些 subpass 需要等另一个 subpass 的某些操作做完——因为只有一个 subpass 所以我们不用再管。。。真的吗？vulkan

### 启动渲染过程

书接上回，在我们做好了一切准备后，接下来将会深入到 render loop 里面真正开始我们的渲染；于是我们开始写 beginRenderPass 函数；在这个函数中我们会设置一系列渲染过程相关的东西比如渲染过程本身、渲染附件、渲染区域、渲染前清除画面所用值之类的

完成渲染过程后，
但是情况好像不太妙，Piccolo 源码中关于 CommandBuffer、CommandPool 的部分有这么多，那么到底是用什么呢

```c++
        RHICommandBuffer* m_command_buffers[m_k_max_frames_in_flight];
        RHICommandPool* m_rhi_command_pool; // 命令池
        RHICommandBuffer* m_current_command_buffer = new VulkanCommandBuffer();
        uint8_t         m_current_frame_index {0};
        VkCommandPool   m_command_pools[m_k_max_frames_in_flight];
        VkCommandBuffer m_vk_command_buffers[m_k_max_frames_in_flight];
        VkCommandBuffer m_vk_current_command_buffer;
```

此处先按下不表，之后涉及到多线程渲染时再回来，此时只要知道` m_command_pools``m_command_buffers `和`m_vk_command_buffers`是我们真正用的东西就行了，而后两个其实是一个东西

### 绑定图形管线

调用`vkCmdBindPipeline`命令，将命令绑定到对应的管线上

### 绘制

调用`vkCmdDraw`命令，队列知道我们要绘制

### 结束渲染流程

调用`vkCmdEndRenderPass`，结束掉这次渲染流程

## 提交渲染命令

现在，我们终于到了最后一步，我们已经设定（记录）好了命令，只需要把命令提交到对应的队列就能让 gpu 绘制图形并显示画面了
接下来，我们将工作在`submitRendering`上

### 结束记录命令

我们已经设定好了命令了，现在调用`vkEndCommandBuffer`结束记录

### 提交命令到图形队列

我们现在调用`vkQueueSubmit`把之前设定好的命令提交到图形队列，让 gpu 帮我们绘制图像

### 命令显示队列显示图形

图形队列现在绘制好了图形，现在只需要调用`vkQueuePresentKHR`来命令显示队列显示图形（带有 KHR 的代表着这是 Vulkan 插件）

最简的渲染命令 loop 已经构成，理论上已经可以画出我们期待已久的三角形；但我们现在在用的不是 OpenGL 而是 vulkan——支持多线程渲染的 API，也就是说，我们会遇到让人头疼的多线程问题

## 同步问题

看到这里你可能有所疑惑：我们程序是运行在 cpu 上的，图形和显示队列运行在 gpu 上，而这两个队列是并行的，那么可能图形队列还没绘制完第 i 图片时，显示队列就会把 i 显示出来——没错，确实会这样，所以我们要通过一些方法让它们同步（图形队列绘制完后，显示队列知道图形队列绘制完了）
学过操作系统的朋友里面能想起好几种同步方法——fences/barrier/semaphore（不了解的朋友可以看看[南大蒋炎岩的操作系统课](https://www.bilibili.com/video/BV1Cm4y1d7Ur/?share_source=copy_web&vd_source=78fca262a252b90390c3caa57c3e6f1b)），而我们现在的问题估计也能一下反映过来——没错，就是生产者——消费者问题
应对生产者消费者问题，我们可以使用信号量(semaphore/sem)，具体到这里来说，就是在`vkQueueSubmit`的`pSignalSemaphores`配置为`m_image_finished_for_presentation_semaphores[m_current_frame_index]`，然后再`vkQueuePresentKHR`中的`pWaitSemaphores`给定为`pWaitSemaphores`，从而让显示队列“等”图形队列（图形队列是生产者，显示队列是消费者）
有细心的人可能想到，图片是从交换链中取出进行渲染，然后写回到交换链，再取出显示，再放回；所以在图片显示这一步结束前，不允许对相同图片进行再渲染，也就是说，显示后放回图片与再取出渲染，构成了一个生产者——消费者，不过此时不会在`vkQueuePresentKHR`中写入`pSignalSemaphores`，而是在`prepareBeforePass`中加入`AcquireNextImageKHR`，在参数中包括`m_image_available_for_render_semaphores[m_current_frame_index]`

```
额外的思考：存在死锁问题吗？
回答：不存在，因为我们没有两把“叉子”
```

这样我们就解决了同步问题了吗？其实还剩下一个问题：前面解决了队列之间的问题，本质上是通过信号量解决了 gpu 多线程同步的问题，但事实上 cpu 与 gpu 之间也存在同步问题
这个问题出现在命令缓冲上——我们不允许在命令缓冲还没有提交到队列前就重新写入命令缓冲！
而写入命令缓冲这个事情是 cpu 做的，提交后命令将由 gpu 完成；我们现在需要一个办法，让同一帧的命令缓冲在`vkQueueSubmit`前不会重复写入
所以我们引入 Vulkan 的 fences 机制；所谓 fences，可以理解为专门用于同步 cpu 与 gpu 行为的二值信号量；从使用上来看，具体来说，就是在渲染的一开始要写入命令缓冲前，我们需要`waitForFences`，如果 fence 已经被 signal 了，说明允许写入缓冲，cpu 程序（也就是我们的代码）就会继续运行，接下来我们要`vkResetFences`，把 fence unsignal 掉，然后在`vkQueueSubmit`中加入这个 fence 作为被 signal 的参数，这样如果程序已经在 i 帧中跑了且还没提交队列，那么如果代码又跑到 i 帧要修改命令缓冲了（cpu 的循环执行很快，假设你只定义了 1 帧，那么 gpu 还没渲染完就 cpu 就又开始跑渲染命令循环了），那么就会被暂停到`waitForFences`处
现在我们终于解决了同步问题，让我们编译运行我们的代码吧
![Image](https://pic4.zhimg.com/80/v2-758e89ae69315ac3b8366b1d425d066a.png)
当你看到这张图片时，说明你做对了一切！
我们的虚空世界出现了一束光

---

在下一篇文章里，我将对第一章进行一个小结，回顾一下我们一路上做了什么事情

---

## 番外：写代码过程中遇到的一些 bug

### VUID-VkFramebufferCreateInfo-flags-04534

bug 描述：

```shell
validation layer: Validation Error:
 [ VUID-VkFramebufferCreateInfo-flags-04534 ]
 Object 0:
 handle = 0x22425b36140,
 type = VK_OBJECT_TYPE_DEVICE; |
 MessageID = 0x77e9b3aa |
 vkCreateFramebuffer(): VkFramebufferCreateInfo attachment
 #0 mip level 0 has height (720) smaller than the corresponding framebuffer height (1280).
 The Vulkan spec states: If flags does not include VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT, each element of pAttachments that is used as an input, color, resolve, or depth/stencil attachment by renderPass must have been created with a VkImageCreateInfo::extent.height greater than or equal to height (https://vulkan.lunarg.com/doc/view/1.3.231.1/windows/1.3-extensions/vkspec.html#VUID-VkFramebufferCreateInfo-flags-04534)
```

分析：这里很明显是代码哪里把 height 给写错了

解决方法：vscode 全工程查找`height`

```c++
m_swapchain_extent = {chosen_extent.height, chosen_extent.width};// who wrote this rubbish code (you, 17 hours ago)
```

此处写反了，只需要改回来即可

### VUID-vkBeginCommandBuffer-commandBuffer-00050

bug 描述：

```shell
validation layer: Validation Error:
[ VUID-vkBeginCommandBuffer-commandBuffer-00050 ] Object 0:
 handle = 0x29d6c7dfb10, type = VK_OBJECT_TYPE_COMMAND_BUFFER; Object 1: handle = 0xcfef35000000000a, type = VK_OBJECT_TYPE_COMMAND_POOL; |
  MessageID = 0xb24f00f5 |
  Call to vkBeginCommandBuffer() on VkCommandBuffer 0x29d6c7dfb10[] attempts to implicitly reset cmdBuffer created from VkCommandPool 0xcfef35000000000a[] that does NOT have the VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT bit set. The Vulkan spec states: If commandBuffer was allocated from a VkCommandPool which did not have the VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT flag set, commandBuffer must be in the initial state (https://vulkan.lunarg.com/doc/view/1.3.231.1/windows/1.3-extensions/vkspec.html#VUID-vkBeginCommandBuffer-commandBuffer-00050)
```

分析：上面的信息很完整——我没有设置特定的 bit 导致`vkBeginCommandBuffer`隐式重置了某个没有设置特定 bit 的 command buffer

解决方法：在`createCommandPool`中加入`command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;`
其实最新的[vulkan-tutorial](https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Command_buffersvscode)中是有这个的，但之前我看的是翻译版，内容比较老了，是没这一条的
