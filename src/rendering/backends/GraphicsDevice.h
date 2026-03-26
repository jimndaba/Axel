#pragma once
#ifndef GRAPHICSDEVICE_H
#define GRAPHICSDEVICE_H

#include "core/Core.h"
#include "core/UUID.h"
#include <string>

namespace Axel {

    struct Buffer;
    class Texture2D;
    class Mesh;
    class VertexBuffer;
	class IndexBuffer;  
    class VulkanContext;

    struct DeviceCapabilities {
        std::string RendererName;
        uint32_t MaxTextureSize;
        bool SupportsRayTracing;
    };

    class GraphicsDevice {
    public:
        GraphicsDevice();
        virtual ~GraphicsDevice() = default;

        virtual void Init() = 0;
        virtual void Shutdown() =0;
        virtual void WaitIdle() = 0;
        virtual const DeviceCapabilities& GetCaps() const = 0;

        // Resource Creation (Abstracted)
        virtual Ref<VertexBuffer> CreateVertexBuffer(float* vertices, uint32_t size) = 0;
        virtual Ref<IndexBuffer> CreateIndexBuffer(uint32_t* indices, uint32_t count) = 0;
        virtual Ref<Texture2D> CreateTexture(uint32_t width, uint32_t height, const unsigned char* data) = 0;

        virtual bool UploadTexture(Ref<Texture2D> texture) = 0;
        virtual bool UploadMesh(Ref<Mesh> mesh) =0;
        virtual bool UploadBuffer(Ref<Buffer> buffer)=0;
        virtual void UnloadTexture(UUID textureID) =0;
        virtual bool IsTextureResident(UUID textureID) const =0;
    protected:
      
    };
}

#endif