#pragma once
#ifndef VULKANGEOPOOL_H
#define VULKANGEOPOOL_H

#include <rendering/Mesh.h>
#include <rendering/IGeometryPool.h>
#include "VkIndexBuffer.h"
#include "VkVertexBuffer.h"

namespace Axel
{
    class VulkanContext;
    class VulkanGeometryPool : public IGeometryPool {
    public:
        VulkanGeometryPool(GraphicsContext* context, uint32_t vertexPoolSize, uint32_t indexPoolSize);
        virtual ~VulkanGeometryPool() override;

        VulkanGeometryPool(const VulkanGeometryPool&) = delete;
        VulkanGeometryPool& operator=(const VulkanGeometryPool&) = delete;

        Ref<VertexBuffer>& GetVertexBuffer() { return m_GlobalVertexBuffer; };
       Ref<IndexBuffer>& GetIndexBuffer() { return m_GlobalIndexBuffer; };

    protected:
        MeshAllocation CommitAllocation(
            const void* vData, uint32_t vByteSize,
            const void* iData, uint32_t iByteSize,
            uint32_t indexCount, uint32_t vertexStride
        ) override;

    private:
        GraphicsContext* mContext;
        // The actual GPU memory blocks
        Ref<VertexBuffer> m_GlobalVertexBuffer;
        Ref<IndexBuffer> m_GlobalIndexBuffer;

        // Write heads (tracked in units/counts, not bytes, for the MeshAllocation)
        uint32_t m_CurrentVertexOffset = 0;
        uint32_t m_CurrentIndexOffset = 0;

        // Size limits to prevent overflow
        uint32_t m_MaxVertexBytes;
        uint32_t m_MaxIndexBytes;

        uint32_t m_VertexCountHead = 0; // Current vertex count
        uint32_t m_IndexCountHead = 0;  // Current index count
        uint32_t m_VertexByteOffset = 0; // Current byte offset for vertices
        uint32_t m_IndexByteOffset = 0;  // Current byte offset for indices

        void PushToGPU(const void* data, uint32_t size, uint32_t offset, bool isVertexBuffer);
    };





}


#endif