#pragma once
#ifndef SCENEPACKET_H
#define SCENEPACKET_H

#include "core/Core.h"
#include "math/Math.h"
#include <vector>
#include <core/UUID.h>
#include "GraphicsCore.h"
#include <variant>



namespace Axel
{

    class Material;

    enum class RenderPacketTypeOptions {
        Mesh,
        SkinnedMesh,
        Billboard,
        UI,
        Particle,
        DebugLine
    };
    

    // --- 1. MESH PACKET (3D Environment & Static Props) ---
    struct MeshRenderPacket {
        Mat4 Transform;
        UUID MeshHandle;
        uint32_t SubmeshIndex;
        uint32_t MaterialIndex; // Index into your MaterialManager SSBO
    };

    // --- 2. SKINNED MESH PACKET (Characters & NPCs) ---
    struct SkinnedMeshPacket {
        Mat4 Transform;
        UUID MeshHandle;
        uint32_t SubmeshIndex;
        uint32_t MaterialIndex;
        uint32_t BoneOffset;    // Offset into the Global Bone SSBO
    };

    // --- 3. BILLBOARD/SPRITE PACKET (Trees, Pickups, GBA-style FX) ---
    struct BillboardRenderPacket {
        Vec3 Position;
        Vec2 Size;
        float Rotation;
        Vec4 ColorTint;
        uint32_t TextureHandle;
        uint32_t MaterialIndex;
        uint32_t Flags;         // e.g., Lock Y-Axis, Screen-Space, etc.
    };

    // --- 4. UI PACKET (Health Bars, Menus) ---
    struct UIRenderPacket {
        Rect ScreenRect;
        Rect UVRect;
        Vec4 Color;
        uint32_t TextureHandle;
        uint32_t Layer;         // For Z-sorting the UI
    };

    struct DebugLinePacket {
        Vec3  Start;
        Vec3  End;
        Vec4  Color;
        float Duration;     // 0.0f = one frame only, >0 persists
        bool  DepthTest;    // false = always on top (useful for UI debug)
    };

    // --- 6. PARTICLE PACKET (per-particle, uploaded to GPU SSBO) ---
    struct ParticlePacket {
        Vec3     Position;
        Vec2     Size;
        float    Rotation;
        Vec4     Color;
        uint32_t AtlasIndex;    // Which sprite in the texture atlas
        UUID EmitterID;     // Groups particles for batch sorting
    };

    using RenderPacketData = std::variant<
        MeshRenderPacket,
        SkinnedMeshPacket,
        BillboardRenderPacket,
        UIRenderPacket,
        DebugLinePacket,
        ParticlePacket
    >;

    struct RenderPacket
    {
        RenderPacketTypeOptions Type{};
        RenderPacketData Data{};
        uint64_t SortKey = 0;

        RenderPacket() = default;

        RenderPacket(const RenderPacket&) = default;
        RenderPacket(RenderPacket&&) noexcept = default;

        RenderPacket& operator=(const RenderPacket&) = default;
        RenderPacket& operator=(RenderPacket&&) noexcept = default;

        ~RenderPacket() = default;

        template<typename T>
        RenderPacket(RenderPacketTypeOptions type, T&& data, uint64_t sortKey)
            : Type(type),
            Data(std::forward<T>(data)),
            SortKey(sortKey)
        {
        }

        static uint64_t MakeSortKey(
            uint16_t materialTemplateID,
            uint16_t materialIndex,
            float depth,
            uint16_t drawOrder = 0)
        {
            uint16_t quantisedDepth =
                static_cast<uint16_t>(
                    std::clamp(depth, 0.0f, 1.0f) * 65535.0f
                    );

            return
                (static_cast<uint64_t>(materialTemplateID) << 48) |
                (static_cast<uint64_t>(materialIndex) << 32) |
                (static_cast<uint64_t>(quantisedDepth) << 16) |
                static_cast<uint64_t>(drawOrder);
        }

        template<typename T>
        bool Is() const
        {
            return std::holds_alternative<T>(Data);
        }

        template<typename T>
        T& Get()
        {
            return std::get<T>(Data);
        }

        template<typename T>
        const T& Get() const
        {
            return std::get<T>(Data);
        }
    };
}


#endif
