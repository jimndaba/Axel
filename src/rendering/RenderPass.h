#pragma once
#ifndef RENDERPASS_H
#define RENDERPASS_H

#include "core/Core.h"
#include "math/Math.h"
#include <string>
#include <rendering/GraphicsCore.h>

namespace Axel
{
    enum class AttachmentLoadOp { Load, Clear, DontCare };
    enum class AttachmentStoreOp { Store, DontCare };

    class GraphicsDevice;
    class GraphicsContext;

    struct RenderPassSpecification {
        std::string DebugName;
        std::vector<TextureFormatOptions> Formats;
        bool HasDepthStencil = false;
        bool IsSampled = false;
        TextureFormatOptions DepthFormat = TextureFormatOptions::Depth24Stencil8;
        AttachmentLoadOp LoadOp = AttachmentLoadOp::Clear;
        AttachmentStoreOp StoreOp = AttachmentStoreOp::Store;
        Vec4 ClearColor{ 0.0f, 0.0f, 0.0f, 1.0f };
        bool SwapChainTarget = false;
        
    };

  
    class RenderPass {
    public:
        RenderPass(){}
        virtual ~RenderPass() = default;
        virtual const RenderPassSpecification& GetSpecification() const = 0;
		virtual void Destroy() = 0;
        static Ref<RenderPass> Create(GraphicsContext* ctxt ,const RenderPassSpecification& spec);  
    };




}

#endif