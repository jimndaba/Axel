#include "axelpch.h"
#include "VulkanFramebuffer.h"
#include "VulkanContext.h"
#include "VulkanRenderPass.h"
#include <rendering/Texture.h>
#include "VulkanTexture2D.h"
#include <core/Logger.h>
#include "VulkanUtils.h"



Axel::VulkanFramebuffer::VulkanFramebuffer(VulkanContext* ctx,const FramebufferSpecification& spec) : m_Specification(spec),context(ctx)
{
   RT_Invalidate();
}

Axel::VulkanFramebuffer::~VulkanFramebuffer()
{
    if (m_Framebuffer) {
		auto mDevice = std::static_pointer_cast<VulkanDevice>(context->GetDevice());
        vkDestroyFramebuffer(mDevice->GetLogicalDevice(), m_Framebuffer, nullptr);
    }
}

void Axel::VulkanFramebuffer::Resize(uint32_t width, uint32_t height)
{
    m_Specification.Width = width;
    m_Specification.Height = height;
    RT_Invalidate();
}

std::shared_ptr<Axel::Texture2D> Axel::VulkanFramebuffer::GetColorAttachmentRendererID(uint32_t index) const
{
    return m_AttachementImages[index];
}

void Axel::VulkanFramebuffer::RT_Invalidate()
{
    auto mDevice = std::static_pointer_cast<VulkanDevice>(context->GetDevice());
    VkDevice device = mDevice->GetLogicalDevice();
    vkDeviceWaitIdle(device);
    if (m_Framebuffer) {
        vkDestroyFramebuffer(device, m_Framebuffer, nullptr);
        m_Framebuffer = VK_NULL_HANDLE;
    }
    m_AttachementImages.clear();
    // Cast our abstract RenderPass to the Vulkan implementation
    auto vRenderPass = std::static_pointer_cast<VulkanRenderPass>(m_Specification.RenderPass);

    // Convert our void* image views back to VkImageView
  
    
    std::vector<VkImageView> attachments;
    if (!m_Specification.ExistingImages.empty())
    {
        // ✅ Use swapchain images (or externally provided ones)
        for (auto& view : m_Specification.ExistingImages)
        {
            attachments.push_back((VkImageView)view);
        }
    }
    else
    {
        // ✅ Create your own images (offscreen rendering)
        for (auto& format : m_Specification.Attachments)
        {
            TextureCreationInfo info;
            info.Height = m_Specification.Height;
            info.Width = m_Specification.Width;
            info.TextureFormat = format;

            if (IsDepthFormat(format))
            {
                info.Usage = TextureUsageOptions::DepthStencil;
            }
            else
            {
                // COMBINE BITS: Tell Vulkan this is a RenderTarget AND we intend to Sample it (ImGui)
                info.Usage = TextureUsageOptions::RenderTarget | TextureUsageOptions::Sampled;
            }

            Ref<Texture2D> attachment = Texture2D::Create(info);
            m_AttachementImages.push_back(attachment);

            auto vTex = std::static_pointer_cast<VulkanTexture2D>(attachment);
            attachments.push_back(vTex->GetImageView());
        }
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
		AXLOG_ERROR("Failed to create Vulkan Framebuffer! Error Code: %d");
    }
      

}
