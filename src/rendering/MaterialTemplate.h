#pragma once
#ifndef MATERIAL_H
#define MATERIAL_H

#include <core/Core.h>
#include <core/UUID.h>
#include <assets/ISerialisable.h>
#include <math/Math.h>
#include "GraphicsCore.h"
#include <assets/AssetBase.h>
#include <assets/Serialisation/Archive.h>

namespace Axel
{
    class Pipeline;
    class GraphicsContext;
    
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
            ar.Property("Name", Name);
            ar.Property("Type", (int&)Type);
            ar.Property("Offset", (int)Offset);
            ar.Property("DisplayName", DisplayName);
            ar.EndStruct();
        }
    };

    struct MaterialTemplate : public ISerialisable, public IAsset {
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

        void BuildPipeline(GraphicsContext* context);

        const std::vector<PropertyDescriptor>& GetDescriptors() const { return m_Descriptors; }
        Ref<Pipeline> GetPipeline() const { return m_Pipeline; }

        virtual AssetTypeOptions GetType() const override { return AssetTypeOptions::MaterialTemplate; }
        
    private:
        Ref<Pipeline> m_Pipeline; // Hardware-specific object
        std::vector<PropertyDescriptor> m_Descriptors;
        virtual void Serialize(IArchive& ar) override;
    };

}


#endif