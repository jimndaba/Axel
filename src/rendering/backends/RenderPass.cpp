#include "axelpch.h"
#include "RenderPass.h"
#include "../RenderAPI.h"
#include "vulkanbackend/VulkanRenderPass.h"
#include "vulkanbackend/VulkanDevice.h"

std::shared_ptr<Axel::RenderPass > Axel::RenderPass::Create(const RenderPassSpecification& spec, GraphicsDevice* device)
{
    switch (RendererAPI::GetAPI()) {
    case RendererAPI::API::None:    return nullptr;
    case RendererAPI::API::Vulkan:  return CreateRef<VulkanRenderPass>(spec, *static_cast<VulkanDevice*>(device));
    }
    return nullptr;
}
