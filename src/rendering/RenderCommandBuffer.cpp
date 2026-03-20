#include "axelpch.h"
#include "RenderCommandBuffer.h"
#include "RenderAPI.h"
#include "backends/vulkanbackend/VulkanContext.h"
#include "backends/vulkanbackend/VulkanCommandBuffer.h"


std::shared_ptr<Axel::RenderCommandBuffer > Axel::RenderCommandBuffer::Create(GraphicsContext* ctxt)
{
    switch (RendererAPI::GetAPI()) {
    case RendererAPI::API::None:    return nullptr;
    case RendererAPI::API::Vulkan:
    {
        auto context = static_cast<VulkanContext*>(ctxt);
        auto device = context->GetDevice();
        auto pool = context->GetCommandPool()->GetHandle();

        return CreateRef<VulkanCommandBuffer>(*device, pool);
    }
    }

    AXEL_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
}
