#include "axelpch.h"
#include "VkVertexBuffer.h"
#include "VulkanBuffer.h"
#include "VulkanDevice.h"
#include "VulkanContext.h"

Axel::VkVertexBuffer::VkVertexBuffer(GraphicsContext& context,float* vertices, uint32_t size)
{
	auto& m_Device = static_cast<VulkanDevice&>(*context.GetDevice().get());
    // 1. Create Staging Buffer (CPU Accessible)
    VulkanBuffer stagingBuffer(m_Device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // 2. Map and Copy Data
    void* data;
    vkMapMemory(m_Device.GetLogicalDevice(), stagingBuffer.GetMemory(), 0, size, 0, &data);
    memcpy(data, vertices, (size_t)size);
    vkUnmapMemory(m_Device.GetLogicalDevice(), stagingBuffer.GetMemory());

    // 3. Create GPU Buffer (Device Local)
    m_Buffer = new VulkanBuffer(m_Device, size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	auto& vkContext = static_cast<const VulkanContext&>(context);
    // 4. Copy from Staging to GPU (Requires a one-time command)
    m_Device.CopyBuffer(stagingBuffer.GetHandle(), m_Buffer->GetHandle(), size);
    stagingBuffer.Destroy(m_Device.GetLogicalDevice());
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
