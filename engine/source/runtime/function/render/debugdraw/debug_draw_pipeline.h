#pragma once

#include "function/render/interface/rhi.h"

namespace Piccolo
{
    struct DebugDrawFrameBufferAttachment
    {
        RHIImage*        image = nullptr;
        RHIDeviceMemory* mem   = nullptr;
        RHIImageView*    view  = nullptr;
        RHIFormat        format;
    };

    struct DebugDrawFramebuffer
    {
        int            width;
        int            height;
        RHIRenderPass* render_pass = nullptr;

        std::vector<RHIFramebuffer*>                framebuffers;
        std::vector<DebugDrawFrameBufferAttachment> attachments;
    };

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
        void setupFramebuffer();

        DebugDrawPipelineType m_pipeline_type;
        // RHIDescriptorSetLayout*            m_descriptor_layout; // 管线布局描述器
        std::vector<DebugDrawPipelineBase> m_render_pipelines; // 渲染管线
        DebugDrawFramebuffer               m_framebuffer;      // 帧缓冲
        std::shared_ptr<RHI>               m_rhi;
    };
} // namespace Piccolo