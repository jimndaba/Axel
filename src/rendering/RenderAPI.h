#pragma once
#ifndef RENDERAPI_H
#define RENDERAPI_H

#include "core/Core.h"
#include "math/Math.h"
#include "GraphicsCore.h"

namespace Axel
{
    class GraphicsContext;
    class RenderCommandBuffer;
    class DescriptorSet;
    class Pipeline;
    class Texture2D;

    class AX_API RenderAPI {
    public:

        virtual ~RenderAPI() = default;


        virtual const char* GetAPIName() const = 0;
        virtual std::string GetAPIInfo() const = 0;
       
        virtual void Init() =0;
        virtual void Shutdown() = 0;

        virtual void Clear() = 0;
        virtual void SetClearColor(const Vec4& color) = 0;
       
        virtual void SubmitCommandBuffer(Ref<RenderCommandBuffer> commandBuffer) = 0;
        virtual void BindDescriptorSet(uint32_t setIndex, const Ref<DescriptorSet>& set, const Ref<Pipeline>& pipeline) = 0;
        virtual void BindTextureDescriptorSet(uint32_t setIndex, Ref<Texture2D>& texture,Ref<Pipeline>& pipeline) = 0;

        virtual void DrawQuad(const Mat4& transform, const Ref<Texture2D>& texture) = 0;
        virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1) = 0;

        virtual void PushConstants(
            Ref<Pipeline> pipeline,
            ShaderStage stages, // e.g. Vertex | Fragment
            const void* data,
            uint32_t size,
            uint32_t offset = 0
        ) = 0;

        enum class API {
            None = 0,
            Vulkan = 1,
            DX12 = 2,
            Metal = 3
        };

        static API GetCurrentAPI();

	protected:
        virtual GraphicsContext* GetContext() = 0;
    };
}

#endif