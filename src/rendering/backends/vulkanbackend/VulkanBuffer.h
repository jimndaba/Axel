#pragma once
#ifndef VULKANBUFFER_H
#define VULKANBUFFER_H	

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>


#include <rendering/Buffers.h>





namespace Axel
{
	class VulkanDevice;
	class VulkanBuffer 
	{
	public:
		VulkanBuffer(VulkanDevice& device, uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
		~VulkanBuffer();
		VkBuffer GetHandle() const { return m_Buffer; }		
		void* GetMappedData() const {
			return m_AllocationInfo.pMappedData;
		}
		void Destroy(VkDevice device);

	private:
		VulkanDevice& m_Device;
		VkBuffer m_Buffer;
		VkDeviceSize Size = 0;
		VmaAllocation m_BufferAllocation = nullptr;		
		VmaAllocationInfo m_AllocationInfo; // Store this during creation!
	};
}

#endif