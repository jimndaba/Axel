#include "axelpch.h"
#include "MaterialTemplate.h"
#include <assets/AssetManager.h>
#include "GraphicsCore.h"
#include "Pipeline.h"
#include "backends/GraphicsContext.h"
#include "Shader.h"

Axel::MaterialTemplate::MaterialTemplate(UUID shader):
    ShaderID(shader)
{
}

void Axel::MaterialTemplate::BuildPipeline(GraphicsContext* context)
{
    // 1. Get Shader and Reflect
    auto shader = AssetManager::GetAsset<Shader>(ShaderID);
    auto allresources = shader->GetResources();

    const uint32_t MaterialSetIndex = 1;

    m_Descriptors.clear();

    if (allresources.find(MaterialSetIndex) != allresources.end())
    {
        auto& bindings = allresources.at(MaterialSetIndex);

        for (auto& [bindingIndex, resource] : bindings)
        {
            // We only want the UBO that holds scalar/vector properties.
            // The texture sampler at binding 1 is handled separately.
            if (resource.Type != ShaderResourceType::StorageBuffer)
                continue;

            // Accept the MaterialData block regardless of exact name
            for (auto& member : resource.Members)
            {
                PropertyDescriptor desc;
                desc.Name = member.Name;
                desc.Type = member.Type;
                desc.Offset = member.Offset;
                desc.Size = member.Size;
                m_Descriptors.push_back(desc);
            }
        }
    }
  
    // 2. Define the Hardware State
    PipelineSpecification spec;
    spec.Shader = shader;
    spec.FaceCulling = CullMode;
    spec.BlendMode = BlendMode;
    spec.DepthTest = DepthTest;
    spec.DepthWrite = DepthWrite;
    spec.TargetRenderPass = context->GetMainRenderPass();
    spec.Layout = shader->m_VertexLayout;

    // 3. Bake the Hardware Truth
    m_Pipeline = Pipeline::Create(context,spec);

}

void Axel::MaterialTemplate::Serialize(IArchive& ar)
{
    ar.BeginStruct("MaterialTemplate");
    ar.Property("ID", ID);
    ar.Property("ShaderID", ShaderID);
    ar.Property("CullMode", (int)CullMode);
    ar.Property("BlendMode", (int)BlendMode);
    ar.Property("DepthTest", DepthTest);
    ar.Property("DepthWrite", DepthWrite);

    uint32_t size = m_Descriptors.size();
    ar.BeginCollection("PropertyDescriptors", size);
    for (auto& prop : m_Descriptors)
    {
        prop.Serialize(ar);
    }
    ar.EndCollection();

    ar.EndStruct();
}

uint32_t Axel::MaterialTemplate::CalculateBufferSize()
{
    if (m_Descriptors.empty()) return 0;

    // 1. Find the end of the last element
    const auto& lastDesc = m_Descriptors.back();
    uint32_t rawSize = lastDesc.Offset + lastDesc.Size;

    // 2. std430 Alignment: Round up to the nearest 16 bytes
    // This ensures that an array of these structs in an SSBO stays aligned.
    uint32_t alignment = 16;
    uint32_t paddedSize = (rawSize + (alignment - 1)) & ~(alignment - 1);

    return paddedSize;
}

