#pragma once
#ifndef DESCRSETLAYOUT_H
#define DESCRSETLAYOUT_H

#include <core/Core.h>
#include <rendering/GraphicsCore.h>

namespace Axel
{
    class GraphicsContext;
    // These enums should map to your Vulkan/DX12 equivalents

    struct ShaderMember {
        std::string Name;
        uint32_t Offset;
        uint32_t Size;
        PropertyType Type; // e.g., Float, Vec3, Bool
    };

    struct DescriptorBinding {
        std::string Name;
        uint32_t Binding;
        uint32_t Size;
        DescriptorType Type;
        uint32_t Count; // For arrays/bindless
        ShaderStage Stage;
        std::vector<ShaderMember> Members;
    };

    using ShaderResource = DescriptorBinding;

    class DescriptorSetLayout {
    public:
        virtual ~DescriptorSetLayout() = default;

        static Ref<DescriptorSetLayout> Create(GraphicsContext* ctxt, const std::initializer_list<DescriptorBinding>& bindings);
        static Ref<DescriptorSetLayout> Create(GraphicsContext* ctxt, const std::vector<DescriptorBinding>& bindings);

        // This allows the Vulkan backend to extract the VkDescriptorSetLayout
        virtual void* GetNativeLayout() const = 0;
    };

}

#endif