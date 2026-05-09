#pragma once
#ifndef VULKANUTILS_H
#define VULKANUTILS_H


#include <vulkan/vulkan.h>
#include <rendering/GraphicsCore.h>
#include <spirv_refl/spirv_reflect.h>
#include <core/Logger.h>


namespace Axel
{
    static VkFilter AxelFilterToVulkan(Axel::TextureFilterOptions filter) {
        return filter == Axel::TextureFilterOptions::Nearest ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;
    }

    static VkSamplerAddressMode AxelWrapToVulkan(Axel::TextureWrapOptions wrap) {
        switch (wrap) {
        case Axel::TextureWrapOptions::Repeat:         return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case Axel::TextureWrapOptions::MirroredRepeat: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case Axel::TextureWrapOptions::ClampToEdge:    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case Axel::TextureWrapOptions::ClampToBorder:  return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        default:                                 return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        }
    }

    static  bool IsDepthFormat(Axel::TextureFormatOptions format)
    {
        switch (format)
        {
        case Axel::TextureFormatOptions::Depth24Stencil8:
        case Axel::TextureFormatOptions::Depth32F:
            return true;

        default:
            return false;
        }
    }

    static  bool IsVkDepthFormat(VkFormat format)
    {
        switch (format)
        {
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_X8_D24_UNORM_PACK32:
        case VK_FORMAT_D32_SFLOAT:
        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return true;

        default:
            return false;
        }
    }
   
    static  bool HasVKStencil(VkFormat format)
    {
        switch (format)
        {
        case VK_FORMAT_S8_UINT:
        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return true;

        default:
            return false;
        }
    }

    static VkImageUsageFlags AxelUsageToVulkan(Axel::TextureFormatOptions format,
        Axel::TextureUsageOptions usage)
    {
        VkImageUsageFlags flags = 0;

        // Always allow transfers (common and useful)
        flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        // --- Attachment usage ---
        if (usage & Axel::TextureUsageOptions::RenderTarget)
        {
            if (IsDepthFormat(format))
                flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            else
                flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }

        if (usage & Axel::TextureUsageOptions::DepthStencil)
        {
            flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }

        // --- Storage ---
        if (usage & Axel::TextureUsageOptions::Storage)
        {
            flags |= VK_IMAGE_USAGE_STORAGE_BIT;
        }

        // --- Sampling ---
        // Treat "Texture" as implicitly sampled
        if ((usage & Axel::TextureUsageOptions::Sampled) ||
            (usage & Axel::TextureUsageOptions::Texture))
        {
            flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
        }

        return flags;
    }

    static VkFormat AxelTextureFormatToVulkan(Axel::TextureFormatOptions format)
    {
        switch (format)
        {
        case Axel::TextureFormatOptions::R8:              return VK_FORMAT_R8_UNORM;
        case Axel::TextureFormatOptions::RG8:             return VK_FORMAT_R8G8_UNORM;
        case Axel::TextureFormatOptions::RGB8:            return VK_FORMAT_R8G8B8_UNORM;
        case Axel::TextureFormatOptions::RGBA8:           return VK_FORMAT_R8G8B8A8_UNORM;
        case Axel::TextureFormatOptions::RGBA_SRGB:       return VK_FORMAT_B8G8R8A8_SRGB;

            // HDR Formats
        case Axel::TextureFormatOptions::RG16F:           return VK_FORMAT_R16G16_SFLOAT;
        case Axel::TextureFormatOptions::RG32F:           return VK_FORMAT_R32G32_SFLOAT;
        case Axel::TextureFormatOptions::RGB16F:          return VK_FORMAT_R16G16B16_SFLOAT;
        case Axel::TextureFormatOptions::RGBA16F:         return VK_FORMAT_R16G16B16A16_SFLOAT;
        case Axel::TextureFormatOptions::RGBA32F:         return VK_FORMAT_R32G32B32A32_SFLOAT;
        case Axel::TextureFormatOptions::B10G11R11UF:     return VK_FORMAT_B10G11R11_UFLOAT_PACK32;

            // Depth/Stencil Formats
        case Axel::TextureFormatOptions::Depth24Stencil8: return VK_FORMAT_D24_UNORM_S8_UINT;
        case Axel::TextureFormatOptions::Depth32F:        return VK_FORMAT_D32_SFLOAT;
        case Axel::TextureFormatOptions::Depth32FStencil8: return VK_FORMAT_D32_SFLOAT_S8_UINT;

        case Axel::TextureFormatOptions::None:
        default:
            AXLOG_ERROR("Unknown or unsupported TextureFormatOptions!");
            return VK_FORMAT_UNDEFINED;
        }
    }

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

    static VkDescriptorType AxelDescriptorTypeToVulkan(DescriptorType type)
    {
        switch (type)
        {
        case DescriptorType::UniformBuffer:       return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case DescriptorType::StorageBuffer:       return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case DescriptorType::ImageSampler:       return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        case DescriptorType::StorageImage:        return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        }

        AXEL_CORE_ASSERT(false, "Unknown DescriptorType!");
        return VK_DESCRIPTOR_TYPE_MAX_ENUM;
    }

}

#endif