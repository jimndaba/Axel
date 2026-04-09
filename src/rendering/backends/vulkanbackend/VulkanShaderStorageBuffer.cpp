#include "axelpch.h"
#include <core/Logger.h>
#include "VulkanShaderStorageBuffer.h"
#include "VulkanDevice.h"
#include "VulkanContext.h"
#include <rendering/Renderer.h>

Axel::VulkanShaderStorageBuffer::VulkanShaderStorageBuffer(GraphicsContext* ctxt, uint32_t size, uint32_t binding):
	m_Size(size), m_Binding(binding)
{
    auto device = std::static_pointer_cast<VulkanDevice>(ctxt->GetDevice());

    // 1. Create the Buffer
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
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
    allocInfo.memoryTypeIndex = device->FindMemoryType(memRequirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(device->GetLogicalDevice(), &allocInfo, nullptr, &m_BufferMemory) != VK_SUCCESS) {
        AXLOG_ERROR("VulkanShaderStorageBuffer: Failed to allocate buffer memory!");
    }

    // 3. Bind Memory to Buffer
    vkBindBufferMemory(device->GetLogicalDevice(), m_Buffer, m_BufferMemory, 0);

    // 4. Persistent Mapping
    // We map it once and keep the pointer for the lifetime of the object	
    vkMapMemory(device->GetLogicalDevice(), m_BufferMemory, 0, size, 0, &m_MappedData);
    m_IsMapped = true;
}

Axel::VulkanShaderStorageBuffer::~VulkanShaderStorageBuffer()
{
}

void Axel::VulkanShaderStorageBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
{
    // Ensure we don't overflow the allocated buffer size
    uint32_t uploadSize = std::min(size, m_Size - offset);

    memcpy((uint8_t*)m_MappedData + offset, data, uploadSize);

    // If memory isn't Host Coherent, you would need vkFlushMappedMemoryRanges here.
    // Given your performance targets, Coherent memory is preferred for this use case.
}

void Axel::VulkanShaderStorageBuffer::Bind(GraphicsContext& ctxt, uint32_t binding)
{
    auto device = std::static_pointer_cast<VulkanDevice>(ctxt.GetDevice());
    auto commandBuffer = Renderer::GetActiveCommandBuffer();
    //Renderer::UpdateBufferDescriptor(binding, , VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
}

void Axel::VulkanShaderStorageBuffer::Destroy(GraphicsContext* context)
{
}
