#pragma once
#ifndef RENDERGRAPH_H
#define RENDERGRAPH_H

#include "core/Core.h"
#include "RendergraphTypes.h"
#include <string>
#include <functional>
#include "RendergraphPass.h"

namespace Axel
{
    struct RenderGraphBuilder {
        // Tells the graph: "I will write to this texture"
        virtual void Write(RenderGraphResourceID resource) = 0;
        // Tells the graph: "I need to read from this texture"
        virtual void Read(RenderGraphResourceID resource) = 0;
    };

    struct RendererData;
    class RenderCommandBuffer;

    class RenderGraph {
    public:
        // Setup a pass without knowing the backend
        
        void Execute(Ref<RenderCommandBuffer> cmd, const RendererData& data);
    private:
        std::vector<Scope<RenderGraphPass>> m_Passes;
    };





}


#endif