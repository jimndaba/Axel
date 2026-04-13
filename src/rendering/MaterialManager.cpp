#include "axelpch.h"
#include "MaterialManager.h"
#include <assets/AssetManager.h>
#include "MaterialInstance.h"

Axel::MaterialManager::MaterialManager(GraphicsContext* ctxt):
    m_Context(ctxt)
{
}

void Axel::MaterialManager::RegisterMaterial(Ref<MaterialInstance> instance)
{
    UUID id = instance->AssetID;

    // 1. Prevent double-registration
    if (m_MaterialRegistry.find(id) != m_MaterialRegistry.end())
        return;

    // 2. Fetch the template to know the byte size requirement
    auto mt = AssetManager::GetAsset<MaterialTemplate>(instance->GetTemplateID());
    if (!mt) {
        AXLOG_ERROR("Failed to register material: Template not found!");
        return;
    }

    // 3. Store metadata
    MaterialMetadata meta;
    meta.Size = mt->CalculateBufferSize();

    // Calculate offset based on current end of buffer (with alignment)
    // std430 alignment usually requires 16-byte boundaries for structs
    uint32_t alignment = 16;
    uint32_t currentTotalSize = 0;
    for (const auto& [regId, regMeta] : m_MaterialRegistry) {
        currentTotalSize += regMeta.Size;
        // Pad to alignment if necessary
        if (currentTotalSize % alignment != 0)
            currentTotalSize += (alignment - (currentTotalSize % alignment));
    }

    meta.Offset = currentTotalSize;

    // 4. Update Registry and Tracking list
    m_MaterialRegistry[id] = meta;
    m_ActiveInstances.push_back(instance);

    m_NeedsReallocation = true; // Signal that the GPU SSBO needs to grow
    m_IsDirty = true;
}

void Axel::MaterialManager::Update()
{
    // If nothing changed, don't waste time re-packing or re-uploading
    if (!m_IsDirty && !m_NeedsReallocation) return;

    // 1. Calculate total size (using a fixed stride for simple indexing)
    uint32_t totalSize = 0;
    for (auto& instance : m_ActiveInstances) {
        auto mt = AssetManager::GetAsset<MaterialTemplate>(instance->GetTemplateID());
        totalSize += mt->CalculateBufferSize();
    }

    // 2. Buffer Management
    if (m_CPUBuffer.size() < totalSize || !m_MaterialTableBuffer) {
        m_CPUBuffer.resize(totalSize);
        // Create SSBO at Set 1, Binding 0
        m_MaterialTableBuffer = ShaderStorageBuffer::Create(m_Context, totalSize, 0);
    }

    // 3. Packing & Indexing
    uint32_t currentOffset = 0;
    for (uint32_t i = 0; i < m_ActiveInstances.size(); ++i) {
        auto& instance = m_ActiveInstances[i];
        auto mt = AssetManager::GetAsset<MaterialTemplate>(instance->GetTemplateID());
        uint32_t size = mt->CalculateBufferSize();

        // Pack the variant data into the staging buffer
        uint32_t alignedSize = (size + 15) & ~15; // Round up to 16
        instance->PackData(m_CPUBuffer.data() + currentOffset);       
        currentOffset += alignedSize;
        // Map the UUID to the specific offset and size
        // This 'i' becomes your rc.u_MaterialIndex!
        m_MaterialRegistry[instance->AssetID] = { currentOffset, size,i };
    }

    // 4. Synchronization
    m_MaterialTableBuffer->SetData(m_CPUBuffer.data(), totalSize);

    m_IsDirty = false;
    m_NeedsReallocation = false;
}

uint32_t Axel::MaterialManager::GetMaterialIndex(UUID id) {
    auto it = m_MaterialRegistry.find(id);
    if (it != m_MaterialRegistry.end())
    {
        return it->second.Index; // No division, no guesswork
    }
    return 0;
}
