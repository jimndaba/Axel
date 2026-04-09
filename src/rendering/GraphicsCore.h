#pragma once
#ifndef GRAPHICSCORE_H
#define GRAPHICSCORE_H


namespace Axel
{
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

    enum class ShaderDataType {
        None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
    };

    enum class ShaderResourceType {
        None = 0,
        UniformBuffer,
        StorageBuffer,
        ImageSampler,
        InputAttachment,
        CombinedImageSampler
    };

    enum ShaderStage {
        None = 0,
        Vertex = 1 << 0,
        Fragment = 1 << 1,
        Compute = 1 << 2,
        TessellationControl = 1<<3, 
        TessellationEvaluation = 1 <<4, 
        Geometry = 1 << 5,
        All = 0x7FFFFFFF
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
        Float, Int, Vec2, Vec3, Vec4, Bool, Texture2D, TextureCube,Unknown
    };

    struct ShaderMember {
        std::string Name;
        PropertyType Type;
        uint32_t Size;
        uint32_t Offset;
    };

    struct ShaderResource {
        std::string Name;
        ShaderResourceType Type ;
        ShaderStage Stage = ShaderStage::None;
        uint32_t Binding = 0;
        uint32_t Set = 0;    // Crucial for Vulkan/DX12 frequency levels
        uint32_t Count = 0;  // For arrays of textures/buffers
        uint32_t Size = 0;  // For arrays of textures/buffers
        std::vector<ShaderMember> Members;
    };
}
#endif  