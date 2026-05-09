#include "axelpch.h"
#include "VkVertexBuffer.h"
#include "VulkanBuffer.h"
#include "VulkanDevice.h"
#include "VulkanContext.h"
#include <core/Logger.h>

Axel::VkVertexBuffer::VkVertexBuffer(GraphicsContext& context,float* vertices, uint32_t size)
{
    auto& m_Device = static_cast<VulkanDevice&>(*context.GetDevice().get());

    // 1. Always create the GPU Buffer (Device Local)
    // We need this regardless of whether we have data now or later.
    m_Buffer = new VulkanBuffer(m_Device, size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // 2. Only perform the upload if vertices are actually provided
    if (vertices != nullptr)
    {
        // Create Staging Buffer (CPU Accessible)
        VulkanBuffer stagingBuffer(m_Device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        // Map and Copy Data
        void* data = stagingBuffer.GetMappedData();
        if (data) {
            memcpy(data, vertices, (size_t)size);
        }


        // Copy from Staging to GPU
        m_Device.CopyBuffer(stagingBuffer.GetHandle(), m_Buffer->GetHandle(), size);

        // Staging buffer is destroyed automatically when it goes out of scope 
        // if your VulkanBuffer RAII handles it, otherwise call Destroy.
        stagingBuffer.Destroy(m_Device.GetLogicalDevice());
    }
    else
    {
        AXLOG_TRACE("VulkanVertexBuffer: Allocated {0} bytes of empty GPU memory (Pool Mode)", size);
    }
}

void Axel::VkVertexBuffer::Bind(GraphicsContext& context) const
{
    // Get the active command buffer from your Renderer or Context
	auto& vkContext = static_cast<VulkanContext&>(context);
    VkCommandBuffer cmd = vkContext .GetActiveCommandBuffer();
	auto bufferHandle = m_Buffer->GetHandle();

    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(cmd, 0, 1, &bufferHandle, offsets);
}

void Axel::VkVertexBuffer::Destroy(GraphicsContext* ctxt)
{
    if (m_Buffer)
    {
        auto m_Device = ctxt->GetDevice();
		auto vkDevice = static_cast<VulkanDevice*>(m_Device.get());
        m_Buffer->Destroy(vkDevice->GetLogicalDevice());
        delete m_Buffer;
    }
}
