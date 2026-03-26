#include <axelpch.h>
#include "Pipeline.h"
#include "backends/vulkanbackend/VulkanPipeline.h"
#include "backends/GraphicsContext.h"
#include "backends/vulkanbackend/VulkanDevice.h"

std::shared_ptr<Axel::Pipeline> Axel::Pipeline::Create(GraphicsContext* ctxt, const PipelineSpecification& spec)
{
    switch (ctxt->GetCurrentAPI())
    {
    case RenderAPI::API::None:
        AXEL_CORE_ASSERT(false, "RendererAPI::None is not supported!");
        return nullptr;

    case RenderAPI::API::Vulkan:
        // We create the concrete Vulkan implementation
		auto device = std::dynamic_pointer_cast<VulkanDevice>(ctxt->GetDevice());
        return CreateRef<VulkanPipeline>(spec,*device);
    }

    AXEL_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
}
