cmake_minimum_required(VERSION 3.19) # cmake最小版本需求

project(MyPiccolo VERSION 0.1.0) # 工程名称与版本号

set(CMAKE_CXX_STANDARD 17) # 设置c++标准
set(CMAKE_CXX_STANDARD_REQUIRED on) # 设置指定的C++编译器版本是必须的，如果不设置，或者为OFF，则指定版本不可用时，会使用上一版本
set(BUILD_SHARED_LIBS OFF) # 不构建共享库

set(PICCOLO_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}") # 定义变量

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # 输出commpile_commands.json，用于clangd

add_subdirectory(engine) # 添加一个子目录并构建该子目录