#include "axelpch.h"
#include "VulkanUniformBuffer.h"
#include "VulkanDevice.h"
#include "VulkanContext.h"
#include <core/Logger.h>

Axel::VulkanUniformBuffer::VulkanUniformBuffer(GraphicsContext* ctxt, uint32_t size, uint32_t binding)
	:m_Size(size), m_Binding(binding)
{
	auto device = std::static_pointer_cast<VulkanDevice>(ctxt->GetDevice());

    // 1. Create the Buffer
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device->GetLogicalDevice(), &bufferInfo, nullptr, &m_Buffer) != VK_SUCCESS) {
        AXLOG_ERROR("VulkanUniformBuffer: Failed to create buffer!");
    }
    
    // 2. Memory Requirements & Allocation
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device->GetLogicalDevice(), m_Buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    // Helper function to find memory type that is Host Visible + Coherent
    allocInfo.memoryTypeIndex =device->FindMemoryType(memRequirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(device->GetLogicalDevice(), &allocInfo, nullptr, &m_BufferMemory) != VK_SUCCESS) {
        AXLOG_ERROR("VulkanUniformBuffer: Failed to allocate buffer memory!");
    }

    // 3. Bind Memory to Buffer
    vkBindBufferMemory(device->GetLogicalDevice(), m_Buffer, m_BufferMemory, 0);

    // 4. Persistent Mapping
    // We map it once and keep the pointer for the lifetime of the object	
    vkMapMemory(device->GetLogicalDevice(), m_BufferMemory, 0, size, 0, &m_MappedData);
    m_IsMapped = true;
}

Axel::VulkanUniformBuffer::~VulkanUniformBuffer()
{
	
}

void Axel::VulkanUniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
{
    // Basic safety check: don't overflow the buffer
    if (offset + size <= m_Size) {
        memcpy((uint8_t*)m_MappedData + offset, data, size);
    }
    else {
        AXLOG_WARN("UniformBuffer: Attempted to write out of bounds!");
    }
}

void Axel::VulkanUniformBuffer::Destroy(GraphicsContext* context)
{
    if (m_Buffer != VK_NULL_HANDLE)
    {
         auto m_Device = context->GetDevice();
        auto device = static_cast<VulkanDevice*>(m_Device.get())->GetLogicalDevice();
	

        // 1. Unmap first (if it was mapped)
        if (m_IsMapped) {
            vkUnmapMemory(device, m_BufferMemory);
            m_IsMapped = false;
        }

        // 2. Destroy the Buffer handle
        vkDestroyBuffer(device, m_Buffer, nullptr);
        m_Buffer = VK_NULL_HANDLE;

        // 3. Free the actual GPU memory
        if (m_BufferMemory != VK_NULL_HANDLE) {
            vkFreeMemory(device, m_BufferMemory, nullptr);
            m_BufferMemory = VK_NULL_HANDLE;
        }
    }
}
