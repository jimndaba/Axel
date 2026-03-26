#include "axelpch.h"
#include "Renderer.h"
#include "RenderCommandBuffer.h"
#include "backends/GraphicsContext.h"
#include "backends/vulkanbackend/VulkanContext.h"
#include "Texture.h"
#include <core/Logger.h>

std::unique_ptr<Axel::RendererData> Axel::Renderer::s_Data;
std::shared_ptr<Axel::Texture2D> Axel::Renderer::s_WhiteTexture = nullptr;

void Axel::Renderer::Init(GraphicsContext* context)
{
	s_Data = std::make_unique<RendererData>();
    s_Data->Context = context;
    AXLOG_INFO("Context Initialized.");

    // Create a 1x1 white buffer
    //uint32_t white = 0xffffffff;
    //s_WhiteTexture = Texture2D::Create(1, 1, (unsigned char*)&white);
    //AXLOG_INFO("Renderer: White Texture Initialized.");
}

void Axel::Renderer::Shutdown()
{
	s_Data->Context->Shutdown();
	s_Data.reset();
}

void Axel::Renderer::BeginFrame()
{
    // 1. Tell the Context (Vulkan) to wait for fences and acquire the next image
    s_Data->Context->BeginFrame();
    s_Data->ActiveCommandBuffer = s_Data->Context->GetCurrentCommandBuffer();

}

void Axel::Renderer::EndFrame()
{
    s_Data->Context->EndFrame();
    s_Data->Context->SwapBuffers();
}

void Axel::Renderer::BeginRenderPass(Ref<RenderPass> renderPass, bool clear)
{
    // 1. Get the current target from the context
    auto targetFramebuffer = s_Data->Context->GetCurrentFramebuffer();
    // 2. Tell the active command buffer to record the "Begin" command
    // The Command Buffer implementation will handle the Vulkan specifics    
    s_Data->ActiveCommandBuffer->BeginRenderPass(renderPass, targetFramebuffer);


    auto renderAPI = s_Data->Context->GetRenderAPI();
    renderAPI->SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
    renderAPI->Clear();
}

void Axel::Renderer::EndRenderPass(Ref<RenderPass> renderPass)
{
    //s_Data->ActiveCommandBuffer = s_Data->Context->GetCurrentCommandBuffer();
    s_Data->ActiveCommandBuffer->EndRenderPass();
}

void Axel::Renderer::BeginScene(const CameraComponent& camera, const Mat4& transform)
{
}

void Axel::Renderer::EndScene()
{
}

void Axel::Renderer::Submit(const Ref<RenderCommandBuffer>& commandBuffer)
{
    auto renderAPI = s_Data->Context->GetRenderAPI();
    renderAPI->SubmitCommandBuffer(commandBuffer);
}

void Axel::Renderer::Submit(Ref<Mesh> mesh, Ref<Material> material, const Mat4& transform)
{
}

void Axel::Renderer::SubmitParticle(const Particle& particle)
{
}

void Axel::Renderer::SubmitUI(Ref<UIElement> element)
{
}

void Axel::Renderer::SubmitInstanced(Ref<Mesh> mesh, Ref<Material> material, const std::vector<glm::mat4>& transforms)
{
    /*
    RenderPacket packet;
    packet.Type = RenderPacketType::Mesh;
    packet.Material = material;
    packet. = mesh->GetVBO();
    packet.IBO = mesh->GetIBO();

    packet.InstanceCount = static_cast<uint32_t>(transforms.size());

    // Create or grab a GPU buffer for these transforms
    packet.InstanceData = CreateRef<InstanceBuffer>(transforms.data(), transforms.size() * sizeof(glm::mat4));

    // Sort based on the center of the group or the first instance
    float distance = glm::distance(s_Data->CameraPosition, glm::vec3(transforms[0][3]));
    packet.SortKey = CalculateSortKey(mesh->GetID(), material->GetID(), distance);

    s_Data->MeshPackets.push_back(packet);
    */
}

void Axel::Renderer::BindDescriptorSet(uint32_t setIndex, const Ref<DescriptorSet>& set, const Ref<Pipeline>& pipeline)
{
    auto renderAPI = s_Data->Context->GetRenderAPI();
    renderAPI->BindDescriptorSet(setIndex, set, pipeline);
}

void Axel::Renderer::DrawIndexed(uint32_t indexCount)
{
    // Retrieve the command buffer currently being recorded for this frame
    auto renderAPI = s_Data->Context->GetRenderAPI();
    renderAPI->DrawIndexed(indexCount);
}

void Axel::Renderer::DrawQuad(Mat4 transsform, Ref<Texture2D> texture)
{

    // Retrieve the command buffer currently being recorded for this frame
    //auto commandBuffer = s_Data->ActiveCommandBuffer;
    // Parameters: CommandBuffer, IndexCount, InstanceCount, FirstIndex, VertexOffset, FirstInstance
   // s_Data->ActiveCommandBuffer->DrawQuad(indexCount, 1);

}
