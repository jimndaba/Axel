#include "axelpch.h"
#include <core/Logger.h>
#include "Renderer.h"
#include "RenderCommandBuffer.h"
#include "backends/GraphicsContext.h"
#include "backends/vulkanbackend/VulkanContext.h"
#include "Texture.h"
#include <scene/Scene.h>
#include "Shader.h"
#include "Pipeline.h"
#include "Buffers.h"
#include "backends/FrameBuffer.h"
#include "MaterialTemplate.h"
#include <assets/AssetManager.h>
#include <assets/Loaders/TextureLoader.h>
#include "DescriptorSet.h"
#include "MaterialManager.h"

std::unique_ptr<Axel::RendererData> Axel::Renderer::s_Data;
std::shared_ptr<Axel::Texture2D> Axel::Renderer::s_WhiteTexture = nullptr;

void Axel::Renderer::Init(GraphicsContext* context,MaterialManager* mat_manager)
{
	s_Data = std::make_unique<RendererData>();
    s_Data->Context = context;
    s_Data->m_MaterialManager = mat_manager;
  

    struct SimpleVertex {
        Axel::Vec2 Position;
        Axel::Vec3 Color;
        Axel::Vec2 UV;
    };

    // 1. Define the geometry
    SimpleVertex vertices[4] = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
    };

    uint32_t indices[6] = { 0, 1, 2, 2, 3, 0 };

    // 2. Create Buffers
    s_Data->QuadVertexBuffer = Axel::VertexBuffer::Create((float*)vertices, sizeof(vertices), s_Data->Context);

    s_Data->QuadVertexBuffer->SetLayout({
        { Axel::ShaderDataType::Float2, "a_Position"},
        { Axel::ShaderDataType::Float3, "a_Color"},
        { Axel::ShaderDataType::Float2, "a_UV"}
        });

    s_Data->QuadIndexBuffer = Axel::IndexBuffer::Create(indices, 6, s_Data->Context);

    //3. Load Shader & Build Pipeline
    s_Data->QuadShader = AssetManager::GetAssetByName<Shader>("SpriteShader");
    
    s_Data->m_matTemplate = std::make_shared<Axel::MaterialTemplate>(s_Data->QuadShader->AssetID);
    s_Data->m_matTemplate->BuildPipeline(s_Data->Context);    

    AssetManager::RegisterAsset<MaterialTemplate>(s_Data->m_matTemplate);

    s_Data->SpriteSSBO = ShaderStorageBuffer::Create(
        context,
        sizeof(SpriteRenderPacket) * s_Data->MaxSprites,
        1 // binding point
    );

    s_Data->CameraUBO = UniformBuffer::Create(context,
        sizeof(SceneCamera), 0);


    auto pipeline = s_Data->m_matTemplate->GetPipeline();
    s_Data->MainDescriptorSet = DescriptorSet::Create(s_Data->Context, pipeline, 0);
    s_Data->MainDescriptorSet->Write("ubo",s_Data->CameraUBO);
    s_Data->MainDescriptorSet->Write("ssbo", s_Data->SpriteSSBO);

   
   
    

    s_Data->m_texture = Axel::TextureLoader::Load("Assets/Textures/container.jpg");
    auto result = s_Data->Context->GetDevice()->UploadTexture(s_Data->m_texture);
    if (!result)
    {
        AXLOG_ERROR("Failed to create image: {}",s_Data->m_texture->AssetID);
    }
  
    s_Data->MainDescriptorSet->Write("texSampler", s_Data->m_texture);
    s_Data->MainDescriptorSet->Update();

    s_Data->MaterialDescriptorSets.resize(2);
    for (int i = 0 ;i < 2;i++)
    {
        s_Data->MaterialDescriptorSets[i] = DescriptorSet::Create(s_Data->Context, pipeline, 1);
        s_Data->MaterialDescriptorSets[i]->Update();
    }
   

    AXLOG_INFO("Context Initialized.");

}

