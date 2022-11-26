set(TARGET_NAME PiccoloRuntime)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(json_inlcude ${THIRD_PARTY_DIR}/json11)
# set(json_lib_dir ${THIRD_PARTY_DIR}/lib/json11)

# add_library(json11 ${json_inlcude}/json11.cpp)
# set_target_properties(json11 PROPERTIES FOLDER "ThirdParty/json11")

# 递归GLOB
# file(GLOB_RECURSE RUNTIME_HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
# file(GLOB_RECURSE RUNTIME_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE RUNTIME_HEADERS CONFIGURE_DEPENDS "*.h")
file(GLOB_RECURSE RUNTIME_SOURCES CONFIGURE_DEPENDS "*.cpp")

# message(NOTICE ${RUNTIME_SOURCES})
# message(NOTICE ${RUNTIME_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${RUNTIME_HEADERS} ${RUNTIME_SOURCES})

# 作为库被构建
add_library(${TARGET_NAME} ${RUNTIME_HEADERS} ${RUNTIME_SOURCES})

set(json_include ${THIRD_PARTY_DIR}/json11)
add_library(json11 ${json_include}/json11.cpp)
set_target_properties(json11 PROPERTIES FOLDER "ThirdParty/json11")

# 链接库
target_link_libraries(${TARGET_NAME} PUBLIC glfw) # PUBLIC: 在public后面的库会被Link到你的target中，并且里面的符号也会被导出，提供给第三方使用
target_link_libraries(${TARGET_NAME} PUBLIC ${vulkan_lib})
target_link_libraries(${TARGET_NAME} PRIVATE $<BUILD_INTERFACE:json11>)
target_link_libraries(${TARGET_NAME} PUBLIC spdlog::spdlog)

# IDE标识
set_target_properties(${TARGET_NAME} PROPERTIES CXX_STANDARD 17)
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Engine")

# being a cross-platform target, we enforce standards conformance on MSVC
target_compile_options(${TARGET_NAME} PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->")
target_compile_options(${TARGET_NAME} PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/WX->")
message(NOTICE "Force using MSVC")

# 将目录包含到对应目标里，不写这句话可就找不到.h文件了
target_include_directories(
    ${TARGET_NAME}
    PUBLIC $<BUILD_INTERFACE:${ENGINE_ROOT_DIR}/source>
    $<INSTALL_INTERFACE:include/${TARGET_NAME}-${PROJECT_VERSION}>
)

target_include_directories(
    ${TARGET_NAME}
    PUBLIC $<BUILD_INTERFACE:${ENGINE_ROOT_DIR}/source/runtime>
    $<INSTALL_INTERFACE:include/${TARGET_NAME}-${PROJECT_VERSION}>
)

# Vulkan
target_include_directories(
    ${TARGET_NAME}
    PUBLIC $<BUILD_INTERFACE:${vulkan_include}>)

target_include_directories(
    ${TARGET_NAME}
    PUBLIC $<BUILD_INTERFACE:${THIRD_PARTY_DIR}/vulkanmemoryallocator/include>
)

# shader
add_dependencies(${TARGET_NAME} ${SHADER_COMPILE_TARGET}) # 显式要求cmake先编译${SHADER_COMPILE_TARGET}
target_include_directories(
    ${TARGET_NAME}
    PUBLIC $<BUILD_INTERFACE:${ENGINE_ROOT_DIR}/shader/generated/cpp>)

# json
target_include_directories(
    ${TARGET_NAME}
    PUBLIC $<BUILD_INTERFACE:${json_include}>
)

# set global vari used by precompile
set(PICCOLO_RUNTIME_HEADS "${RUNTIME_HEADERS}" PARENT_SCOPE)