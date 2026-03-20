#include "axelpch.h"
#include "VulkanFramebuffer.h"
#include "VulkanDevice.h"
#include "VulkanRenderPass.h"
#include <core/Logger.h>

Axel::VulkanFramebuffer::VulkanFramebuffer(const FramebufferSpecification& spec, VulkanDevice& device) : m_Specification(spec),mDevice(device)
{
   RT_Invalidate();
}

Axel::VulkanFramebuffer::~VulkanFramebuffer()
{
    if (m_Framebuffer) {
        vkDestroyFramebuffer(mDevice.GetLogicalDevice(), m_Framebuffer, nullptr);
    }
}

void Axel::VulkanFramebuffer::Resize(uint32_t width, uint32_t height)
{
    m_Specification.Width = width;
    m_Specification.Height = height;
    RT_Invalidate();
}

void Axel::VulkanFramebuffer::RT_Invalidate()
{
    VkDevice device = mDevice.GetLogicalDevice();

    if (m_Framebuffer) {
        vkDestroyFramebuffer(device, m_Framebuffer, nullptr);
    }

    // Cast our abstract RenderPass to the Vulkan implementation
    auto vRenderPass = std::static_pointer_cast<VulkanRenderPass>(m_Specification.RenderPass);

    // Convert our void* image views back to VkImageView
    std::vector<VkImageView> attachments;
    for (void* img : m_Specification.ExistingImages) {
        attachments.push_back(static_cast<VkImageView>(img));
    }

    VkFramebufferCreateInfo fbInfo{};
    fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbInfo.renderPass = vRenderPass->GetHandle();
    fbInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    fbInfo.pAttachments = attachments.data();
    fbInfo.width = m_Specification.Width;
    fbInfo.height = m_Specification.Height;
    fbInfo.layers = 1;

    VkResult result = vkCreateFramebuffer(device, &fbInfo, nullptr, &m_Framebuffer);
    if (result != VK_SUCCESS)
    {
		AXLOG_ERROR("Failed to create Vulkan Framebuffer! Error Code: %d", result);
    }
      

}