void Axel::Renderer::Shutdown()
{
    s_Data->Context->GetDevice()->DestroyTexture(s_Data->m_texture);
    s_Data->QuadVertexBuffer->Destroy(s_Data->Context);
    s_Data->QuadIndexBuffer->Destroy(s_Data->Context);

    s_Data->CameraUBO->Destroy(s_Data->Context);
    s_Data->SpriteSSBO->Destroy(s_Data->Context);
    s_Data->MainDescriptorSet->Destroy();  
    s_Data->m_matTemplate->GetPipeline()->Destroy();

	s_Data.reset();
}

void Axel::Renderer::BeginFrame()
{
    // 1. Tell the Context (Vulkan) to wait for fences and acquire the next image
    s_Data->Context->BeginFrame();
    s_Data->ActiveCommandBuffer = s_Data->Context->GetCurrentCommandBuffer();
   
    s_Data->UIPackets.clear();
    s_Data->ParticlePackets.clear();
    s_Data->SpritePackets.clear(); // Reset for the new frame
    s_Data->MeshPackets.clear();
}

void Axel::Renderer::EndFrame()
{
    s_Data->CurrentFrameIndex = (s_Data->CurrentFrameIndex + 1) % 2;
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
}

void Axel::Renderer::EndRenderPass(Ref<RenderPass> renderPass)
{
    s_Data->ActiveCommandBuffer->EndRenderPass();
}

void Axel::Renderer::BeginScene(Scene* current_scene)
{
    //Render System can Kick off Here
// 1. Calculate the Matrix (Agnostic)
    int width = s_Data->Context->GetCurrentFramebuffer()->GetSpecification().Width;
    int height = s_Data->Context->GetCurrentFramebuffer()->GetSpecification().Height;
    float aspectRatio = (float)width / (float)height;
    //Axel::Mat4 projection = Axel::Math::Ortho(-aspectRatio, aspectRatio, -1.0f, 1.0f, -1.0f, 1.0f);

    SceneCamera current_camera{};
    // Find the primary camera
    auto view = current_scene->GetAllEntitiesWith<TransformComponent, CameraComponent>();
    for (auto [entity, transform, camera] : view) {
        if (camera.Primary) {
            camera.AspectRatio = aspectRatio;
            camera.CalculateProjection();
            current_camera.ViewProjection = camera.Projection * Math::Inverse(transform.WorldTransform);
            break;
        }
    }     
    s_Data->CameraData = current_camera;
    s_Data->CameraUBO->SetData(&s_Data->CameraData, sizeof(SceneCamera));


   
    // 4. Batch Submission
    auto render_view = current_scene->GetAllEntitiesWith<TransformComponent, SpriteComponent>();
    for (auto [entity, trans, sprite] : render_view) {
       
        SubmitSprite(trans.WorldTransform, sprite.Color, sprite.TextureHandle,sprite.MaterialID);
    }

}

void Axel::Renderer::EndScene()
{
    if (s_Data->SpritePackets.empty()) return;

    // 1. Sort by TextureHandle to minimize pipeline stalls
    std::sort(s_Data->SpritePackets.begin(), s_Data->SpritePackets.end(),
        [](const SpriteRenderPacket& a, const SpriteRenderPacket& b) {
            return a.TextureHandle < b.TextureHandle;
        });

    s_Data->m_MaterialManager->Update();

    auto materialBuffer = s_Data->m_MaterialManager->GetMaterialBuffer();
    s_Data->MaterialDescriptorSets[s_Data->CurrentFrameIndex]->Write("u_MaterialTable", materialBuffer);
    s_Data->MaterialDescriptorSets[s_Data->CurrentFrameIndex]->Update();

    uint32_t dataSize = s_Data->SpritePackets.size() * sizeof(SpriteRenderPacket);
    s_Data->SpriteSSBO->SetData(s_Data->SpritePackets.data(), dataSize);


    auto pipeline = s_Data->m_matTemplate->GetPipeline();
    pipeline->Bind(*s_Data->Context);

    BindDescriptorSet(0, s_Data->MainDescriptorSet, pipeline);
    BindDescriptorSet(1, s_Data->MaterialDescriptorSets[s_Data->CurrentFrameIndex], pipeline);

    s_Data->QuadVertexBuffer->Bind(*s_Data->Context);
    s_Data->QuadIndexBuffer->Bind(*s_Data->Context);

    uint32_t instanceCount = static_cast<uint32_t>(s_Data->SpritePackets.size());

    DrawIndexedInstanced(6, instanceCount);

    // 5. Clear for next frame
    s_Data->SpritePackets.clear();

}

