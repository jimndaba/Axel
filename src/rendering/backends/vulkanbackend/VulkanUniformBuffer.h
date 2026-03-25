#pragma once
#ifndef VULKANUNIFORMBUFFER_H
#define VULKANUNIFORMBUFFER_H

#include <rendering/Buffers.h>
#include <vulkan/vulkan.h>

namespace Axel
{
	class VulkanUniformBuffer : public UniformBuffer
	{
	public:
		VulkanUniformBuffer(GraphicsContext* ctxt, uint32_t size, uint32_t binding);
		virtual ~VulkanUniformBuffer() override;
		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;
		VkBuffer GetBuffer() const { return m_Buffer; }
		virtual void Destroy();
	private:
		uint32_t m_Size;
		uint32_t m_Binding;
		void* m_MappedData;
		VkBuffer m_Buffer;
		VkDeviceMemory m_BufferMemory;
		bool m_IsMapped =false;
	};
}



#endif