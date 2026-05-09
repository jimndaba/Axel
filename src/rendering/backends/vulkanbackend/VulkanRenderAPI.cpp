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

    // Check each bit individually using bitwise AND
    if (stages & Axel::ShaderStage::Vertex)                  flags |= VK_SHADER_STAGE_VERTEX_BIT;
    if (stages & Axel::ShaderStage::Fragment)                flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
    if (stages & Axel::ShaderStage::Compute)                 flags |= VK_SHADER_STAGE_COMPUTE_BIT;
    if (stages & Axel::ShaderStage::Geometry)                flags |= VK_SHADER_STAGE_GEOMETRY_BIT;
    if (stages & Axel::ShaderStage::TessellationControl)     flags |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    if (stages & Axel::ShaderStage::TessellationEvaluation)  flags |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;

    // IMPORTANT: Do NOT fall back to ALL_GRAPHICS here.
    // If flags == 0, something is wrong upstream - assert instead.
    AXEL_CORE_ASSERT(flags != 0, "AxelStageToVulkan: No valid stage flags provided!");
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

void Axel::VulkanRenderAPI::DrawIndexed(uint32_t indexCount, uint32_t instanceCount,uint32_t firsIndex,uint32_t vertexOffset,uint32_t firstInstance)
{
    if (!m_Context) {
        AXLOG_ERROR("DrawIndexed: context is null");
        return;
    }

    VkCommandBuffer cmd = m_Context->GetActiveCommandBuffer();
    vkCmdDrawIndexed(cmd, indexCount, instanceCount, firsIndex, vertexOffset, firstInstance);
}

void Axel::VulkanRenderAPI::DrawQuad(const Mat4& transform, const Ref<Texture2D>& texture)
{   
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

void Axel::VulkanRenderAPI::SetViewport(float width, float height)
{
    auto commandBuffer = static_cast<VulkanContext*>(m_Context)->GetActiveCommandBuffer();

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = width;
    viewport.height = height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
}

void Axel::VulkanRenderAPI::SetScissor(float width, float height)
{
    auto commandBuffer = static_cast<VulkanContext*>(m_Context)->GetActiveCommandBuffer();

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void Axel::VulkanRenderAPI::BindDescriptorSet(uint32_t setIndex, const Ref<DescriptorSet>& set)
{
    if (!m_ActivePipeline)
    {
        // Log Error: Cannot bind descriptors without a bound pipeline layout
        return;
    }

    auto cmdbuffer = static_cast<VulkanContext*>(m_Context)->GetActiveCommandBuffer();
    // Cast to internal Vulkan types   
    auto vkSet = std::static_pointer_cast<VulkanDescriptorSet>(set)->GetHandle();
    auto vkPipeline = std::static_pointer_cast<VulkanPipeline>(m_ActivePipeline);

    vkCmdBindDescriptorSets(
        cmdbuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        vkPipeline->GetLayout(), // The VkPipelineLayout required by the API
        setIndex,                // firstSet: the starting index in the layout
        1,                       // descriptorSetCount
        &vkSet,
        0, nullptr               // Dynamic offsets (if applicable)
    );

}

void Axel::VulkanRenderAPI::BindTextureDescriptorSet(uint32_t setIndex, Ref<Texture2D>& texture, Ref<Pipeline>& pipeline)
{
    // 1. Cast the pipeline to the Vulkan implementation to access the Layouts
    auto vkPipeline = std::static_pointer_cast<VulkanPipeline>(pipeline);

    // 2. Retrieve the specific layout for this setIndex from the pipeline's reflected layout
    // Assuming your Pipeline class has a way to get a specific set's layout
    auto setLayout = vkPipeline->GetDescriptorSetLayout(setIndex);

    // 3. Request the descriptor from the device using the ID and the required layout
    auto descriptor = m_Context->GetDevice()->GetTextureDescriptor(texture->AssetID, setLayout);

    if (descriptor)
    {
        auto cmdbuffer = static_cast<VulkanContext*>(m_Context)->GetActiveCommandBuffer();
        auto vkSet = std::static_pointer_cast<VulkanDescriptorSet>(descriptor)->GetHandle();

        vkCmdBindDescriptorSets(
            cmdbuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            vkPipeline->GetLayout(), // The VkPipelineLayout
            setIndex,
            1,
            &vkSet,
            0, nullptr
        );
    }
    else
    {
        AXLOG_ERROR("No Descriptor found for Texture: {} using Set Index: {}", texture->AssetID, setIndex);
    }
}

void Axel::VulkanRenderAPI::BindPipeline(const Ref<Pipeline>& pipeline)
{
    m_ActivePipeline = pipeline;
    pipeline->Bind(*m_Context);

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
