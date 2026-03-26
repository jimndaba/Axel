#include "axelpch.h"
#include "RenderCommandBuffer.h"
#include "RenderAPI.h"
#include "backends/vulkanbackend/VulkanContext.h"
#include "backends/vulkanbackend/VulkanCommandBuffer.h"


std::shared_ptr<Axel::RenderCommandBuffer > Axel::RenderCommandBuffer::Create(GraphicsContext* ctxt)
{
    switch (ctxt->GetCurrentAPI()) {
    case RenderAPI::API::None:    return nullptr;
    case RenderAPI::API::Vulkan:
    {
        auto context = static_cast<VulkanContext*>(ctxt);
		auto device = std::dynamic_pointer_cast<VulkanDevice>(context->GetDevice());
        auto pool = context->GetCommandPool()->GetHandle();
        return CreateRef<VulkanCommandBuffer>(*device, pool);
    }
    }

    AXEL_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
}
