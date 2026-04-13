#pragma once
#ifndef UTILS_H
#define UTILS_H	

#include <rendering/GraphicsCore.h>
#include <vulkan/vulkan.h>
#include <spirv_refl/spirv_reflect.h>

namespace Axel
{
    static VkPolygonMode AxelPolygonModeToVulkan(PolygonModeOptions mode) {
        switch (mode) {
        case PolygonModeOptions::Fill:  return VK_POLYGON_MODE_FILL;
        case PolygonModeOptions::Line:  return VK_POLYGON_MODE_LINE;
        case PolygonModeOptions::Point: return VK_POLYGON_MODE_POINT;
        }
        return VK_POLYGON_MODE_FILL;
    }

    static VkCullModeFlags AxelCullModeToVulkan(CullModeOptions mode) {
        switch (mode) {
        case CullModeOptions::None:         return VK_CULL_MODE_NONE;
        case CullModeOptions::Front:        return VK_CULL_MODE_FRONT_BIT;
        case CullModeOptions::Back:         return VK_CULL_MODE_BACK_BIT;
        case CullModeOptions::FrontAndBack: return VK_CULL_MODE_FRONT_AND_BACK;
        }
        return VK_CULL_MODE_BACK_BIT;
    }

    static VkFormat AxelShaderDataTypeToVulkanFormat(ShaderDataType type) {
        switch (type) {
        case ShaderDataType::Float:  return VK_FORMAT_R32_SFLOAT;
        case ShaderDataType::Float2: return VK_FORMAT_R32G32_SFLOAT;
        case ShaderDataType::Float3: return VK_FORMAT_R32G32B32_SFLOAT;
        case ShaderDataType::Float4: return VK_FORMAT_R32G32B32A32_SFLOAT;
        case ShaderDataType::Int:    return VK_FORMAT_R32_SINT;
            // ... add more as needed
        }
        return VK_FORMAT_UNDEFINED;
    }

    static VkShaderStageFlagBits AxelStageToVulkan(ShaderStage stage) {
        switch (stage) {
        case ShaderStage::Vertex:                  return VK_SHADER_STAGE_VERTEX_BIT;
        case ShaderStage::Fragment:                return VK_SHADER_STAGE_FRAGMENT_BIT;
        case ShaderStage::Compute:                 return VK_SHADER_STAGE_COMPUTE_BIT;
        case ShaderStage::TessellationControl:    return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        case ShaderStage::TessellationEvaluation: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        case ShaderStage::Geometry:                return VK_SHADER_STAGE_GEOMETRY_BIT;
        }
        return VK_SHADER_STAGE_ALL_GRAPHICS;
    }

    static VkDescriptorType AxelToVulkanType(ShaderResourceType type) {
        switch (type) {
        case ShaderResourceType::UniformBuffer:         return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case ShaderResourceType::StorageBuffer:         return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case ShaderResourceType::ImageSampler:          return VK_DESCRIPTOR_TYPE_SAMPLER;
        case ShaderResourceType::CombinedImageSampler:  return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            // Add more as you expand (e.g., InputAttachments for deferred rendering      
        }
        return VK_DESCRIPTOR_TYPE_MAX_ENUM;
    }

    static ShaderStage SpvToAxelStage(SpvReflectShaderStageFlagBits stage) {
        switch (stage) {
        case SPV_REFLECT_SHADER_STAGE_VERTEX_BIT:   return  ShaderStage::Vertex;
        case SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT: return  ShaderStage::Fragment;
        case SPV_REFLECT_SHADER_STAGE_COMPUTE_BIT:  return  ShaderStage::Compute;
        case SPV_REFLECT_SHADER_STAGE_GEOMETRY_BIT: return  ShaderStage::Geometry;
            // Add others if your engine supports Tesselation, etc.
        default: return  ShaderStage::None;
        }
    }

}
#endif