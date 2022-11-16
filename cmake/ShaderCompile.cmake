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
