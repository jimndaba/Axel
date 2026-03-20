#pragma once
#ifndef VKFRAMEBUFFER_H
#define VKFRAMEBUFFER_H

#include "../FrameBuffer.h"
#include "vulkan/vulkan.h"

namespace Axel
{
    class VulkanDevice;

    class VulkanFramebuffer : public Framebuffer {
    public:
        VulkanFramebuffer(const FramebufferSpecification& spec, VulkanDevice& device);
        virtual ~VulkanFramebuffer() override;

        virtual const FramebufferSpecification& GetSpecification() const override { return m_Specification; }

        VkFramebuffer GetHandle() const { return m_Framebuffer; }
        virtual void Resize(uint32_t width, uint32_t height)  override;

    private:
        void RT_Invalidate(); // "Real-Time" Invalidate (Create/Recreate)

    private:
        FramebufferSpecification m_Specification;
        VkFramebuffer m_Framebuffer = VK_NULL_HANDLE;
        VulkanDevice& mDevice;
    };





}

#endif