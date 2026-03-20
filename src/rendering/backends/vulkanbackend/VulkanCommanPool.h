#pragma once
#ifndef VKCOMMANDPOOL_H
#define VKCOMMANDPOOL_H

#include <vulkan/vulkan.h>
#include "VulkanDevice.h"


namespace Axel
{

    class VulkanCommandPool {
    public:
        VulkanCommandPool(VulkanDevice& device);
        ~VulkanCommandPool();

        VkCommandPool GetHandle() const { return m_Pool; }

        // Helper to allocate individual buffers from this pool
        VkCommandBuffer AllocateBuffer(bool begin = false);
        void FreeBuffer(VkCommandBuffer buffer);

    private:
        VulkanDevice& m_Device;
        VkCommandPool m_Pool = VK_NULL_HANDLE;
    };





}



#endif