#pragma once
#ifndef RENDERCOMMANDBUFFER_H
#define RENDERCOMMANDBUFFER_H

#include "core/Core.h"

namespace Axel
{

    class Material;
    class RenderPass;
    class Framebuffer;
    class GraphicsContext;

    class AX_API RenderCommandBuffer {
    public:
        virtual ~RenderCommandBuffer() = default;

        virtual void Begin() = 0;
        virtual void End() = 0;

        virtual void BeginRenderPass(Ref<RenderPass> renderPass, Ref<Framebuffer> framebuffer) = 0;
        virtual void EndRenderPass() = 0;

        // High-level draw commands
        virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount) = 0;
        virtual void BindMaterial(Ref<Material> material) = 0;
        // ... more commands

        static Ref<RenderCommandBuffer> Create(GraphicsContext* context);
    };

}


#endif