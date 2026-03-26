#pragma once
#ifndef GRAPHICSCONTEXT_H
#define GRAPHICSCONTEXT_H

#include "core/Core.h"
#include <rendering/RenderAPI.h>

namespace Axel {
	class Framebuffer;
	class RenderPass;
    class RenderCommandBuffer;
    class GraphicsDevice;
    class RenderAPI;

    class GraphicsContext {
    protected:
        Ref<GraphicsDevice> m_Device; //Graphics Device owned by Context
        Ref<RenderAPI> m_API; //Graphics Device owned by Context
		static RenderAPI::API m_APIType; // The type of API this context is using (Vulkan, DX12, etc.)
    public:
        virtual ~GraphicsContext() = default;


        virtual void Init() = 0; 
        virtual void Shutdown() = 0;
        virtual void SwapBuffers() = 0;

        // ✓ All frame management happens here
        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;
        virtual void Submit(Ref<RenderCommandBuffer> cmd) = 0;

        virtual RenderAPI::API GetCurrentAPI() const = 0;
        virtual Ref<GraphicsDevice> GetDevice() = 0;
        virtual Ref<RenderAPI> GetRenderAPI() = 0;  // ✓ NEW       
        virtual Ref<Framebuffer> GetCurrentFramebuffer() = 0;
        virtual Ref<RenderPass> GetMainRenderPass() = 0; // The "Screen" pass
        virtual Ref<RenderCommandBuffer> GetCurrentCommandBuffer() = 0;	

        // Factory method to decide which API to boot
        static Scope<GraphicsContext> Create(void* windowHandle);
    };
}

#endif