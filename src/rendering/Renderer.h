#pragma once
#ifndef RENDERER_H
#define RENDERER_H

#include "core/Core.h"
#include "math/Math.h"
#include "RenderPacket.h"
#include "RenderAPI.h"
#include "RenderCommandBuffer.h"
#include "backends/GraphicsContext.h"

namespace Axel
{
    class Mesh;
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
    class MaterialManager;

    class Scene;

    struct SceneCamera {
        Mat4 ViewProjection;
        // You might add Exposure, Near/Far planes, etc. later
    };

    struct AX_API RendererData {
        // The View/Projection data for the current frame
        // The "Buckets" for sorting and drawing
        std::vector<RenderPacket> MeshPackets;
        std::vector<SpriteRenderPacket> SpritePackets;
        std::vector<RenderPacket> UIPackets;
        std::vector<RenderPacket> ParticlePackets;
        std::shared_ptr<MaterialTemplate> m_matTemplate;

        // Statistics (Optional but helpful for id Tech-style debugging)
        uint32_t DrawCalls = 0;
        uint32_t IndexCount = 0;
        uint32_t MaxSprites = 10000;
        uint32_t CurrentFrameIndex = 0;

        SceneCamera CameraData = SceneCamera();
        GraphicsContext* Context = nullptr;
        MaterialManager* m_MaterialManager = nullptr;
        Ref<RenderCommandBuffer> ActiveCommandBuffer;

        Ref<VertexBuffer> QuadVertexBuffer;
        Ref<IndexBuffer> QuadIndexBuffer;
        Ref<Shader> QuadShader;
        Ref<Axel::Texture2D> m_texture;

        Ref<ShaderStorageBuffer> SpriteSSBO;
        Ref<UniformBuffer> CameraUBO;
        Ref<DescriptorSet> MainDescriptorSet;
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

        static void BeginRenderPass(Ref<RenderPass> renderPass, bool clear);
        static void EndRenderPass(Ref<RenderPass> renderPass);

        // Scene Scope
        static void BeginScene(Scene* current_scene);
        static void EndScene();
        static void Flush();

        static void BindBuffers();

        // High-level Submissions (The "What")
        static void Submit(const Ref<RenderCommandBuffer>& commandBuffer);
        static void Submit(Ref<Mesh> mesh, Ref<Material> material, const Mat4& transform = Mat4(1.0f));
        static void SubmitParticle(const Particle& particle);
        static void SubmitUI(Ref<UIElement> element);
        static void SubmitInstanced(Ref<Mesh> mesh, Ref<Material> material, const std::vector<glm::mat4>& transforms);
        static void SubmitSprite(Mat4& transform, Vec4 colour, UUID TextureHandle, UUID Material);

        static Ref<DescriptorSet>& ProvideTextureDescriptor(const Ref<Texture2D>& texture,Ref<Pipeline>& pipeline, uint32_t index);
        static void RealeaseTextureDescriptor(const Ref<Texture2D>& texture,Ref<Pipeline>& pipeline);
        static void BindDescriptorSet(uint32_t setIndex, const Ref<DescriptorSet>& set, const Ref<Pipeline>& pipeline);
        static void BindTextureDescriptorSet(uint32_t setIndex, Ref<Texture2D>& texture,Ref<Pipeline>& pipeline);

        static void DrawIndexed(uint32_t indexCount);
        static void DrawIndexedInstanced(uint32_t indexCount, uint32_t instanceCount);
        static void DrawQuad(Mat4 transsform,Ref<Texture2D> texture);
        
        static Ref<RenderCommandBuffer> GetActiveCommandBuffer() { return s_Data->ActiveCommandBuffer; }

		static RenderAPI::API GetCurrentAPI() { return s_Data->Context->GetCurrentAPI(); }
        static GraphicsContext* GetGraphicsContext() { return s_Data->Context; }

        static RendererData* GetRenderData () { return s_Data.get(); }

    private:        
        static Scope<RendererData> s_Data;
        static Scope<SceneCamera> s_SceneCamera;
		static Ref<Texture2D> s_WhiteTexture; // A default white texture for materials that don't have one
	};





}



#endif