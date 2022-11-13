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
