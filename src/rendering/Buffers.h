#pragma once
#ifndef VERTEXBUFFER_H
#define VERTEXBUFFER_H

#include <core/Core.h>
#include "BufferLayout.h"

namespace Axel
{
	class GraphicsContext;

	class AX_API VertexBuffer
	{
	public:
		virtual ~VertexBuffer() = default;
		virtual void Bind(GraphicsContext& context) const = 0;

		static Ref<VertexBuffer> Create(float* vertices, uint32_t size, GraphicsContext* ctxt);
		virtual void SetLayout(const BufferLayout& layout) { m_Layout = layout; }
		virtual const BufferLayout& GetLayout() const { return m_Layout; }
		virtual void Destroy(GraphicsContext* context) = 0;

	private:
		BufferLayout m_Layout;
	};

	class AX_API IndexBuffer
	{
	public:
		virtual ~IndexBuffer() = default;
		virtual void Bind(GraphicsContext& context) const = 0;
		virtual uint32_t GetCount() const = 0;
		virtual void Destroy(GraphicsContext* context) = 0;
		static Ref<IndexBuffer> Create(uint32_t* indices, uint32_t count, GraphicsContext* ctxt);
	};


	class AX_API UniformBuffer {
	public:
		virtual ~UniformBuffer() = default;
		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) = 0;
		virtual void Destroy(GraphicsContext* context) = 0;
		static Ref<UniformBuffer> Create(GraphicsContext* ctxt, uint32_t size, uint32_t binding);
	};
}
#endif // !VERTEXBUFFER_H
