#include "axelpch.h"
#include "VulkanBuffer.h"
#include "VulkanDevice.h"

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

Axel::VulkanBuffer::VulkanBuffer(VulkanDevice& device, uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties):
	m_Device(device), Size(size), m_Buffer(VK_NULL_HANDLE)
{
    VkDevice logicalDevice = device.GetLogicalDevice();

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
        // Flag 2: Tell VMA to keep it mapped so we don't have to call vmaMapMemory
        VMA_ALLOCATION_CREATE_MAPPED_BIT;

    vmaCreateBuffer(device.GetAllocator(), &bufferInfo, &allocInfo,
        &m_Buffer, &m_BufferAllocation,&m_AllocationInfo);
}

Axel::VulkanBuffer::~VulkanBuffer()
{
}

void Axel::VulkanBuffer::Destroy(VkDevice device)
{
    if (m_Buffer)
    {	
        vkDeviceWaitIdle(device);
		vkDestroyBuffer(device, m_Buffer, nullptr);
    }
}
