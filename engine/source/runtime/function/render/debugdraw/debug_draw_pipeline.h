#pragma once

#include "function/render/interface/rhi.h"

namespace Piccolo
{
    struct DebugDrawPipelineBase
    {
        RHIPipelineLayout* layout   = nullptr;
        RHIPipeline*       pipeline = nullptr;
    };

    enum class DebugDrawPipelineType : uint8_t
    {
        triangle,
        count,
    };

    class DebugDrawPipeline
    {
    public:
        explicit DebugDrawPipeline(DebugDrawPipelineType pipelineType) { m_pipeline_type = pipelineType; }
        void                  initialize();
        DebugDrawPipelineType getPipelineType() const { return m_pipeline_type; }

    private:
        void setupRenderPass();
        void setupPipelines();

        DebugDrawPipelineType m_pipeline_type;
        // RHIDescriptorSetLayout*            m_descriptor_layout; // 管线布局描述器
        std::vector<DebugDrawPipelineBase> m_render_pipelines; // 渲染管线
        std::shared_ptr<RHI>               m_rhi;
    };
} // namespace Piccolo