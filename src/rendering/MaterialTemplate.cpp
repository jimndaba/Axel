#include "axelpch.h"
#include "MaterialTemplate.h"
#include <assets/AssetManager.h>
#include "GraphicsCore.h"
#include "Pipeline.h"
#include "backends/GraphicsContext.h"
#include "Shader.h"

void Axel::MaterialTemplate::BuildPipeline(GraphicsContext* context)
{
    // 1. Get Shader and Reflect
    auto shader = AssetManager::GetAsset<Shader>(ShaderID);
    auto allresources = shader->GetResources();

    const uint32_t MaterialSetIndex = 2;

    if (allresources.find(MaterialSetIndex) != allresources.end()) {
        auto& bindings = allresources.at(MaterialSetIndex);

        for (auto& [binding, resource] : bindings) {
            // We look for our "Uber-Buffer" (usually at binding 0)
            if (resource.Name == "MaterialData" || resource.Name == "u_MaterialTable") {
                m_Descriptors.clear();

                for (auto& member : resource.Members) {
                    PropertyDescriptor desc;
                    desc.Name = member.Name;
                    desc.Type = member.Type;
                    desc.Offset = member.Offset;
                    desc.Size = member.Size;
                    m_Descriptors.push_back(desc);
                }
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
