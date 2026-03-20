#include "axelpch.h"
#include "VulkanRenderPass.h"
#include "VulkanContext.h"
#include "VulkanDevice.h"

Axel::VulkanRenderPass::VulkanRenderPass(const RenderPassSpecification& spec, VulkanDevice& device):m_Specification(spec),mDevice(device)
{
    // In a real engine, you'd iterate through spec.Attachments
        // For our "Blue Screen" clear, we'll define a single color attachment

    VkAttachmentDescription colorAttachment{};
    // You would get this format from the Swapchain or the Spec's target
    colorAttachment.format = VK_FORMAT_B8G8R8A8_SRGB;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

    // Logic driven by specification:
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0; // Index in the pAttachments array
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Subpass: A render pass can have multiple subpasses (useful for deferred)
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint =VK_PIPELINE_BIND_POINT_GRAPHICS ;// VK_GRAPHICS_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

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
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    // We grab the device from the context (singleton pattern or passed in)
    auto ldevice = device.GetLogicalDevice();
    AXEL_CORE_ASSERT(vkCreateRenderPass(ldevice, &renderPassInfo, nullptr, &m_RenderPass) == VK_SUCCESS,
        "Failed to create Vulkan Render Pass!");
}

Axel::VulkanRenderPass::~VulkanRenderPass()
{
    auto device = mDevice.GetLogicalDevice();
    vkDestroyRenderPass(device, m_RenderPass, nullptr);
}
