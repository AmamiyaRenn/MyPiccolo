set(ENGINE_ROOT_DIR "${PICCOLO_ROOT_DIR}/engine")
set(THIRD_PARTY_DIR "${ENGINE_ROOT_DIR}/3rdparty")
set(ENGINE_ASSET_DIR "/asset")

set(DEPLOY_CONFIG_DIR "configs/deployment")
set(DEVELOP_CONFIG_DIR "configs/development")

# 设置vulkan选项
set(vulkan_include ${THIRD_PARTY_DIR}/VulkanSDK/include)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options("/MP") # enable parallel compilation
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT PiccoloEditor)
endif()

if(WIN32) # 对于不同平台拥有不同设置
    set(vulkan_lib ${THIRD_PARTY_DIR}/VulkanSDK/lib/Win32/vulkan-1.lib)
    set(glslangValidator_executable ${THIRD_PARTY_DIR}/VulkanSDK/bin/Win32/glslangValidator.exe) # 使用glslangValidator编译shader
    add_compile_definitions("PICCOLO_VK_LAYER_PATH=${THIRD_PARTY_DIR}/VulkanSDK/bin/Win32")
elseif(UNIX)
    if(APPLE)
        set(vulkan_lib ${THIRD_PARTY_DIR}/VulkanSDK/lib/MacOS/libvulkan.1.dylib)
        set(glslangValidator_executable ${THIRD_PARTY_DIR}/VulkanSDK/bin/MacOS/glslangValidator)
        add_compile_definitions("PICCOLO_VK_LAYER_PATH=${THIRD_PARTY_DIR}/VulkanSDK/bin/MacOS")
        add_compile_definitions("PICCOLO_VK_ICD_FILENAMES=${THIRD_PARTY_DIR}/VulkanSDK/bin/MacOS/MoltenVK_icd.json")
    else()
        set(vulkan_lib ${THIRD_PARTY_DIR}/VulkanSDK/lib/Linux/libvulkan.so.1)
        set(glslangValidator_executable ${THIRD_PARTY_DIR}/VulkanSDK/bin/Linux/glslangValidator)
        add_compile_definitions("PICCOLO_VK_LAYER_PATH=${THIRD_PARTY_DIR}/VulkanSDK/bin/Linux")
    endif()
else()
    message(FATAL_ERROR "Unknown Platform")
endif()

set(SHADER_COMPILE_TARGET PiccoloShaderCompile)
add_subdirectory(shader)

add_subdirectory(3rdparty)

add_subdirectory(source/runtime)
add_subdirectory(source/editor)

add_subdirectory(source/meta_parser)
set(CODEGEN_TARGET "PiccoloPreCompile")
include(source/precompile/precompile.cmake)

set_target_properties("${CODEGEN_TARGET}" PROPERTIES FOLDER "Engine")

add_dependencies(PiccoloRuntime "${CODEGEN_TARGET}")
add_dependencies("${CODEGEN_TARGET}" "PiccoloParser")