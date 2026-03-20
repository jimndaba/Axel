#include "axelpch.h"
#include "Renderer.h"
#include "RenderCommandBuffer.h"
#include "backends/GraphicsContext.h"
#include "backends/vulkanbackend/VulkanContext.h"

std::unique_ptr<Axel::RendererData> Axel::Renderer::s_Data;


void Axel::Renderer::Init(GraphicsContext* context)
{
	s_Data = std::make_unique<RendererData>();
    s_Data->Context = context;
    RendererAPI::SetAPI(RendererAPI::API::Vulkan);
    s_Data->API = RendererAPI::Create();
    s_Data->API->Init();
}

void Axel::Renderer::Shutdown()
{
	s_Data.reset();
}

void Axel::Renderer::BeginFrame()
{
    // 1. Tell the Context (Vulkan) to wait for fences and acquire the next image
    s_Data->Context->BeginFrame();

    // 2. Create/Get a command buffer for this frame
    // In a real engine, you'd pull this from a pre-allocated pool
    s_Data->ActiveCommandBuffer = RenderCommandBuffer::Create(s_Data->Context);
    s_Data->ActiveCommandBuffer->Begin();
}

void Axel::Renderer::EndFrame()
{
    AXEL_CORE_ASSERT(s_Data->ActiveCommandBuffer, "No active command buffer!");
    auto fb = s_Data->Context->GetTargetFramebuffer();
    auto rp = s_Data->Context->GetMainRenderPass();
    s_Data->ActiveCommandBuffer->BeginRenderPass(rp, fb);

        

    // 1. Finalize recording
    s_Data->ActiveCommandBuffer->End();

    // 2. Submit the recorded work to the GPU
    s_Data->API->SubmitCommandBuffer(s_Data->Context, s_Data->ActiveCommandBuffer);

    // 3. Present the image to the screen and swap semaphores
    s_Data->Context->SwapBuffers();

    s_Data->ActiveCommandBuffer = nullptr;
}

void Axel::Renderer::BeginScene(const CameraComponent& camera, const Mat4& transform)
{
}

void Axel::Renderer::EndScene()
{
}

void Axel::Renderer::Submit(const Ref<RenderCommandBuffer>& commandBuffer)
{
    s_Data->API->SubmitCommandBuffer(s_Data->Context, commandBuffer);
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
