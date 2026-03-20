#pragma once
#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "../../core/Core.h"
#include "RenderPass.h"
#include <vector>

namespace Axel
{
    class GraphicsDevice;

    enum class FramebufferTextureFormat {
        None = 0,
        // Color
        RGBA8,
        RED_INTEGER,
        // Depth/Stencil
        DEPTH24STENCIL8,
        // Defaults
        Depth = DEPTH24STENCIL8
    };

    struct FramebufferSpecification {
        uint32_t Width = 0;
        uint32_t Height = 0;
        float ClearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f }; // Axel Gray default

        Ref<RenderPass> RenderPass;
        std::vector<void*> ExistingImages; // Pointers to VkImageViews for Swapchain
        std::vector<FramebufferTextureFormat> Attachments;
        bool SwapChainTarget = false;
    };

    class Framebuffer {
    public:
        virtual ~Framebuffer() = default;

        virtual void Resize(uint32_t width, uint32_t height) = 0;
        virtual const FramebufferSpecification& GetSpecification() const = 0;

        static Ref<Framebuffer> Create(const FramebufferSpecification& spec, GraphicsDevice* device);
    };

}

#endif