#include "axelpch.h"
#include <core/Logger.h>
#include "Renderer.h"
#include "RenderCommandBuffer.h"
#include "GraphicsContext.h"
#include "GraphicsDevice.h"
#include "Texture.h"
#include <scene/Scene.h>
#include "Shader.h"
#include "Pipeline.h"
#include "MaterialInstance.h"
#include "MaterialTemplate.h"
#include "Mesh.h"
#include "Buffers.h"
#include "FrameBuffer.h"
#include <assets/AssetManager.h>
#include <assets/Loaders/TextureLoader.h>
#include "DescriptorSet.h"
#include "MaterialManager.h"
#include "IGeometryPool.h"
#include "DescriptorSetLayout.h"


std::unique_ptr<Axel::RendererData> Axel::Renderer::s_Data;
std::shared_ptr<Axel::Texture2D> Axel::Renderer::s_WhiteTexture = nullptr;
std::unique_ptr<Axel::IGeometryPool> Axel::Renderer::s_GeometryPool;

void Axel::Renderer::Init(GraphicsContext* context,MaterialManager* mat_manager)
{
	s_Data = std::make_unique<RendererData>();
    s_Data->Context = context;
    s_Data->m_MaterialManager = mat_manager;  

    uint32_t MaxVertexBytes = 256 * 1024 * 1024; // 256 MB
    uint32_t MaxIndexBytes = 64 * 1024 * 1024;  // 64 MB

    s_GeometryPool = IGeometryPool::Create(context, MaxVertexBytes, MaxIndexBytes);
    AXLOG_INFO("Context Initialized.");

    std::vector<DescriptorBinding> bindings;
    {
        DescriptorBinding n;
        n.Binding = 0;
        n.Count = 1;
        n.Name = "u_Scene";
        n.Type = DescriptorType::UniformBuffer;
        n.Stage = ShaderStage::Fragment | ShaderStage::Vertex;
        bindings.push_back(n);
    }
    s_Data->m_SceneLayout = DescriptorSetLayout::Create(s_Data->Context,bindings);
    s_Data->SceneDescriptorSet = DescriptorSet::Create(s_Data->Context, s_Data->m_SceneLayout);
    s_Data->SceneUBO = UniformBuffer::Create(s_Data->Context,sizeof(SceneUBOData),0);
    s_Data->SceneDescriptorSet->Write("u_Scene", s_Data->SceneUBO);
    s_Data->SceneDescriptorSet->Update();
}

void Axel::Renderer::Shutdown()
{
    s_Data.reset();
}

void Axel::Renderer::BeginFrame()
{
    // 1. Tell the Context (Vulkan) to wait for fences and acquire the next image
    s_Data->Context->BeginFrame();
    s_Data->ActiveCommandBuffer = s_Data->Context->GetCurrentCommandBuffer();   
    s_Data->PacketQueue.clear();
    s_Data->PacketQueue.reserve(10000);

}

void Axel::Renderer::EndFrame()
{
    // Submit command buffer and present
    s_Data->Context->EndFrame();
    s_Data->Context->SwapBuffers();
    s_Data->CurrentFrameIndex = (s_Data->CurrentFrameIndex + 1) % 2;
}

void Axel::Renderer::BeginRenderPass(Ref<RenderPass> renderPass, Ref<Framebuffer> framebuffer, bool clear)
{
    // 1. Get the current target from the context
    // 2. Tell the active command buffer to record the "Begin" command
    // The Command Buffer implementation will handle the Vulkan specifics    
    s_Data->ActiveCommandBuffer->BeginRenderPass(renderPass, framebuffer);
}

void Axel::Renderer::EndRenderPass(Ref<RenderPass> renderPass)
{
    s_Data->ActiveCommandBuffer->EndRenderPass();
}

void Axel::Renderer::OnUpdate()
{
}

void Axel::Renderer::BeginScene(SceneRenderDesc& desc)
{
    s_Data->SceneUBO->SetData(&desc, sizeof(SceneUBOData));

    // 1. Update Global Material SSBO
    s_Data->m_MaterialManager->Update();

    // 2. Set Dynamic States
    SetViewport(desc.ViewportWidth, desc.ViewportHeight);
    SetScissor(desc.ViewportWidth, desc.ViewportHeight);
}

void Axel::Renderer::EndScene()
{   
    Flush();   
}

