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
    switch (ctxt->GetCurrentAPI())
    {
    case RenderAPI::API::None:
        AXEL_CORE_ASSERT(false, "RendererAPI::None is not supported!");
        return nullptr;

    case RenderAPI::API::Vulkan:
        // We pass the active VulkanDevice to the constructor
        return std::make_shared<VkVertexBuffer>(*ctxt,vertices, size);
    }

    AXEL_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
}

std::shared_ptr<Axel::IndexBuffer> Axel::IndexBuffer::Create(uint32_t* indices, uint32_t count, GraphicsContext* ctxt)
{
    switch (ctxt->GetCurrentAPI())
    {
    case RenderAPI::API::None:
        AXEL_CORE_ASSERT(false, "RendererAPI::None is not supported!");
        return nullptr;

    case RenderAPI::API::Vulkan:
        // We pass the active VulkanDevice to the constructor		
        return std::make_shared<VkIndexBuffer>(*ctxt, indices, count);
    }

    AXEL_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
}

std::shared_ptr<Axel::UniformBuffer> Axel::UniformBuffer::Create(GraphicsContext* ctxt, uint32_t size, uint32_t binding)
{
    switch (ctxt->GetCurrentAPI())
    {
    case RenderAPI::API::None:
        AXEL_CORE_ASSERT(false, "RendererAPI::None is not supported!");
        return nullptr;

    case RenderAPI::API::Vulkan:
        // We pass the active VulkanDevice to the constructor
        return std::make_shared<VulkanUniformBuffer>(ctxt,size,binding);
    }

    AXEL_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;

}
