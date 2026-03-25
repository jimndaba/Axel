#include "axelpch.h"
#include "Buffers.h"
#include <rendering/RenderAPI.h>
#include "backends/vulkanbackend/VkVertexBuffer.h"
#include "backends/vulkanbackend/VkIndexBuffer.h"
#include "backends/vulkanbackend/VulkanUniformBuffer.h"
#include "backends/vulkanbackend/VulkanDevice.h"

#include "backends/GraphicsContext.h"
#include "backends/GraphicsDevice.h"

std::shared_ptr<Axel::VertexBuffer> Axel::VertexBuffer::Create(float* vertices, uint32_t size, GraphicsContext* ctxt)
{
    switch (RendererAPI::GetAPI())
    {
    case RendererAPI::API::None:
        AXEL_CORE_ASSERT(false, "RendererAPI::None is not supported!");
        return nullptr;

    case RendererAPI::API::Vulkan:
        // We pass the active VulkanDevice to the constructor
        auto device = static_cast<VulkanDevice*>(ctxt->GetDevice());
        return std::make_shared<VkVertexBuffer>(*ctxt,vertices, size,*device);
    }

    AXEL_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
}

std::shared_ptr<Axel::IndexBuffer> Axel::IndexBuffer::Create(uint32_t* indices, uint32_t count, GraphicsContext* ctxt)
{
    switch (RendererAPI::GetAPI())
    {
    case RendererAPI::API::None:
        AXEL_CORE_ASSERT(false, "RendererAPI::None is not supported!");
        return nullptr;

    case RendererAPI::API::Vulkan:
        // We pass the active VulkanDevice to the constructor
		auto device = static_cast<VulkanDevice*>(ctxt->GetDevice());
        return std::make_shared<VkIndexBuffer>(*ctxt, indices, count, *device);
    }

    AXEL_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
}

std::shared_ptr<Axel::UniformBuffer> Axel::UniformBuffer::Create(GraphicsContext* ctxt, uint32_t size, uint32_t binding)
{
    switch (RendererAPI::GetAPI())
    {
    case RendererAPI::API::None:
        AXEL_CORE_ASSERT(false, "RendererAPI::None is not supported!");
        return nullptr;

    case RendererAPI::API::Vulkan:
        // We pass the active VulkanDevice to the constructor
        auto device = static_cast<VulkanDevice*>(ctxt->GetDevice());
        return std::make_shared<VulkanUniformBuffer>(ctxt,size,binding);
    }

    AXEL_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;

}
