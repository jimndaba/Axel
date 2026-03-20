#include "axelpch.h"
#include "RenderAPI.h"
#include "backends/vulkanbackend/VulkanRenderAPI.h"

Axel::RendererAPI::API Axel::RendererAPI::s_API = RendererAPI::API::Vulkan;
std::shared_ptr<Axel::RendererAPI> Axel::RendererAPI::Create()
{
    switch (s_API) {
    case RendererAPI::API::None:    return nullptr;
    case RendererAPI::API::Vulkan:  return CreateRef<VulkanRendererAPI>();
    }
    return nullptr;
}
