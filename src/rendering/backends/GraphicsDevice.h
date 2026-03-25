#pragma once
#ifndef GRAPHICSDEVICE_H
#define GRAPHICSDEVICE_H

#include "core/Core.h"
#include <string>

namespace Axel {

    struct Buffer;
    class Texture;
    struct TextureSpecification;

    struct DeviceCapabilities {
        std::string RendererName;
        uint32_t MaxTextureSize;
        bool SupportsRayTracing;
    };

    class GraphicsDevice {
    public:
        GraphicsDevice();
        virtual ~GraphicsDevice() = default;

        virtual void WaitIdle() = 0;
        virtual const DeviceCapabilities& GetCaps() const = 0;

        // Resource Creation (Abstracted)
        virtual Ref<Buffer> CreateVertexBuffer(uint32_t size) = 0;
        virtual Ref<Texture> CreateTexture(const TextureSpecification& spec) = 0;

        // Internal singleton-style access
        static GraphicsDevice& Get() { return *s_Instance; }

    protected:
        static GraphicsDevice* s_Instance;
    };
}

#endif