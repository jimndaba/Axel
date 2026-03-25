#pragma once
#ifndef VKVERTEXBUFFER_H
#define VKVERTEXBUFFER_H

#include <rendering/Buffers.h>


namespace Axel
{
	class VulkanDevice;
	class GraphcisContext;
	class VulkanBuffer;

	class VkVertexBuffer : public VertexBuffer
	{
	public:
		VkVertexBuffer(const GraphicsContext& context,float* vertices, uint32_t size, VulkanDevice& device);
		virtual void Bind(GraphicsContext& context) const;
		virtual void Destroy();
	private:
		VulkanDevice& m_Device;
		VulkanBuffer* m_Buffer;
	};



}

#endif
