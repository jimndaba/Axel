#pragma once
#ifndef VKRENDERAPI
#define VKRENDERAPI

#include "../../RenderAPI.h"
#include <vulkan/vulkan.h>
#include "core/Core.h"
#include "math/Math.h"

namespace Axel
{
	class Texture2D;
    class VulkanRendererAPI : public RendererAPI {
    public:
        virtual void Init() override;
        virtual void SetClearColor(const Vec4& color) override;
        virtual void Clear() override;

        //virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) override;
        virtual void DrawQuad(GraphicsContext* context, const Mat4& transform, const Ref<Texture2D>& texture) override;
        virtual void SubmitCommandBuffer(GraphicsContext* context, Ref<RenderCommandBuffer> commandBuffer) override;
		virtual void BindDescriptorSet(GraphicsContext* context, uint32_t setIndex, const Ref<DescriptorSet>& set, const Ref<Pipeline>& pipeline) override;

    private:
        Vec4 m_ClearColor = { 0.1f, 0.1f, 0.8f, 1.0f };
    };




}

#endif