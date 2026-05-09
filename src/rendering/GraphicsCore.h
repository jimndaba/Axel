#pragma once
#ifndef GRAPHICSCORE_H
#define GRAPHICSCORE_H


namespace Axel
{

    struct Rect {
        float X, Y;         // Top-left corner in screen pixels (or NDC, pick one and commit)
        float Width, Height;
    };

    enum class MeshTypeOptions
    {
        Static,
        Skeletal,
        Vegetation
    };

    enum class PolygonModeOptions {
        Fill = 0,
        Line,   // Wireframe
        Point
    };

    enum class CullModeOptions {
        None = 0,
        Front,
        Back,
        FrontAndBack
    };

    enum class BlendingModeOptions
    {
        None = 0,
    };

    enum class TextureFilterOptions {
        Nearest = 0,
        Linear = 1
    };

    enum class TextureWrapOptions {
        Repeat = 0,
        MirroredRepeat = 1,
        ClampToEdge = 2,
        ClampToBorder = 3
    };


    enum class TextureUsageOptions : uint32_t
    {
        None = 0,     
        Texture = 1 << 1,
        RenderTarget = 1 << 2,
        DepthStencil = 1 << 3,
        Storage = 1 << 4,
        Sampled = 1 << 5
    };

    // This allows you to do: StageA | StageB
    inline Axel::TextureUsageOptions operator|(Axel::TextureUsageOptions a, Axel::TextureUsageOptions b) {
        return static_cast<Axel::TextureUsageOptions>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    // This allows you to do: StageA |= StageB
    inline bool operator&(TextureUsageOptions a, TextureUsageOptions b)
    {
        return static_cast<uint32_t>(a) & static_cast<uint32_t>(b);
    }
 

    enum class ShaderDataType {
        None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
    };

    // Helper to get byte size of types
    static uint32_t ShaderDataTypeSize(ShaderDataType type) {
        switch (type) {
        case ShaderDataType::Float:    return 4;
        case ShaderDataType::Float2:   return 4 * 2;
        case ShaderDataType::Float3:   return 4 * 3;
        case ShaderDataType::Float4:   return 4 * 4;
        case ShaderDataType::Mat3:     return 4 * 3 * 3;
        case ShaderDataType::Mat4:     return 4 * 4 * 4;
        case ShaderDataType::Int:      return 4;
        case ShaderDataType::Int2:     return 4 * 2;
        case ShaderDataType::Int3:     return 4 * 3;
        case ShaderDataType::Int4:     return 4 * 4;
        case ShaderDataType::Bool:     return 1;
        }
        return 0;
    }

    enum class ShaderResourceType {
        None = 0,
        UniformBuffer,
        StorageBuffer,
        ImageSampler,
        InputAttachment,
        CombinedImageSampler
    };

    enum ShaderStage : uint32_t {
        None = 0,
        Vertex = 1 << 0,
        Fragment = 1 << 1,
        Compute = 1 << 2,
        TessellationControl = 1<<3, 
        TessellationEvaluation = 1 <<4, 
        Geometry = 1 << 5,
        All = 0x7FFFFFFF
    };

    enum class DescriptorType {
        UniformBuffer,
        StorageBuffer,
        ImageSampler,
        StorageImage,
        None
    };

    // This allows you to do: StageA | StageB
    inline Axel::ShaderStage operator|(Axel::ShaderStage a, Axel::ShaderStage b) {
        return static_cast<Axel::ShaderStage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    // This allows you to do: StageA |= StageB
    inline Axel::ShaderStage& operator|=(Axel::ShaderStage& a, Axel::ShaderStage b) {
        a = a | b;
        return a;
    }

    enum class PropertyType {
        None,Float, Int, Vec2, Vec3, Vec4, Bool, Texture2D, TextureCube,Unknown
    };



    enum class TextureFormatOptions {
        None = 0,

        // Standard SDR
        R8,
        RG8,
        RGB8,
        RGBA8,
        RGBA_SRGB,

        // HDR Formats
        RG16F,            // High precision 2-channel (e.g., for Bloom/Motion vectors)
        RG32F,            // Maximum precision 2-channel
        RGB16F,           // Standard HDR color (16-bit float per channel)
        RGBA16F,          // Standard HDR with Alpha
        RGBA32F,          // Maximum precision HDR (usually overkill for final color)
        B10G11R11UF,      // Packed HDR: 10/10/11 bits. Great for performance/G-Buffers.

        // Depth/Stencil
        Depth24Stencil8,
        Depth32F,
        Depth32FStencil8
    };

    static uint32_t TextureFormatComponentCount(TextureFormatOptions type)
    {
        switch (type) {
        case TextureFormatOptions::R8:               return 1;
        case TextureFormatOptions::RG8:              return 2;
        case TextureFormatOptions::RG16F:            return 2;
        case TextureFormatOptions::RG32F:            return 2;
        case TextureFormatOptions::RGB8:             return 3;
        case TextureFormatOptions::RGB16F:            return 3;
        case TextureFormatOptions::B10G11R11UF:      return 3;
        case TextureFormatOptions::RGBA8:            return 4;
        case TextureFormatOptions::RGBA16F:          return 4;
        case TextureFormatOptions::RGBA32F:          return 4;
        case TextureFormatOptions::RGBA_SRGB:        return 4;

            // Depth/Stencil are technically single-channel in shader space
        case TextureFormatOptions::Depth24Stencil8:  return 1;
        case TextureFormatOptions::Depth32F:         return 1;
        case TextureFormatOptions::Depth32FStencil8: return 1;

        case TextureFormatOptions::None:
        default:                                     return 0;
        }
    }
}
#endif  