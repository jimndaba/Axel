#include "axelpch.h"
#include "VulkanBuffer.h"
#include "VulkanDevice.h"

Axel::VulkanBuffer::VulkanBuffer(VulkanDevice& device, uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties):
	m_Device(device), Size(size), m_Buffer(VK_NULL_HANDLE), m_BufferMemory(VK_NULL_HANDLE)
{
    VkDevice logicalDevice = device.GetLogicalDevice();

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vkCreateBuffer(logicalDevice, &bufferInfo, nullptr, &m_Buffer);

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(logicalDevice, m_Buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = device.FindMemoryType(memRequirements.memoryTypeBits, properties);

    vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &m_BufferMemory);
    vkBindBufferMemory(logicalDevice, m_Buffer, m_BufferMemory, 0);
}

Axel::VulkanBuffer::~VulkanBuffer()
{
}

void Axel::VulkanBuffer::Destroy(VkDevice device)
{
    if (m_Buffer)
    {	
        vkDeviceWaitIdle(device);
        vkFreeMemory(device, m_BufferMemory, nullptr);
		vkDestroyBuffer(device, m_Buffer, nullptr);
    }
}
