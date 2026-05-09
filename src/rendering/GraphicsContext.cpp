#include "axelpch.h"
#include "GraphicsContext.h"
#include "rendering/RenderAPI.h"

// Backend headers
#include "rendering/backends/vulkanbackend/VulkanContext.h"

Axel::RenderAPI::API Axel::GraphicsContext::m_APIType = Axel::RenderAPI::API::Vulkan;

std::unique_ptr<Axel::GraphicsContext> Axel::GraphicsContext::Create(void* windowHandle)
{
    switch (m_APIType) {
    case RenderAPI::API::None:
        AXEL_CORE_ASSERT(false, "RendererAPI::None is not supported!");
        return nullptr;

    case RenderAPI::API::Vulkan:
        return CreateScope<VulkanContext>(windowHandle);

    case RenderAPI::API::DX12:
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
