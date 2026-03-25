#pragma once
#ifndef VULKANBUFFER_H
#define VULKANBUFFER_H	

#include <vulkan/vulkan.h>

namespace Axel
{
	class VulkanDevice;
	class VulkanBuffer
	{
	public:
		VulkanBuffer(VulkanDevice& device, uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
		~VulkanBuffer();
		VkBuffer GetHandle() const { return m_Buffer; }
		VkDeviceMemory GetMemory() const { return m_BufferMemory; }
		void Destroy(VkDevice device);

	private:
		VulkanDevice& m_Device;
		VkBuffer m_Buffer;
		VkDeviceSize Size = 0;
		VkDeviceMemory m_BufferMemory;
	};
}

#endif