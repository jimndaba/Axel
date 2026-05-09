#include "axelpch.h"
#include "IGeometryPool.h"
#include "GraphicsContext.h"
#include "backends/vulkanbackend/VulkanContext.h"
#include "backends/vulkanbackend/VulkanGeometryPool.h"

std::unique_ptr<Axel::IGeometryPool> Axel::IGeometryPool::Create(GraphicsContext* ctxt, uint32_t vertexPoolSize, uint32_t indexPoolSize)
{
    switch (ctxt->GetCurrentAPI())
    {
    case RenderAPI::API::None:
        AXEL_CORE_ASSERT(false, "RendererAPI::None is not supported!");
        return nullptr;

    case RenderAPI::API::Vulkan:
        // We pass the active VulkanDevice to the constructor     
        auto vContext = static_cast<VulkanContext*>(ctxt);
        return std::make_unique<VulkanGeometryPool>(vContext,vertexPoolSize,indexPoolSize);
    }

    AXEL_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
}
