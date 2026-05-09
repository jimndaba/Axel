#include "axelpch.h"
#include "RenderPass.h"
#include "RenderAPI.h"
#include "backends/vulkanbackend/VulkanRenderPass.h"
#include "backends/vulkanbackend/VulkanDevice.h"
#include "GraphicsContext.h"

std::shared_ptr<Axel::RenderPass > Axel::RenderPass::Create(GraphicsContext* ctxt, const RenderPassSpecification& spec)
{
    switch (ctxt->GetCurrentAPI()) 
    {
        case RenderAPI::API::None:    return nullptr;

        case RenderAPI::API::Vulkan:
        {
		    auto vdevice = std::dynamic_pointer_cast<VulkanDevice>(ctxt->GetDevice()).get();
            return CreateRef<VulkanRenderPass>(spec, *vdevice);
        }
    }
    return nullptr;
}
