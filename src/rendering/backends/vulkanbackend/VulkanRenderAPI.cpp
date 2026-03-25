#include "axelpch.h"
#include "core/Logger.h"
#include "VulkanRenderAPI.h"
#include "VulkanCommandBuffer.h"
#include "../GraphicsContext.h"
#include "VulkanContext.h"
#include "VulkanPipeline.h"
#include "VulkanDescriptorSet.h"

#include "VulkanTexture2D.h"

void Axel::VulkanRendererAPI::Init()
{
	// You can set up global Vulkan states here.
}

void Axel::VulkanRendererAPI::SetClearColor(const Vec4& color)
{
	m_ClearColor = color;
}

void Axel::VulkanRendererAPI::Clear()
{
}

void Axel::VulkanRendererAPI::DrawQuad(GraphicsContext* context, const Mat4& transform, const Ref<Texture2D>& texture)
{
    auto vContext = static_cast<VulkanContext*>(context);
    auto vkTex = std::static_pointer_cast<VulkanTexture2D>(texture);

    // 1. Get the currently bound Vulkan Shader from your Renderer State
    auto vkShader = std::static_pointer_cast<VulkanShader>(m_CurrentShader);
    VkPipelineLayout layout = vkShader->GetLayout();

    VkCommandBuffer cmd = vContext->GetActiveCommandBuffer();

    // 2. Bind Pipeline (Crucial: You must bind the pipeline before drawing!)
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vkShader->GetPipeline());

    // 3. Bind Descriptor Set using the Shader's Layout
    VkDescriptorSet set = vkTex->GetDescriptorSet();
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &set, 0, nullptr);

    // 4. Push Constants using the Shader's Layout
    vkCmdPushConstants(cmd, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Mat4), &transform);

    vkCmdDrawIndexed(cmd, 6, 1, 0, 0, 0);
}

void Axel::VulkanRendererAPI::SubmitCommandBuffer(GraphicsContext* context, Ref<RenderCommandBuffer> commandBuffer)
{
    auto vContext = static_cast<VulkanContext*>(context);
    auto device = (VulkanDevice*)(context->GetDevice());
    auto vCmdBuffer = std::static_pointer_cast<VulkanCommandBuffer>(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    // Wait for the swapchain image to be ready before writing to it
    VkSemaphore waitSemaphores[] = { vContext->GetImageAvailableSemaphore() };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    VkCommandBuffer cmd = vCmdBuffer->GetHandle();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;

    // Signal that rendering is finished so the Swapchain can present
    VkSemaphore signalSemaphores[] = { vContext->GetRenderFinishedSemaphore() };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    // Submit to the Graphics Queue!
  
	VkResult result = vkQueueSubmit(device->GetGraphicsQueue(), 1, &submitInfo, vContext->GetInFlightFence());

    if (result != VK_SUCCESS) {
        AXLOG_ERROR("Failed to submit Vulkan command buffer!");
    }
}

void Axel::VulkanRendererAPI::BindDescriptorSet(GraphicsContext* context, uint32_t setIndex, const Ref<DescriptorSet>& set, const Ref<Pipeline>& pipeline)
{
	auto cmdbuffer = static_cast<VulkanContext*>(context)->GetActiveCommandBuffer();
    auto vkSet = std::static_pointer_cast<VulkanDescriptorSet>(set)->GetHandle();
    auto vkPipeline = std::static_pointer_cast<VulkanPipeline>(pipeline);

    vkCmdBindDescriptorSets(
        cmdbuffer, // The internal VkCommandBuffer
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        vkPipeline->GetLayout(), // Using the reflected layout!
        setIndex,
        1,
        &vkSet,
        0, nullptr
    );

}
