#include "axelpch.h"
#include "core/Logger.h"
#include "VulkanRenderAPI.h"
#include "VulkanCommandBuffer.h"
#include <rendering/backends/vulkanbackend/VulkanContext.h>
#include "VulkanPipeline.h"
#include "VulkanDescriptorSet.h"
#include "VulkanTexture2D.h"
#include "VulkanShader.h"

VkShaderStageFlags AxelStageToVulkan(Axel::ShaderStage stages) {
    VkShaderStageFlags flags = 0;

    if ((uint32_t)stages & (uint32_t)Axel::ShaderStage::Vertex)
        flags |= VK_SHADER_STAGE_VERTEX_BIT;

    if ((uint32_t)stages & (uint32_t)Axel::ShaderStage::Fragment)
        flags |= VK_SHADER_STAGE_FRAGMENT_BIT;

    if ((uint32_t)stages & (uint32_t)Axel::ShaderStage::Compute)
        flags |= VK_SHADER_STAGE_COMPUTE_BIT;

    if ((uint32_t)stages & (uint32_t)Axel::ShaderStage::Geometry)
        flags |= VK_SHADER_STAGE_GEOMETRY_BIT;

    // Handle your "All" or "AllGraphics" shortcuts if you have them
    if ((uint32_t)stages & (uint32_t)Axel::ShaderStage::All)
        flags |= VK_SHADER_STAGE_ALL_GRAPHICS;

    return flags;
}

Axel::VulkanRenderAPI::VulkanRenderAPI(VulkanContext* context):
	m_Context(context)
{
}

void Axel::VulkanRenderAPI::Init()
{
    m_ClearColor.color = { {0.1f, 0.1f, 0.1f, 1.0f} };
	// You can set up global Vulkan states here.
}

void Axel::VulkanRenderAPI::Shutdown()
{    
    AXLOG_INFO("VulkanRenderAPI shutdown");
}

void Axel::VulkanRenderAPI::SetClearColor(const Vec4& color)
{
    m_ClearColor.color = { {color.r, color.g, color.b, color.a} };
}

void Axel::VulkanRenderAPI::Clear()
{
    auto cmd = m_Context->GetActiveCommandBuffer();
	VkClearAttachment clearAttachment{};
	clearAttachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; 
	clearAttachment.clearValue = m_ClearColor;
	clearAttachment.colorAttachment = 0; // Assuming we're clearing the first color attachment  

    // Clear the current framebuffer's color attachment
    vkCmdClearAttachments(cmd, 1, &clearAttachment, 0, nullptr);
}

void Axel::VulkanRenderAPI::DrawIndexed(uint32_t indexCount, uint32_t instanceCount)
{
    if (!m_Context) {
        AXLOG_ERROR("DrawIndexed: context is null");
        return;
    }

    VkCommandBuffer cmd = m_Context->GetActiveCommandBuffer();
    vkCmdDrawIndexed(cmd, indexCount, instanceCount, 0, 0, 0);
}

void Axel::VulkanRenderAPI::DrawQuad(const Mat4& transform, const Ref<Texture2D>& texture)
{   
    /*
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
    */
}

void Axel::VulkanRenderAPI::SubmitCommandBuffer(Ref<RenderCommandBuffer> commandBuffer)
{
    auto device = (VulkanDevice*)(m_Context->GetDevice().get());
    auto vCmdBuffer = std::static_pointer_cast<VulkanCommandBuffer>(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    // Wait for the swapchain image to be ready before writing to it
    VkSemaphore waitSemaphores[] = { m_Context->GetImageAvailableSemaphore() };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    VkCommandBuffer cmd = vCmdBuffer->GetHandle();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;

    // Signal that rendering is finished so the Swapchain can present
    VkSemaphore signalSemaphores[] = { m_Context->GetRenderFinishedSemaphore() };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    // Submit to the Graphics Queue!
  
	VkResult result = vkQueueSubmit(device->GetGraphicsQueue(), 1, &submitInfo, m_Context->GetInFlightFence());

    if (result != VK_SUCCESS) {
        AXLOG_ERROR("Failed to submit Vulkan command buffer!");
    }
}

void Axel::VulkanRenderAPI::BindDescriptorSet(uint32_t setIndex, const Ref<DescriptorSet>& set, const Ref<Pipeline>& pipeline)
{
	auto cmdbuffer = static_cast<VulkanContext*>(m_Context)->GetActiveCommandBuffer();
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

void Axel::VulkanRenderAPI::BindTextureDescriptorSet(uint32_t setIndex, Ref<Texture2D>& texture, Ref<Pipeline>& pipeline)
{
    auto descriptor = m_Context->GetDevice()->GetTextureDescriptor(texture->AssetID,pipeline,setIndex);
    if (descriptor)
    {
        auto cmdbuffer = static_cast<VulkanContext*>(m_Context)->GetActiveCommandBuffer();
        auto vkSet = std::static_pointer_cast<VulkanDescriptorSet>(descriptor)->GetHandle();
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
    else
    {
        AXLOG_ERROR("No Descriptor found for Texture: {}", texture->AssetID);
    }
}

void Axel::VulkanRenderAPI::PushConstants(Ref<Pipeline> pipeline, ShaderStage stages, const void* data, uint32_t size, uint32_t offset)
{
    auto cmdbuffer = static_cast<VulkanContext*>(m_Context)->GetActiveCommandBuffer();
    auto vkPipeline = std::static_pointer_cast<VulkanPipeline>(pipeline);
    VkPipelineLayout layout = vkPipeline->GetLayout();

    // Map Axel ShaderStageOptions to Vulkan Flags
    VkShaderStageFlags vkStages = AxelStageToVulkan(stages);

    vkCmdPushConstants(
        cmdbuffer,
        layout,
        vkStages,
        offset,
        size,
        data
    );
}

void Axel::VulkanRenderAPI::ValidateCommandBuffer(Ref<RenderCommandBuffer> commandBuffer)
{
}
