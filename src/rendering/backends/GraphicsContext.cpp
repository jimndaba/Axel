#include "axelpch.h"
#include "GraphicsContext.h"
#include "rendering/RenderAPI.h"

// Backend headers
#include "rendering/backends/vulkanbackend/VulkanContext.h"


std::unique_ptr<Axel::GraphicsContext> Axel::GraphicsContext::Create(void* windowHandle)
{
    switch (RendererAPI::GetAPI()) {
    case RendererAPI::API::None:
        AXEL_CORE_ASSERT(false, "RendererAPI::None is not supported!");
        return nullptr;

    case RendererAPI::API::Vulkan:
        return CreateScope<VulkanContext>(windowHandle);

    case RendererAPI::API::DX12:
#ifdef AXEL_PLATFORM_WINDOWS
        return CreateScope<DX12Context>(windowHandle);
#else
        AXEL_CORE_ASSERT(false, "DX12 is only supported on Windows!");
        return nullptr;
#endif
    }

    AXEL_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
}
