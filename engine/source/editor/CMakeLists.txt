set(TARGET_NAME PiccoloEditor) # 增加构建目标

# 收集对应目录的文件到对应变量
# If the CONFIGURE_DEPENDS flag is specified,
# CMake will add logic to the main build system check target to rerun the flagged GLOB commands at build time. If any of the outputs change, CMake will regenerate the build system.
file(GLOB EDITOR_HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
file(GLOB EDITOR_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)

# source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${EDITOR_HEADERS} ${EDITOR_SOURCES}) # 给IDE用于文件归类
add_executable(${TARGET_NAME} ${EDITOR_HEADERS} ${EDITOR_SOURCES}) # 给建构目标添加文件

# set_target_properties(${TARGET_NAME} PROPERTIES CXX_STANDARD 17 OUTPUT_NAME "PiccoloEditor")
# set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Engine")
target_link_libraries(${TARGET_NAME} PiccoloRuntime) # Runtime已经作为静态库被构建了，所以要链接过来
