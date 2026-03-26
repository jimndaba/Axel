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
		VkVertexBuffer(GraphicsContext& context,float* vertices, uint32_t size);
		virtual void Bind(GraphicsContext& context) const;
		virtual void Destroy(GraphicsContext* context);
	private:
		VulkanBuffer* m_Buffer;
	};



}

#endif
