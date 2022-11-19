#! https://zhuanlan.zhihu.com/p/584113907
![Image](https://w.wallhaven.cc/full/01/wallhaven-01322w.jpg)

# [Piccolo 引擎复刻笔记] 目录

## 说明

本系列是笔者学习王希老师的 GAMES104 现代游戏引擎入门课程时抄写 Piccolo 源码的笔记，由于笔者连科班计算机都不是，且基本不懂图形学(也就学过 101 与 learningOpenGL)，如有错误那请求各位大佬斧正
“吾尝终日而思矣，不如须臾之所学也”，就本人过往的学习经验来看，学代码最好的方式就是抄一遍，此处抄写 Piccolo 代码的主要目的是提高对引擎的掌握程度与应用学过的图形学知识，顺带简单体验一下复杂工程的构建过程（写一个有几万行代码的工程也是宝贵的体验），再就是提高点 C++水平
本系列最终目标是王希老师所言“完成 104 后，人手一个小引擎”“最后能做个多人网络对战游戏”
笔者代码在[此处](https://github.com/AmamiyaRenn/MyPiccolo.git)，所有的系列文章都有对应的 git tag

## 目录

### 第一章 创世

在这一章中，我们将基于 Piccolo 的框架，画出一个三角形

[第一节 从 0 开始创造一个窗口](https://zhuanlan.zhihu.com/p/583024686)
[第二节 Vulkan_RHI 初始化](https://zhuanlan.zhihu.com/p/583980556)
[第三节 设置图形管线 setupPipelines()](https://zhuanlan.zhihu.com/p/584238207)
[第四节 最后的准备：渲染过程、组装管线、帧缓冲与命令缓冲](https://zhuanlan.zhihu.com/p/584449336)
[第五节 你好，三角形！](https://zhuanlan.zhihu.com/p/584897287?)

## 参考资料

### 引擎

[GAMES104](https://games104.boomingtech.com/)
[虚幻 5 文档](https://docs.unrealengine.com/5.0/zh-CN/)
[御币的 GAMES104 图形读码笔记](https://zhuanlan.zhihu.com/p/556305878)

### 渲染

[GAMES101](https://www.bilibili.com/video/BV1X7411F744?p=1&vd_source=319cfc2457dab41418812a7cbf1411b8)

### 图形 API

[LearningOpenGL CN](https://learnopengl-cn.github.io/)
[vulkan-tutorial](https://vulkan-tutorial.com/)
vulkan-tutorial，最好的 vulkan 入门教程
[vulkan 编程指南](https://github.com/fangcun010/VulkanTutorialCN/blob/master/Vulkan%E7%BC%96%E7%A8%8B%E6%8C%87%E5%8D%97.pdf)
vulkan-tutorial 的翻译版，内容较老，不推荐
[vulkan-tutorial code](https://github.com/heitaoflower/vulkan-tutorial)

### 计算机科学

[C++设计模式入门](https://www.bilibili.com/video/BV1Yr4y157Ci/?share_source=copy_web&vd_source=78fca262a252b90390c3caa57c3e6f1b)
[南京大学 2022 操作系统-蒋炎岩](https://www.bilibili.com/video/BV1Cm4y1d7Ur/?share_source=copy_web&vd_source=78fca262a252b90390c3caa57c3e6f1b)
南大蒋炎岩操作系统，看过都说好
