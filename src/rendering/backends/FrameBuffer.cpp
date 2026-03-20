#include "axelpch.h"
#include "FrameBuffer.h"
#include "../RenderAPI.h"
#include "vulkanbackend/VulkanFramebuffer.h"
#include "vulkanbackend/VulkanDevice.h"

std::shared_ptr<Axel::Framebuffer> Axel::Framebuffer::Create(const FramebufferSpecification& spec, GraphicsDevice* device)
{
    switch (RendererAPI::GetAPI()) {
    case RendererAPI::API::None:
        AXEL_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
        return nullptr;

    case RendererAPI::API::Vulkan:
        return CreateRef<VulkanFramebuffer>(spec,*static_cast<VulkanDevice*>(device));

        // Future-proofing for Axel's DX12 or Metal support
        // case RendererAPI::API::DX12: return CreateRef<DX12Framebuffer>(spec);
    }

    AXEL_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
}