void Axel::Renderer::Flush()
{
    if (s_Data->SpritePackets.empty()) return;

    // 1. Map the Storage Buffer (SSBO) or Instance Buffer
    // 2. Upload s_Data->SpritePackets to the GPU
    s_Data->SpriteSSBO->SetData(s_Data->SpritePackets.data(),
        s_Data->SpritePackets.size() * sizeof(SpriteRenderPacket));

    // 3. Clear the CPU-side queue for the next frame/pass
    s_Data->SpritePackets.clear();
}

void Axel::Renderer::BindBuffers()
{
    s_Data->QuadVertexBuffer->Bind(*s_Data->Context);
    s_Data->QuadIndexBuffer->Bind(*s_Data->Context);
    s_Data->SpriteSSBO->Bind(*s_Data->Context,1);
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

void Axel::Renderer::SubmitSprite(Mat4& transform, Vec4 colour, UUID TextureHandle, UUID MaterialID)
{
    auto index= s_Data->m_MaterialManager->GetMaterialIndex(MaterialID);
    s_Data->SpritePackets.push_back({ transform, colour,0,index });
}


std::shared_ptr<Axel::DescriptorSet>& Axel::Renderer::ProvideTextureDescriptor(const Ref<Texture2D>& texture, Ref<Pipeline>& pipeline, uint32_t index)
{
    auto device = s_Data->Context->GetDevice();
    auto descriptor = device->GetTextureDescriptor(texture->AssetID,pipeline, index);
    if (descriptor)
        return descriptor;

    AXLOG_ERROR("Faield to get texture descriptor: {}", texture->AssetID);
    return descriptor;
}

void Axel::Renderer::RealeaseTextureDescriptor(const Ref<Texture2D>& texture,Ref<Pipeline>& pipeline)
{
}

void Axel::Renderer::BindDescriptorSet(uint32_t setIndex, const Ref<DescriptorSet>& set, const Ref<Pipeline>& pipeline)
{
    auto renderAPI = s_Data->Context->GetRenderAPI();
    renderAPI->BindDescriptorSet(setIndex, set, pipeline);
}

void Axel::Renderer::BindTextureDescriptorSet(uint32_t setIndex, Ref<Texture2D>& texture,Ref<Pipeline>& pipeline)
{
    auto renderAPI = s_Data->Context->GetRenderAPI();
    renderAPI->BindTextureDescriptorSet(setIndex, texture, pipeline);
}

void Axel::Renderer::DrawIndexed(uint32_t indexCount)
{
    // Retrieve the command buffer currently being recorded for this frame
    auto renderAPI = s_Data->Context->GetRenderAPI();
    renderAPI->DrawIndexed(indexCount);
}

void Axel::Renderer::DrawIndexedInstanced(uint32_t indexCount, uint32_t instanceCount)
{
    auto renderAPI = s_Data->Context->GetRenderAPI();
    renderAPI->DrawIndexed(indexCount,instanceCount);
}

void Axel::Renderer::DrawQuad(Mat4 transsform, Ref<Texture2D> texture)
{

    // Retrieve the command buffer currently being recorded for this frame
    //auto commandBuffer = s_Data->ActiveCommandBuffer;
    // Parameters: CommandBuffer, IndexCount, InstanceCount, FirstIndex, VertexOffset, FirstInstance
   // s_Data->ActiveCommandBuffer->DrawQuad(indexCount, 1);

}
