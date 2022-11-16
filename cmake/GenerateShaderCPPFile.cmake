# ###################################################################################################
# This function converts any file into C/C++ source code.
# Example:
# - input file: data.dat
# - output file: data.h
# - variable name declared in output file: DATA
# - data length: sizeof(DATA)
# embed_resource("data.dat" "data.h" "DATA")
# ###################################################################################################

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
