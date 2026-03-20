#include "axelpch.h"
#include "Rendergraph.h"
#include "rendering/Renderer.h"
#include "../RenderCommandBuffer.h"

void Axel::RenderGraph::Execute(Ref<RenderCommandBuffer> cmd, const RendererData& data)
{
    for (auto& pass : m_Passes) {
        // The Backend implementation of 'BeginRenderPass' 
        // will handle the actual Vulkan/DX12 transitions
        cmd->Begin();

        // Route packets based on pass name or type
        if (pass->Name == "MainForward")
            pass->Execute(cmd, data.MeshPackets);

        cmd->End();
    }
}
