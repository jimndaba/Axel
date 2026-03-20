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
        ImageFormat Format;
        AttachmentLoadOp LoadOp = AttachmentLoadOp::Clear;
        AttachmentStoreOp StoreOp = AttachmentStoreOp::Store;
        glm::vec4 ClearColor{ 0.0f, 0.0f, 0.0f, 1.0f };
    };

    class RenderGraphPass {
    public:
        using ExecuteFunc = std::function<void(Ref<RenderCommandBuffer>, const std::vector<RenderPacket>&)>;

        RenderGraphPass(const std::string& name, ExecuteFunc func)
            : Name(name), m_Execute(func) {}

        void Execute(Ref<RenderCommandBuffer> cmd, const std::vector<RenderPacket>& packets) {
            m_Execute(cmd, packets);
        }

        std::string Name;
    private:
        ExecuteFunc m_Execute;
    };






}


#endif