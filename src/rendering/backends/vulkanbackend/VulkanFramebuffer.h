#pragma once
#ifndef VKFRAMEBUFFER_H
#define VKFRAMEBUFFER_H

#include <rendering/FrameBuffer.h>
#include "vulkan/vulkan.h"

namespace Axel
{
    class VulkanContext;
    class Texture2D;

    class VulkanFramebuffer : public Framebuffer {
    public:
        VulkanFramebuffer(VulkanContext* ctxt, const FramebufferSpecification& spec);
        virtual ~VulkanFramebuffer() override;

        virtual const FramebufferSpecification& GetSpecification() const override { return m_Specification; }

        VkFramebuffer GetHandle() const { return m_Framebuffer; }
        virtual void Resize(uint32_t width, uint32_t height)  override;
        virtual std::shared_ptr<Texture2D> GetColorAttachmentRendererID(uint32_t index = 0) const override;
    private:
        void RT_Invalidate(); // "Real-Time" Invalidate (Create/Recreate)

    private:
        FramebufferSpecification m_Specification;
        VkFramebuffer m_Framebuffer = VK_NULL_HANDLE;
        std::vector<Ref<Texture2D>> m_AttachementImages;
        VulkanContext* context;
    };





}

#endif