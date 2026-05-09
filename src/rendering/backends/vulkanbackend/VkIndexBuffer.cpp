#include "axelpch.h"
#include "VkIndexBuffer.h"
#include "VulkanBuffer.h"
#include "VulkanDevice.h"
#include "VulkanContext.h"
#include <core/Logger.h>

Axel::VkIndexBuffer::VkIndexBuffer(GraphicsContext& context, uint32_t* indices, uint32_t count):
	m_Count(count)
{
    VkDeviceSize size = sizeof(uint32_t) * count;
    auto m_Device = std::static_pointer_cast<VulkanDevice>(context.GetDevice()).get();

    // 1. Always create the GPU Buffer (Device Local)
    m_Buffer = new VulkanBuffer(*m_Device, size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // 2. Only upload if we actually have data
    if (indices != nullptr)
    {
        // Create Staging Buffer (CPU Accessible)
        VulkanBuffer stagingBuffer(*m_Device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        // Map and Copy Data
        void* data = stagingBuffer.GetMappedData();
        if (data) {
            memcpy(data, indices, (size_t)size);           
        }

        // Copy from Staging to GPU
        m_Device->CopyBuffer(stagingBuffer.GetHandle(), m_Buffer->GetHandle(), size);

        // Cleanup staging
        stagingBuffer.Destroy(m_Device->GetLogicalDevice());
    }
    else
    {
        AXLOG_TRACE("VulkanIndexBuffer: Reserved {0} bytes for Index Pool", size);
    }
}

Axel::VkIndexBuffer::~VkIndexBuffer()
{
    
}

void Axel::VkIndexBuffer::Bind(GraphicsContext& context) const
{
    // Get the active command buffer from your Renderer or Context
    auto& vkContext = static_cast<VulkanContext&>(context);
    VkCommandBuffer cmd = vkContext.GetActiveCommandBuffer();
    auto bufferHandle = m_Buffer->GetHandle();
	vkCmdBindIndexBuffer(cmd, bufferHandle, 0, VK_INDEX_TYPE_UINT32);
}

void Axel::VkIndexBuffer::Destroy(GraphicsContext* context)
{
    if (m_Buffer)
    {
        auto m_Device = context->GetDevice();
        auto vkDevice = static_cast<VulkanDevice*>(m_Device.get());
        m_Buffer->Destroy(vkDevice->GetLogicalDevice());
        delete m_Buffer;
    }
}
