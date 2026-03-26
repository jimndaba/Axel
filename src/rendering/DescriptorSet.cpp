#include "axelpch.h"
#include "DescriptorSet.h"
#include "backends/vulkanbackend/VulkanContext.h"
#include "RenderAPI.h"
#include "backends//vulkanbackend/VulkanDescriptorSet.h"
#include "backends//vulkanbackend/VulkanPipeline.h"

std::shared_ptr<Axel::DescriptorSet> Axel::DescriptorSet::Create(GraphicsContext* ctxt, const Ref<Pipeline>& pipeline, uint32_t setIndex)
{
    switch (ctxt->GetCurrentAPI())
    {
    case RenderAPI::API::None:
        AXEL_CORE_ASSERT(false, "RendererAPI::None is not supported!");
        return nullptr;

    case RenderAPI::API::Vulkan:
        // We pass the active VulkanDevice to the constructor     
		auto vContext = static_cast<VulkanContext*>(ctxt);
		auto vPipeline = std::static_pointer_cast<VulkanPipeline>(pipeline);
        return std::make_shared<VulkanDescriptorSet>(vContext, vPipeline, setIndex);
    }

    AXEL_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
}
