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


    struct AX_API RendererData {
        // The View/Projection data for the current frame
        Mat4 ViewProjection;
        Vec3 CameraPosition;

        // The "Buckets" for sorting and drawing
        std::vector<RenderPacket> MeshPackets;
        std::vector<RenderPacket> UIPackets;
        std::vector<RenderPacket> ParticlePackets;

        // Statistics (Optional but helpful for id Tech-style debugging)
        uint32_t DrawCalls = 0;
        uint32_t IndexCount = 0;

        GraphicsContext* Context;
        Ref<RendererAPI> API;
        Ref<RenderCommandBuffer> ActiveCommandBuffer;
    };

	class AX_API Renderer
	{
    public:
        // Lifecycle
        static void Init(GraphicsContext* context);
        static void Shutdown();

        static void BeginFrame();
        static void EndFrame();

        void BeginRenderPass(Ref<RenderPass> renderPass, bool clear);
        void EndRenderPass(Ref<RenderPass> renderPass);

        // Scene Scope
        static void BeginScene(const CameraComponent& camera, const Mat4& transform);
        static void EndScene();

        // High-level Submissions (The "What")
        static void Submit(const Ref<RenderCommandBuffer>& commandBuffer);
        static void Submit(Ref<Mesh> mesh, Ref<Material> material, const Mat4& transform = Mat4(1.0f));
        static void SubmitParticle(const Particle& particle);
        static void SubmitUI(Ref<UIElement> element);
        static void SubmitInstanced(Ref<Mesh> mesh, Ref<Material> material, const std::vector<glm::mat4>& transforms);

        static void BindDescriptorSet(uint32_t setIndex, const Ref<DescriptorSet>& set, const Ref<Pipeline>& pipeline);

        static void DrawIndexed(uint32_t indexCount);

        static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
        static Ref<RenderCommandBuffer> GetActiveCommandBuffer() { return s_Data->ActiveCommandBuffer; }

    private:
        
        static Scope<RendererData> s_Data;
	};





}



#endif