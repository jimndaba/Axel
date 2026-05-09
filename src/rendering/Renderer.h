#pragma once
#ifndef RENDERER_H
#define RENDERER_H

#include "core/Core.h"
#include "math/Math.h"
#include "RenderPacket.h"
#include "RenderAPI.h"
#include "RenderCommandBuffer.h"
#include "GraphicsContext.h"
#include "render system/IRenderSystem.h"

namespace Axel
{
    class Material;
    class Particle;
    class UIElement;
    class CameraComponent;
    class Texture2D;
    class Pipeline;
    class Shader;
    struct TransformComponent;
    struct SpriteComponent;
    struct IDComponent;
    struct TagComponent;
    struct MeshComponent;
    class VertexBuffer;
    class IndexBuffer;
    class ShaderStorageBuffer;
    class UniformBuffer;
    struct MaterialTemplate;
    class MaterialInstance;
    class MaterialManager;
    class IGeometryPool;
    class DescriptorSetLayout;
   
    struct AX_API RendererData {
  
        std::vector<RenderPacket> PacketQueue;

        // Statistics (Optional but helpful for id Tech-style debugging)
        uint32_t DrawCalls = 0;
        uint32_t IndexCount = 0;
        uint32_t CurrentFrameIndex = 0;
        GraphicsContext* Context = nullptr;
        MaterialManager* m_MaterialManager = nullptr;
        Ref<RenderCommandBuffer> ActiveCommandBuffer;        
        
        Ref<ShaderStorageBuffer> SpriteSSBO;
        Ref<UniformBuffer> SceneUBO;
        Ref<DescriptorSetLayout> m_SceneLayout;
        Ref<DescriptorSet> SceneDescriptorSet;
        std::vector<Ref<DescriptorSet>> MaterialDescriptorSets;
    };

   

	class AX_API Renderer
	{
    public:
        // Lifecycle
        static void Init(GraphicsContext* context, MaterialManager* mat_manager);
        static void Shutdown();

        static void BeginFrame();
        static void EndFrame();

        static void BeginRenderPass(Ref<RenderPass> renderPass,Ref<Framebuffer> framebuffer, bool clear);
        static void EndRenderPass(Ref<RenderPass> renderPass);

        static void OnUpdate();

        // Scene Scope
        static void BeginScene(SceneRenderDesc& desc);
        static void EndScene();
        static void Flush();

        static void RegisterMaterial(const Ref<MaterialInstance>& material);
        static uint32_t GetMaterialIndex(UUID materialID);
        static void BindBuffers();

        // High-level Submissions (The "What")
        static void Submit(const Ref<RenderCommandBuffer>& commandBuffer);     
        static void SubmitParticle(const Particle& particle);

        static void SubmitUI(Ref<UIElement> element);     

        static void SubmitMesh(MeshComponent& meshComp, TransformComponent& transform, float depth = 0.0f);

        static void SubmitDebugLine(
            const Vec3& start,
            const Vec3& end,
            const Vec4& color,
            float       duration,
            bool        depthTest);



        static Ref<DescriptorSet>& ProvideTextureDescriptor(const Ref<Texture2D>& texture, Ref<DescriptorSetLayout> layout);
        static void RealeaseTextureDescriptor(const Ref<Texture2D>& texture,Ref<Pipeline>& pipeline);
        static void BindDescriptorSet(uint32_t setIndex, const Ref<DescriptorSet>& set, const Ref<Pipeline>& pipeline);
        static void BindTextureDescriptorSet(uint32_t setIndex, Ref<Texture2D>& texture,Ref<Pipeline>& pipeline);
        static void BindPipeline(Ref<Pipeline>& pipeline);
        static void SetViewport(float width, float height);
        static void SetScissor(float width, float height);

        static void DrawIndexed(uint32_t indexCount);
        static void DrawIndexedInstanced(uint32_t indexCount, uint32_t instanceCount);
        static void DrawQuad(Mat4 transsform,Ref<Texture2D> texture);

        static void ExecuteOpaquePass(const std::vector<RenderPacket>& packets);
        
        static Ref<RenderCommandBuffer> GetActiveCommandBuffer() { return s_Data->ActiveCommandBuffer; }
		static RenderAPI::API GetCurrentAPI() { return s_Data->Context->GetCurrentAPI(); }
        static GraphicsContext* GetGraphicsContext() { return s_Data->Context; }
        static IGeometryPool* GetGeometryPool() { return s_GeometryPool.get(); }
        static RendererData* GetRenderData () { return s_Data.get(); }
        static Ref<UniformBuffer> GetSceneBuffer() { return  s_Data->SceneUBO; }

    private:        
        static Scope<RendererData> s_Data;
		static Ref<Texture2D> s_WhiteTexture; // A default white texture for materials that don't have one
        static Scope<IGeometryPool> s_GeometryPool;
	};





}



#endif