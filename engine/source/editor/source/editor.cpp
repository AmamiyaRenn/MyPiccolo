#include <cassert>

#include "editor/include/editor.h"

namespace Piccolo
{
    PiccoloEditor::PiccoloEditor() {}
    void PiccoloEditor::initialize(PiccoloEngine* engine_runtime)
    {
        assert(engine_runtime);                // 确保engine正确被初始化了
        this->engine_runtime = engine_runtime; // 绑定engine_runtime
    }
    void PiccoloEditor::run() {}
    void PiccoloEditor::shutdown() {}
} // namespace Piccolo
