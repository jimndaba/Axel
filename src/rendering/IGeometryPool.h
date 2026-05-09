#pragma once
#ifndef IGEOMETRYPOOL_H
#define IGEOMETRYPOOL_H

#include <core/Core.h>
#include "Mesh.h"

namespace Axel
{
    class GraphicsContext;
    class VertexBuffer;
    class IndexBuffer;
    class IGeometryPool {
    public:
        virtual ~IGeometryPool() = default;

        // The Front-End only calls this. It doesn't care how the copy happens.
        template<typename T>
        MeshAllocation Allocate(const Mesh<T>& meshData) {
            uint32_t vSize = static_cast<uint32_t>(meshData.m_Vertices.size() * sizeof(T));
            uint32_t iSize = static_cast<uint32_t>(meshData.m_Indices.size() * sizeof(uint32_t));

            // Request the actual API implementation to move the bytes
            return CommitAllocation(
                meshData.m_Vertices.data(), vSize,
                meshData.m_Indices.data(), iSize, 
                static_cast<uint32_t>(meshData.m_Indices.size()),
                sizeof(T)
            );
        }

        virtual Ref<VertexBuffer>& GetVertexBuffer() = 0;
        virtual Ref<IndexBuffer>& GetIndexBuffer() = 0;

        static Scope<IGeometryPool> Create(GraphicsContext* ctxt, uint32_t vertexPoolSize, uint32_t indexPoolSize);

    protected:
        // Internal "Bridge" that the Vulkan/DX12 implementation must override
        virtual MeshAllocation CommitAllocation(
            const void* vData, uint32_t vByteSize,
            const void* iData, uint32_t iByteSize,
            uint32_t indexCount, uint32_t vertexStride
        ) = 0;
    };




}


#endif
