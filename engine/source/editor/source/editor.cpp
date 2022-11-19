#include "editor/include/editor_ui.h"
#include "function/global/global_context.h"
#include <cassert>

#include "editor/include/editor.h"
#include <memory>

namespace Piccolo
{
    void PiccoloEditor::initialize(PiccoloEngine* engine_runtime)
    {
        assert(engine_runtime);                // 确保engine正确被初始化了
        this->engine_runtime = engine_runtime; // 绑定engine_runtime

        editor_ui                     = std::make_shared<EditorUI>();
        WindowUIInitInfo ui_init_info = {g_runtime_global_context.m_window_system,
                                         g_runtime_global_context.m_render_system};
        editor_ui->initialize(ui_init_info);
    }
    void PiccoloEditor::run()
    {
        assert(engine_runtime);
        assert(editor_ui);
        float delta_time = 0;
        do
        {
            delta_time = engine_runtime->calculateDeltaTime();
        } while (engine_runtime->tickOneFrame(delta_time));
    }
    void PiccoloEditor::shutdown() {}
} // namespace Piccolo
