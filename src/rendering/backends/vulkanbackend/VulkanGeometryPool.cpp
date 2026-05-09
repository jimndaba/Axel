#include "axelpch.h"
#include "VulkanGeometryPool.h"
#include "VulkanDevice.h"
#include "VulkanContext.h"
#include "VulkanBuffer.h"
#include <core/Logger.h>


Axel::VulkanGeometryPool::VulkanGeometryPool(GraphicsContext* context,uint32_t vertexPoolSize, uint32_t indexPoolSize):
    m_MaxVertexBytes(vertexPoolSize),m_MaxIndexBytes(indexPoolSize),mContext(context)
{

    m_GlobalVertexBuffer = context->GetDevice()->CreateVertexBuffer(nullptr, vertexPoolSize);
    m_GlobalIndexBuffer = context->GetDevice()->CreateIndexBuffer(nullptr, (indexPoolSize/sizeof(uint32_t)));

}

Axel::VulkanGeometryPool::~VulkanGeometryPool()
{
}

Axel::MeshAllocation Axel::VulkanGeometryPool::CommitAllocation(const void* vData, uint32_t vByteSize, const void* iData, uint32_t iByteSize, uint32_t indexCount, uint32_t vertexStride)
{
    // 1. Safety check for buffer overflow
    if ((m_CurrentVertexOffset * vertexStride) + vByteSize > m_MaxVertexBytes ||
        (m_CurrentIndexOffset * sizeof(uint32_t)) + iByteSize > m_MaxIndexBytes)
    {
        AXLOG_ERROR("Geometry Pool: Out of memory during allocation!");
        return MeshAllocation{ 0, 0, 0 }; // Return null allocation
    }

    // 2. Capture the current offsets for the return token
    MeshAllocation allocation;
    allocation.VertexOffset = m_CurrentVertexOffset;
    allocation.IndexOffset = m_CurrentIndexOffset;
    allocation.IndexCount = indexCount;

    // 3. Convert current offsets (indices) to byte offsets for the API
    uint32_t vertexByteOffset = m_CurrentVertexOffset * vertexStride;
    uint32_t indexByteOffset = m_CurrentIndexOffset * sizeof(uint32_t);

    // 4. Push the raw data to the GPU
    // True for Vertex Buffer, False for Index Buffer
    PushToGPU(vData, vByteSize, vertexByteOffset, true);
    PushToGPU(iData, iByteSize, indexByteOffset, false);

    // 5. Increment the pool "Write Heads" 
    // We increment by the count of elements, not bytes
    m_CurrentVertexOffset += (vByteSize / vertexStride);
    m_CurrentIndexOffset += (iByteSize / sizeof(uint32_t));

    return allocation;
}

void Axel::VulkanGeometryPool::PushToGPU(const void* data, uint32_t size, uint32_t offset, bool isVertexBuffer)
{

    const auto& vDevice = std::static_pointer_cast<VulkanDevice>(mContext->GetDevice());
    auto staging = vDevice->AcquireStagingBuffer(size);

    void* mappedMemory;
    vkMapMemory(vDevice->GetLogicalDevice(), staging.Memory, 0, size, 0, &mappedMemory);
    memcpy(mappedMemory, data, size);
    vkUnmapMemory(vDevice->GetLogicalDevice(), staging.Memory);

    VkBuffer dstHandle = VK_NULL_HANDLE;
    if (isVertexBuffer)
        dstHandle = std::static_pointer_cast<VkVertexBuffer>(m_GlobalVertexBuffer)->GetBuffer()->GetHandle();
    else
        dstHandle = std::static_pointer_cast<VkIndexBuffer>(m_GlobalIndexBuffer)->GetBuffer()->GetHandle();

    // The 'Forklift' move: Staging -> Device Local Pool
    vDevice->CopyBuffer(staging.Handle, dstHandle, static_cast<VkDeviceSize>(size), static_cast<VkDeviceSize>(offset));

    vDevice->ReleaseStagingBuffer(staging);
}
