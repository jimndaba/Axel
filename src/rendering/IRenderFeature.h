#pragma once
#ifndef IRENDERFEATURE_H
#define IRENDERFEATURE_H

#include <math/Math.h>
#include <core/Core.h>

namespace Axel
{
    class CameraComponent;
    class EditorCamera;

    struct RenderContext {
       Ref<EditorCamera> Camera;
       Ref<CameraComponent> MainCamera;
       uint32_t ViewportWidth = 1;
       uint32_t ViewportHeight = 1;

       uint32_t ScissorWidth = 1;
       uint32_t ScissorHeight = 1;
    };

    class IRenderFeature {
    public:
        virtual ~IRenderFeature() = default;
        virtual void OnRender(RenderContext& ctx) = 0;
        virtual const char* GetName() const = 0;
    };

}

#endif