#include "axelpch.h"
#include "MaterialManager.h"
#include <assets/AssetManager.h>
#include "MaterialInstance.h"
#include "DescriptorSet.h"
#include "DescriptorSetLayout.h"
#include "Renderer.h"

Axel::MaterialManager::MaterialManager(GraphicsContext* ctxt):
    m_Context(ctxt)
{
   
    {
        std::vector<DescriptorBinding> bindings;
        DescriptorBinding n;
        n.Binding = 0;
        n.Count = 1;
        n.Name = "u_MaterialPool";
        n.Type = DescriptorType::StorageBuffer;
        n.Stage = ShaderStage::Fragment | ShaderStage::Vertex;
        bindings.push_back(n);
        m_MaterialTableLayout = DescriptorSetLayout::Create(m_Context, bindings);
    }
}

void Axel::MaterialManager::RegisterMaterial(Ref<MaterialInstance> instance)
{
    UUID id = instance->AssetID;

    if (m_MaterialRegistry.find(id) != m_MaterialRegistry.end())
        return;

    auto mt = AssetManager::GetAsset<MaterialTemplate>(instance->GetTemplateID());
    if (!mt) {
        AXLOG_ERROR("Failed to register material: Template not found! {} ", instance->GetTemplateID().ToString());
        return;
    }

    MaterialMetadata meta;
    meta.Size = mt->CalculateBufferSize();

    uint32_t alignment = 16;
    uint32_t currentTotalSize = 0;
    for (const auto& [regId, regMeta] : m_MaterialRegistry) {
        currentTotalSize += (regMeta.Size + (alignment - 1)) & ~(alignment - 1);
    }

    meta.Offset = currentTotalSize;
    m_MaterialRegistry[id] = meta;
    m_ActiveInstances.push_back(instance);

    m_NeedsReallocation = true;
    m_IsDirty = true;
}

void Axel::MaterialManager::Update()
{
    if (!m_IsDirty && !m_NeedsReallocation) return;

    // 1. Calculate total size
    uint32_t totalSize = 0;
    for (auto& instance : m_ActiveInstances) {
        auto mt = AssetManager::GetAsset<MaterialTemplate>(instance->GetTemplateID());
        uint32_t size = mt->CalculateBufferSize();
        totalSize += (size + 15) & ~15;
    }

    // 2. Buffer Management
    bool bufferResized = false;
    if (m_CPUBuffer.size() < totalSize || !m_MaterialTableBuffer) {
        m_CPUBuffer.resize(totalSize);
        m_MaterialTableBuffer = ShaderStorageBuffer::Create(m_Context, totalSize, 0);
        bufferResized = true;
    }

    // 3. Packing & Indexing
    uint32_t currentOffset = 0;
    for (uint32_t i = 0; i < m_ActiveInstances.size(); ++i) {
        auto& instance = m_ActiveInstances[i];
        auto mt = AssetManager::GetAsset<MaterialTemplate>(instance->GetTemplateID());
        uint32_t size = mt->CalculateBufferSize();
        uint32_t alignedSize = (size + 15) & ~15;

        instance->PackData(m_CPUBuffer.data() + currentOffset);

        m_MaterialRegistry[instance->AssetID] = { currentOffset, size, i };
        currentOffset += alignedSize;
    }
        
    // Synchronize Material Data (Set 1)
    if (bufferResized || !m_MaterialDescriptorSet) {

        m_MaterialDescriptorSet = DescriptorSet::Create(m_Context, m_MaterialTableLayout);
        m_MaterialDescriptorSet->Write("u_MaterialPool", m_MaterialTableBuffer);
        m_MaterialDescriptorSet->Update();
        
    }

    // 5. GPU Upload
    m_MaterialTableBuffer->SetData(m_CPUBuffer.data(), totalSize);

    m_IsDirty = false;
    m_NeedsReallocation = false;
}

uint32_t Axel::MaterialManager::GetMaterialIndex(UUID id) {
    auto it = m_MaterialRegistry.find(id);
    if (it != m_MaterialRegistry.end())
        return it->second.Index;
    return 0;
}

Axel::UUID Axel::MaterialManager::GetMaterialTemplateID(uint32_t materialIndex)
{
    if (materialIndex < m_ActiveInstances.size()) {
        auto& instance = m_ActiveInstances[materialIndex];
        if (instance) return instance->GetTemplateID();
    }
    AXLOG_WARN("MaterialManager: Invalid material index {0}", materialIndex);
    return UUID(0);
}