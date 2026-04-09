#pragma once
#ifndef VULKANSHADERSTORAGEBUFFER_H
#define VULKANSHADERSTORAGEBUFFER_H

#include <rendering/Buffers.h>
#include <vulkan/vulkan.h>

namespace Axel
{
	class VulkanShaderStorageBuffer : public ShaderStorageBuffer
	{
	public:
		VulkanShaderStorageBuffer(GraphicsContext* ctxt, uint32_t size, uint32_t binding);
		virtual ~VulkanShaderStorageBuffer() override;
		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;
		virtual void Bind(GraphicsContext& ctxt, uint32_t binding) override;
		VkBuffer GetBuffer() const { return m_Buffer; }
		virtual void Destroy(GraphicsContext* context);
		virtual uint32_t GetSize() override { return m_Size; }
	private:
		uint32_t m_Size;
		uint32_t m_Binding;
		void* m_MappedData;
		VkBuffer m_Buffer;
		VkDeviceMemory m_BufferMemory;
		bool m_IsMapped = false;
	};
}



#endif