#pragma once
#ifndef RENDERAPI_H
#define RENDERAPI_H

#include "core/Core.h"
#include "math/Math.h"

namespace Axel
{
    class RenderCommandBuffer;
    class GraphicsContext;

    class RendererAPI {
    public:
        enum class API {
            None = 0, Vulkan = 1, DX12 = 2
        };

        static API GetAPI() { return s_API; }
        static void SetAPI(API api) { s_API = api; }


        static Ref<RendererAPI> Create();

        virtual void Init() =0;
        virtual void SetClearColor(const Vec4& color) = 0;
        virtual void Clear() = 0;
        virtual void SubmitCommandBuffer(GraphicsContext* context, Ref<RenderCommandBuffer> commandBuffer) = 0;

    private:
        static API s_API;
    };
}

#endif