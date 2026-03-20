#include "axelpch.h"
#include "VulkanCommanPool.h"
#include "../../../core/Logger.h"

Axel::VulkanCommandPool::VulkanCommandPool(VulkanDevice& device):
	m_Device(device)
{
    auto indices = m_Device.GetQueueFamilyIndices();

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = indices.GraphicsFamily.value();

    VkResult result = vkCreateCommandPool(m_Device.GetLogicalDevice(), &poolInfo, nullptr, &m_Pool);
    if (result != VK_SUCCESS)
    {
        AXLOG_ERROR("Failed to create Vulkan Command Pool!");
    }
}

Axel::VulkanCommandPool::~VulkanCommandPool()
{
    //if(m_Pool)vkDestroyCommandPool(m_Device.GetLogicalDevice(), m_Pool, nullptr);
}

VkCommandBuffer Axel::VulkanCommandPool::AllocateBuffer(bool begin)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_Pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer{};
    AXEL_CORE_ASSERT(vkAllocateCommandBuffers(m_Device.GetLogicalDevice(), &allocInfo, &commandBuffer) == VK_SUCCESS,
        "Failed to allocate Command Buffer!");

    if (begin) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(commandBuffer, &beginInfo);
    }

    return commandBuffer;
}

void Axel::VulkanCommandPool::FreeBuffer(VkCommandBuffer buffer)
{
    vkFreeCommandBuffers(m_Device.GetLogicalDevice(), m_Pool, 1, &buffer);
}
