#pragma once
#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <core/Core.h>
#include <rendering/GraphicsCore.h>
#include "RenderPass.h"
#include <vector>

namespace Axel
{
    class GraphicsDevice;
    class Texture2D;

    struct FramebufferSpecification {
        uint32_t Width = 0;
        uint32_t Height = 0;
        float ClearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f }; // Axel Gray default

        Ref<RenderPass> RenderPass;
        std::vector<void*> ExistingImages; // Pointers to VkImageViews for Swapchain
        std::vector<TextureFormatOptions> Attachments;
        bool HasDepthStencil = false;
        bool IsSampled = false;
        bool SwapChainTarget = false;
    };

    class Framebuffer {
    public:
        virtual ~Framebuffer() = default;
		virtual std::shared_ptr<Texture2D> GetColorAttachmentRendererID(uint32_t index = 0) const = 0;
        virtual void Resize(uint32_t width, uint32_t height) = 0;
        virtual const FramebufferSpecification& GetSpecification() const = 0;

        static Ref<Framebuffer> Create(GraphicsContext* ctxt, const FramebufferSpecification& spec);
    };

}

#endif