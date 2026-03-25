#pragma once
#ifndef VKINDEXBUFFER_H
#define VKINDEXBUFFER_H

#include <rendering/Buffers.h>
#include "VulkanBuffer.h"

namespace Axel
{
	class VulkanDevice;
	class GraphicsContext;
	class VulkanBuffer;

	class VkIndexBuffer : public IndexBuffer
	{
	public:
			VkIndexBuffer(const GraphicsContext& context,uint32_t* indices, uint32_t count, VulkanDevice& device);
			~VkIndexBuffer();
			virtual void Bind(GraphicsContext& context) const;
			virtual uint32_t GetCount() const override { return m_Count; }
			virtual void Destroy();
	private:
		VulkanDevice& m_Device;
		VulkanBuffer* m_Buffer;
		uint32_t m_Count;
	};



}

#endif
