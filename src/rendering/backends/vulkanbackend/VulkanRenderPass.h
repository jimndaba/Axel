#pragma once
#ifndef VKRENDERPASS_H
#define VKRENDERPASS_H

#include "../RenderPass.h"
#include <vulkan/vulkan.h>

namespace Axel {

    class VulkanDevice;
    class VulkanRenderPass : public RenderPass {
    public:
        VulkanRenderPass(const RenderPassSpecification& spec,VulkanDevice& device);
        virtual ~VulkanRenderPass() override;

        virtual const RenderPassSpecification& GetSpecification() const override { return m_Specification; }

        // Vulkan-specific access
        VkRenderPass GetHandle() const { return m_RenderPass; }

    private:
        RenderPassSpecification m_Specification;
        VkRenderPass m_RenderPass = VK_NULL_HANDLE;
        VulkanDevice& mDevice;
    };
}

#endif