void Axel::Renderer::Flush()
{
    auto& q = s_Data->PacketQueue;

    std::vector<RenderPacket> opaquePackets;
    std::vector<RenderPacket> transparentPackets;
    std::vector<RenderPacket> uiPackets;
    std::vector<RenderPacket> debugPackets;

    for (auto& packet : q) {
        switch (packet.Type) {
        case RenderPacketTypeOptions::Mesh:
        case RenderPacketTypeOptions::SkinnedMesh:
            opaquePackets.push_back(packet);
            break;
        case RenderPacketTypeOptions::Billboard:
        case RenderPacketTypeOptions::Particle:
            transparentPackets.push_back(packet);
            break;
        case RenderPacketTypeOptions::UI:
            uiPackets.push_back(packet);
            break;
        case RenderPacketTypeOptions::DebugLine:
            debugPackets.push_back(packet);
            break;
        }
    }

    // Opaque: front-to-back (reduce overdraw), then by pipeline/material
    std::sort(opaquePackets.begin(), opaquePackets.end(),
        [](const RenderPacket& a, const RenderPacket& b) {
            return a.SortKey < b.SortKey;
        });

    std::sort(transparentPackets.begin(), transparentPackets.end(),
        [](const RenderPacket& a, const RenderPacket& b) {
            return a.SortKey > b.SortKey; // back-to-front for transparency
        });
      

    // -- - Step 3: Dispatch in the correct render order-- - TODO
    ExecuteOpaquePass(opaquePackets);
    //ExecuteTransparentPass(transparentPackets);
    //ExecuteUIPass(uiPackets);
    //ExecuteDebugPass(debugPackets);     // Always last so it draws on top

    q.clear();
}

void Axel::Renderer::RegisterMaterial(const Ref<MaterialInstance>& material)
{
    s_Data->m_MaterialManager->RegisterMaterial(material);
}

uint32_t Axel::Renderer::GetMaterialIndex(UUID materialID)
{
    return  s_Data->m_MaterialManager->GetMaterialIndex(materialID);
}

void Axel::Renderer::BindBuffers()
{
    //s_Data->QuadVertexBuffer->Bind(*s_Data->Context);
    //s_Data->QuadIndexBuffer->Bind(*s_Data->Context);
    //s_Data->SpriteSSBO->Bind(*s_Data->Context,1);
}

void Axel::Renderer::Submit(const Ref<RenderCommandBuffer>& commandBuffer)
{
    auto renderAPI = s_Data->Context->GetRenderAPI();
    renderAPI->SubmitCommandBuffer(commandBuffer);
}

void Axel::Renderer::SubmitParticle(const Particle& particle)
{
}

void Axel::Renderer::SubmitUI(Ref<UIElement> element)
{
}

void Axel::Renderer::SubmitMesh(MeshComponent& meshComp, TransformComponent& transform, float depth)
{
    // 1. Build the typed sub-packet
    const auto& mesh_asset = AssetManager::GetAsset<Mesh<StaticVertex>>(meshComp.MeshHandle);
    
    for (uint32_t j = 0; j < mesh_asset->m_Submeshes.size(); ++j)
    {
        const auto& submesh = mesh_asset->m_Submeshes[j];


        MeshRenderPacket mesh;
        mesh.Transform = transform.WorldTransform;
        mesh.MeshHandle = meshComp.MeshHandle; // raw value, safe in union
        mesh.SubmeshIndex = j;
        mesh.MaterialIndex = submesh.MaterialIndex;

        // 2. Wrap in the master packet
        RenderPacket packet{};  
        packet.Data = std::move(mesh);
        // 3. Compute sort key
        // Opaque: front-to-back, grouped by pipeline then material
        // Pipeline ID comes from material — look it up once here, not in the flush loop
        UUID materialtemplateID = s_Data->m_MaterialManager->GetMaterialTemplateID(submesh.MaterialIndex);
        packet.SortKey = RenderPacket::MakeSortKey(materialtemplateID, submesh.MaterialIndex, depth);
        s_Data->PacketQueue.push_back(packet);
    }  
}

void Axel::Renderer::SubmitDebugLine(const Vec3& start, const Vec3& end, const Vec4& color, float duration, bool depthTest)
{
    DebugLinePacket line;
    line.Start = start;
    line.End = end;
    line.Color = color;
    line.Duration = duration;
    line.DepthTest = depthTest;

    RenderPacket packet{};
    packet.SortKey = 0;
    packet.Data = std::move(line);

    s_Data->PacketQueue.push_back(packet);
}

std::shared_ptr<Axel::DescriptorSet>& Axel::Renderer::ProvideTextureDescriptor(const Ref<Texture2D>& texture, Ref<DescriptorSetLayout> layout)
{
    auto device = s_Data->Context->GetDevice();
    auto descriptor = device->GetTextureDescriptor(texture->AssetID,layout);
    if (descriptor)
        return descriptor;

    AXLOG_ERROR("Faield to get texture descriptor: {}", texture->AssetID);
    return descriptor;
}

