#include "axelpch.h"
#include "FrameBuffer.h"
#include "../RenderAPI.h"
#include "vulkanbackend/VulkanFramebuffer.h"
#include "vulkanbackend/VulkanContext.h"

std::shared_ptr<Axel::Framebuffer> Axel::Framebuffer::Create(GraphicsContext* ctxt, const FramebufferSpecification& spec)
{
    switch (ctxt->GetCurrentAPI()) {
    case RenderAPI::API::None:
        AXEL_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
        return nullptr;

    case RenderAPI::API::Vulkan:
		auto vContext = static_cast<VulkanContext*>(ctxt);
        return CreateRef<VulkanFramebuffer>(vContext,spec);

        // Future-proofing for Axel's DX12 or Metal support
        // case RendererAPI::API::DX12: return CreateRef<DX12Framebuffer>(spec);
    }

    AXEL_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
}
