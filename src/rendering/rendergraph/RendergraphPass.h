#pragma once
#ifndef RENDERGRAPHPASS_H
#define RENDERGRAPHPASS_H

#include <string>
#include "RendergraphTypes.h"
#include "../RenderCommandBuffer.h"
#include "../RenderPacket.h"

#include <functional>

namespace Axel
{       
    enum class AttachmentLoadOp { Load, Clear, DontCare };
    enum class AttachmentStoreOp { Store, DontCare };

    struct RenderPassSpecification {
        std::string DebugName;
        TextureFormatOptions Format;
        AttachmentLoadOp LoadOp = AttachmentLoadOp::Clear;
        AttachmentStoreOp StoreOp = AttachmentStoreOp::Store;
        glm::vec4 ClearColor{ 0.0f, 0.0f, 0.0f, 1.0f };
    };

    class RenderGraphPass {
    public:
  
               

        std::string Name;
    private:

    };






}


#endif