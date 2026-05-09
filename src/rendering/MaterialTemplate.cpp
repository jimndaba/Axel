#include "axelpch.h"
#include "MaterialTemplate.h"
#include <assets/AssetManager.h>
#include "GraphicsCore.h"
#include "Pipeline.h"
#include "GraphicsContext.h"
#include "Shader.h"
#include <rendering/MaterialInstance.h>

Axel::MaterialTemplate::MaterialTemplate(UUID shader):
    ShaderID(shader)
{
}

void Axel::MaterialTemplate::BuildPipeline(GraphicsContext* context, const Ref<RenderPass>& targetPass)
{
    // 1. Get Shader and its reflected resources
    auto shader = AssetManager::GetAsset<Shader>(ShaderID);
    const auto& allresources = shader->GetResources();

    // In your architecture, Set 1 is reserved for Material-specific data
    const uint32_t MaterialSetIndex = 1;

    m_Descriptors.clear();

    // 2. Check if the Material Set exists in this shader
    if (allresources.contains(MaterialSetIndex))
    {
        const auto& bindings = allresources.at(MaterialSetIndex);

        for (auto& [bindingIndex, resource] : bindings)
        {
            // We are looking for the Uniform Buffer that contains our material properties
            // (Note: Some shaders might use UniformBuffer, others StorageBuffer for materials)
            if (resource.Type != DescriptorType::UniformBuffer &&
                resource.Type != DescriptorType::StorageBuffer)
                continue;

            // 3. Iterate through the members of the buffer block
            // 'resource' is a DescriptorBinding which contains a 'Members' vector
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
    spec.TargetRenderPass = targetPass;
    spec.Layout = shader->m_VertexLayout;

    // 3. Bake the Hardware Truth
    m_Pipeline = Pipeline::Create(context,spec);

}

void Axel::MaterialTemplate::Serialize(IArchive& ar)
{
    ar.BeginStruct("MaterialTemplate");
    VisitProperty("ID", ID,ar);
    VisitProperty("ShaderID", ShaderID, ar);
    VisitProperty("CullMode", (int&)CullMode, ar);
    VisitProperty("BlendMode", (int&)BlendMode, ar);
    VisitProperty("DepthTest", DepthTest, ar);
    VisitProperty("DepthWrite", DepthWrite, ar);

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

std::shared_ptr<Axel::MaterialInstance> Axel::MaterialTemplate::CreateInstance(const std::string& instanceName)
{
    // 1. Instantiate the object
    auto instance = std::make_shared<MaterialInstance>(AssetID);

    // 2. Optional: Initialize with Template Defaults
    // This ensures that if a glTF doesn't provide a value, we don't have zeros
    for (const auto& desc : m_Descriptors) {
        // You could extend PropertyDescriptor to have a 'DefaultValue' field
       //instance->Set(desc.Name, desc.);
    }

    return instance;
}

void Axel::MaterialTemplate::AddProperty(PropertyDescriptor type)
{
    m_Descriptors.push_back(type);
}