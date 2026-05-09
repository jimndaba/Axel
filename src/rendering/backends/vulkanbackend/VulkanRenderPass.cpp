#include "axelpch.h"
#include "VulkanRenderPass.h"
#include "VulkanContext.h"
#include "VulkanDevice.h"
#include <core/Logger.h>

#include "VulkanUtils.h"

Axel::VulkanRenderPass::VulkanRenderPass(const RenderPassSpecification& spec, VulkanDevice& device):m_Specification(spec),mDevice(device)
{
    // In a real engine, you'd iterate through spec.Attachments
        // For our "Blue Screen" clear, we'll define a single color attachment


    VkSubpassDescription subpass{};

    std::vector<VkAttachmentDescription>  attachments;
    std::vector<VkAttachmentReference> attachmentRefs;
    int attachmentindex = 0;
    for (auto& frmt : spec.Formats)
    {
        VkAttachmentDescription colorAttachment{};
        // You would get this format from the Swapchain or the Spec's target
        colorAttachment.format = AxelTextureFormatToVulkan(frmt);
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

        // Logic driven by specification:
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        if (spec.SwapChainTarget)
        {
            colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        }
        else if (spec.IsSampled) // Your new member
        {
            // This transition allows ImGui and other shaders to read the texture
            colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }
        else
        {
            colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = attachmentindex; // Index in the pAttachments array
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
       
        attachments.push_back(colorAttachment);
        attachmentRefs.push_back(colorAttachmentRef);
        attachmentindex++;
    }
    
    
    VkAttachmentReference depthRef{};
    if (spec.HasDepthStencil)
    {
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = device.FindDepthFormat(); // IMPORTANT
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        attachments.push_back(depthAttachment);

        depthRef.attachment = attachmentindex; // index 1 (after color)
        depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        subpass.pDepthStencilAttachment = &depthRef;
        attachmentindex++;
    }    
    // Subpass: A render pass can have multiple subpasses (useful for deferred) 
    subpass.pipelineBindPoint =VK_PIPELINE_BIND_POINT_GRAPHICS ;// VK_GRAPHICS_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = static_cast<uint32_t>(attachmentRefs.size());
    subpass.pColorAttachments = attachmentRefs.data();
    if (spec.HasDepthStencil)
        subpass.pDepthStencilAttachment = &depthRef;

    // Dependency: Ensures the image layout transition happens at the right time
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;


    // We grab the device from the context (singleton pattern or passed in)
    auto ldevice = device.GetLogicalDevice();
    VkResult result = vkCreateRenderPass(ldevice, &renderPassInfo, nullptr, &m_RenderPass); 
    if (result != VK_SUCCESS)
    {
        AXLOG_ERROR("Failed to create Vulkan Render Pass!");
    };
        
}

Axel::VulkanRenderPass::~VulkanRenderPass()
{
   
}

void Axel::VulkanRenderPass::Destroy()
{
    auto device = mDevice.GetLogicalDevice();
    vkDestroyRenderPass(device, m_RenderPass, nullptr);
}
