#include "axelpch.h"
#include "RenderAPI.h"
#include "GraphicsContext.h"
#include "DescriptorSetLayout.h"
#include "backends/vulkanbackend/VulkanDescriptorSetLayout.h"
#include "backends/vulkanbackend/VulkanContext.h"

namespace Axel
{
    Ref<DescriptorSetLayout> DescriptorSetLayout::Create(GraphicsContext* ctxt, const std::initializer_list<DescriptorBinding>& bindings)
    {
        return Create(ctxt,std::vector<DescriptorBinding>(bindings));
    }

    Ref<DescriptorSetLayout> DescriptorSetLayout::Create(GraphicsContext* ctxt, const std::vector<DescriptorBinding>& bindings)
    {
        // Use a global/static accessor or pass a context if available. 
        // Following your Framebuffer pattern:
        
        RenderAPI::API api = ctxt->GetCurrentAPI();

        switch (api)
        {
        case RenderAPI::API::None:
            AXEL_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
            return nullptr;

        case RenderAPI::API::Vulkan:
            // We cast the global context to VulkanContext to satisfy the backend

            return CreateRef<VulkanDescriptorSetLayout>(static_cast<VulkanContext*>(ctxt),bindings);
        }

        AXEL_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }
}