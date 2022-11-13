#include <filesystem>

#include "editor/include/editor.h"
#include "runtime/engine.h"

int main(int argc, char** argv)
{
    // 获得可执行文件名称与config文件目录
    std::filesystem::path executable_path(argv[0]);
    std::filesystem::path config_file_path = executable_path.parent_path();

    // 引擎创建
    Piccolo::PiccoloEngine* engine = new Piccolo::PiccoloEngine();
    engine->initialize(config_file_path.generic_string()); // 通过config文件初始化引擎

    // 编辑器创建
    Piccolo::PiccoloEditor* editor = new Piccolo::PiccoloEditor();
    editor->initialize(engine);

    // 开始运行编辑器及其引擎
    editor->run();

    // 关机的收尾工作
    editor->shutdown();
    engine->shutdown();

    return 0;
}