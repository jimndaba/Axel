#include "axelpch.h"
#include "RenderAPI.h"
#include "DescriptorSet.h"
#include "GraphicsContext.h"
#include "backends/vulkanbackend/VulkanContext.h"
#include "backends/vulkanbackend/VulkanDescriptorSet.h"
#include "backends/vulkanbackend/VulkanPipeline.h"


std::shared_ptr<Axel::DescriptorSet> Axel::DescriptorSet::Create(GraphicsContext* ctxt, const Ref<DescriptorSetLayout>& setlayout)
{
    switch (ctxt->GetCurrentAPI())
    {
    case RenderAPI::API::None:
        AXEL_CORE_ASSERT(false, "RendererAPI::None is not supported!");
        return nullptr;

    case RenderAPI::API::Vulkan:
        // We pass the active VulkanDevice to the constructor     
		auto vContext = static_cast<VulkanContext*>(ctxt);
        return std::make_shared<VulkanDescriptorSet>(vContext,setlayout);
    }

    AXEL_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
}
