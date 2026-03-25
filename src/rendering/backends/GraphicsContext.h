#pragma once
#ifndef GRAPHICSCONTEXT_H
#define GRAPHICSCONTEXT_H

#include "core/Core.h"

namespace Axel {
	class Framebuffer;
	class RenderPass;
    class RenderCommandBuffer;
    class GraphicsDevice;

    class GraphicsContext {
    public:
        virtual ~GraphicsContext() = default;

        virtual void Init() = 0;
        virtual void SwapBuffers() = 0;

        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;
        virtual void Shutdown() = 0;

        virtual Ref<Framebuffer> GetCurrentFramebuffer() = 0;
        virtual Ref<RenderPass> GetMainRenderPass() = 0; // The "Screen" pass
        virtual Ref<RenderCommandBuffer> GetCurrentCommandBuffer() = 0;
		virtual GraphicsDevice* GetDevice() = 0;

        // Factory method to decide which API to boot
        static Scope<GraphicsContext> Create(void* windowHandle);
    };
}

#endif