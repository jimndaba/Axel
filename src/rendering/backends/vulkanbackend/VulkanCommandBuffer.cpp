#include "axelpch.h"
#include "VulkanCommandBuffer.h"
#include "VulkanRenderPass.h"
#include "VulkanFramebuffer.h"

#include "VulkanContext.h"+
#include "../../../core/Logger.h"

Axel::VulkanCommandBuffer::VulkanCommandBuffer(VulkanDevice& device, VkCommandPool pool):m_Device(device)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkResult result = vkAllocateCommandBuffers(m_Device.GetLogicalDevice(), &allocInfo, &m_ActiveBuffer);

    if (result != VK_SUCCESS)
    {
        AXLOG_ERROR("Failed to allocate Vulkan Command Buffer!");
    }
}

void Axel::VulkanCommandBuffer::Begin()
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    // Flag for optimized single submission per frame
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VkResult result = vkBeginCommandBuffer(m_ActiveBuffer, &beginInfo);

    if (result != VK_SUCCESS)
    {
        AXLOG_ERROR("Failed to begin recording Vulkan Command Buffer!");
    }
       
}

void Axel::VulkanCommandBuffer::End()
{

    VkResult result = vkEndCommandBuffer(m_ActiveBuffer);
       
    if (result != VK_SUCCESS)
    {
		AXLOG_ERROR("Failed to end recording Vulkan Command Buffer!");
    }
}

void Axel::VulkanCommandBuffer::BeginRenderPass(Ref<RenderPass> renderPass, Ref<Framebuffer> framebuffer)
{
    auto vRenderPass = std::static_pointer_cast<VulkanRenderPass>(renderPass);
    auto vFramebuffer = std::static_pointer_cast<VulkanFramebuffer>(framebuffer);

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = vRenderPass->GetHandle();
    renderPassInfo.framebuffer = vFramebuffer->GetHandle();
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = { vFramebuffer->GetSpecification().Width, vFramebuffer->GetSpecification().Height };

    // This is the color for the "Blue Screen"
    VkClearValue clearValue = { {{ 0.1f, 0.1f, 0.8f, 1.0f }} };
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearValue;

    vkCmdBeginRenderPass(m_ActiveBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void Axel::VulkanCommandBuffer::EndRenderPass()
{
    vkCmdEndRenderPass(m_ActiveBuffer);
}

void Axel::VulkanCommandBuffer::SubmitCommandBuffer(GraphicsContext* context, Ref<RenderCommandBuffer> commandBuffer)
{
    auto vContext = static_cast<VulkanContext*>(context);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    // Wait for the image from the swapchain
    VkSemaphore waitSems[] = { vContext->GetImageAvailableSemaphore() };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSems;
    submitInfo.pWaitDstStageMask = waitStages;

    // The recorded commands
    VkCommandBuffer cmd = std::static_pointer_cast<VulkanCommandBuffer>(commandBuffer)->GetHandle();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;

    // Signal when done
    VkSemaphore signalSems[] = { vContext->GetRenderFinishedSemaphore() };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSems;

    // Submit and signal the InFlightFence
	auto device = (VulkanDevice*)(context->GetDevice().get());
    vkQueueSubmit(device->GetGraphicsQueue(), 1, &submitInfo, vContext->GetInFlightFence());
}

void Axel::VulkanCommandBuffer::DrawIndexed(uint32_t indexCount, uint32_t instanceCount)
{
    // In Vulkan, we draw with an offset of 0 for the first index and first instance
    vkCmdDrawIndexed(m_ActiveBuffer, indexCount, instanceCount, 0, 0, 0);
}

void Axel::VulkanCommandBuffer::BindMaterial(Ref<Material> material)
{
}
