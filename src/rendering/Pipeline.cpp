#include <axelpch.h>
#include "Pipeline.h"

#include <rendering/RenderAPI.h>
#include "backends/vulkanbackend/VulkanPipeline.h"
#include "backends/GraphicsDevice.h"
#include "backends/vulkanbackend/VulkanDevice.h"

std::shared_ptr<Axel::Pipeline> Axel::Pipeline::Create(const PipelineSpecification& spec, GraphicsDevice* gd)
{
    switch (RendererAPI::GetAPI())
    {
    case RendererAPI::API::None:
        AXEL_CORE_ASSERT(false, "RendererAPI::None is not supported!");
        return nullptr;

    case RendererAPI::API::Vulkan:
        // We create the concrete Vulkan implementation
		auto device = dynamic_cast<VulkanDevice*>(gd);
        return CreateRef<VulkanPipeline>(spec,*device);
    }

    AXEL_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
}
