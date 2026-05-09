#pragma once
#ifndef MATERIAL_H
#define MATERIAL_H

#include <core/Core.h>
#include <core/UUID.h>
#include <assets/ISerialisable.h>
#include <core/Reflection.h>
#include <math/Math.h>
#include "GraphicsCore.h"
#include <assets/AssetBase.h>
#include <assets/Serialisation/Archive.h>
#include <any>


namespace Axel
{
    class Pipeline;
    class GraphicsContext;
    class MaterialInstance;
    class RenderPass;
    
    struct PropertyDescriptor : public ISerialisable {
        std::string Name;        // e.g., "u_AlbedoColour"
        PropertyType Type;      // e.g., PropertyType::Vec4
        uint32_t Size;          // Size in bytes
        uint32_t Offset;        // Offset within the Material SSBO slot

        // Metadata for the Editor
        std::string DisplayName; // e.g., "Albedo Tint"
        float Min = 0.0f;
        float Max = 1.0f;

        virtual void Serialize(IArchive& ar) override {
            ar.BeginStruct("PropertyDesciptor");
            VisitProperty("Name", Name,ar);
            VisitProperty("Type", (int&)Type,ar);
            VisitProperty("Offset", (int&)Offset,ar);
            VisitProperty("DisplayName", DisplayName,ar);
            ar.EndStruct();
        }

        virtual const char* GetName() { return Name.c_str(); }
    };

    struct MaterialTemplate : public ISerialisable, public IAsset {

        MaterialTemplate(UUID shader);

        // 1. Identification
        UUID ID;
        std::string Name;

        // 2. The Assets
        UUID ShaderID; // Points to your .glsl or .spv asset

        // 3. Fixed-Function Pipeline State (The "How")
        CullModeOptions CullMode = CullModeOptions::Back;
        BlendingModeOptions BlendMode = BlendingModeOptions::None;
        bool DepthTest = true;
        bool DepthWrite = true;

        void BuildPipeline(GraphicsContext* context,const Ref<RenderPass>& targetPass = nullptr);

        const std::vector<PropertyDescriptor>& GetDescriptors() const { return m_Descriptors; }
        Ref<Pipeline> GetPipeline() const { return m_Pipeline; }

        void AddProperty(PropertyDescriptor type);
        virtual AssetTypeOptions GetType() const override { return AssetTypeOptions::MaterialTemplate; }
        virtual void Serialize(IArchive& ar) override;

        // Inside MaterialTemplate.h/cpp
        uint32_t CalculateBufferSize();

        Ref<MaterialInstance> CreateInstance(const std::string& instanceName);

        virtual const char* GetName() { return Name.c_str(); }
    private:
        Ref<Pipeline> m_Pipeline; // Hardware-specific object
        std::vector<PropertyDescriptor> m_Descriptors;       
    };

}


#endif