void Axel::Renderer::RealeaseTextureDescriptor(const Ref<Texture2D>& texture,Ref<Pipeline>& pipeline)
{
    //Not used in vulkan
}

void Axel::Renderer::BindDescriptorSet(uint32_t setIndex, const Ref<DescriptorSet>& set, const Ref<Pipeline>& pipeline)
{
    auto renderAPI = s_Data->Context->GetRenderAPI();
    renderAPI->BindDescriptorSet(setIndex, set);
}

void Axel::Renderer::BindTextureDescriptorSet(uint32_t setIndex, Ref<Texture2D>& texture,Ref<Pipeline>& pipeline)
{
    auto renderAPI = s_Data->Context->GetRenderAPI();
    renderAPI->BindTextureDescriptorSet(setIndex, texture, pipeline);
}

void Axel::Renderer::BindPipeline(Ref<Pipeline>& pipeline)
{
    auto renderAPI = s_Data->Context->GetRenderAPI();
    renderAPI->BindPipeline(pipeline);
}

void Axel::Renderer::SetViewport(float width, float height)
{
    auto renderAPI = s_Data->Context->GetRenderAPI();
    renderAPI->SetViewport(width,height);
}

void Axel::Renderer::SetScissor(float width, float height)
{
    auto renderAPI = s_Data->Context->GetRenderAPI();
    renderAPI->SetScissor(width, height);
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

void Axel::Renderer::ExecuteOpaquePass(const std::vector<RenderPacket>& packets)
{
    if (packets.empty()) return;

    auto renderAPI = s_Data->Context->GetRenderAPI();

    // 1. GLOBAL STATE (Top-level Bindings)
    // All meshes in our RPG share the same Vertex/Index pool
    s_GeometryPool->GetVertexBuffer()->Bind(*s_Data->Context);
    s_GeometryPool->GetIndexBuffer()->Bind(*s_Data->Context);

    // Track state to avoid redundant Vulkan commands
    uint16_t currentPipelineID = 0xFFFF;
    UUID currentMeshHandle = 0; // Optimization: avoid re-binding mesh assets if same

    for (const auto& packet : packets)
    {
        auto& meshData = packet.Get<MeshRenderPacket>();
        uint16_t pipelineID = (uint16_t)(packet.SortKey >> 48);

        // 2. PIPELINE & DESCRIPTOR STATE SWITCH
        // Only run this when the material template changes
        if (pipelineID != currentPipelineID)
        {
            UUID templateID = s_Data->m_MaterialManager->GetMaterialTemplateID(meshData.MaterialIndex);
            auto materialTemplate = AssetManager::GetAsset<MaterialTemplate>(templateID);

            if (materialTemplate)
            {
                auto pipeline = materialTemplate->GetPipeline();
                renderAPI->BindPipeline(pipeline);
                renderAPI->BindDescriptorSet(0, s_Data->SceneDescriptorSet);
                renderAPI->BindDescriptorSet(1, s_Data->m_MaterialManager->GetMaterialDescriptorSet());

                currentPipelineID = pipelineID;
            }
        }


        // 3. OBJECT DATA (The "What" of the draw call)
        // Update Push Constants (Set 2 equivalent)
        struct PushConstants {
            Mat4 Transform;
            uint32_t MaterialIndex;
        } pc;

        pc.Transform = meshData.Transform;
        pc.MaterialIndex = meshData.MaterialIndex;

        // Note: You need a GetPipelineLayout() on your template or pipeline
        auto materialTemplate = AssetManager::GetAsset<MaterialTemplate>(
            s_Data->m_MaterialManager->GetMaterialTemplateID(meshData.MaterialIndex)
        );

        renderAPI->PushConstants(
            materialTemplate->GetPipeline(),
            ShaderStage::Vertex | ShaderStage::Fragment,
            &pc,
            sizeof(PushConstants)
        );

        
        // 4. THE DRAW CALL
        auto meshAsset = AssetManager::GetAsset<Mesh<StaticVertex>>(meshData.MeshHandle);
        const auto& submesh = meshAsset->m_Submeshes[meshData.SubmeshIndex];
        const auto& allocation = meshAsset->m_Allocation; // The global pool position

        renderAPI->DrawIndexed(
            submesh.IndexCount,
            1,
            allocation.IndexOffset + submesh.IndexOffset,  // absolute index start
            allocation.VertexOffset + submesh.VertexOffset,0
        );
    }
}