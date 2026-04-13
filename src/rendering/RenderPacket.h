#pragma once
#ifndef SCENEPACKET_H
#define SCENEPACKET_H

#include "core/Core.h"
#include "math/Math.h"
#include <vector>
#include <core/UUID.h>



namespace Axel
{
    class Mesh;
    class Material;

    enum class RenderPacketType { Mesh, UI, Particle, Skybox };

    struct RenderPacket
    {
        RenderPacketType Type;
        uint64_t SortKey;
        Ref<Mesh> Mesh;
        Ref<Material> Material;
        glm::mat4 Transform;
        uint32_t InstanceCount = 1;
        // For skinned meshes
        std::vector<Mat4> BoneMatrices;     
    };

    struct SpriteRenderPacket {
        Mat4 Transform;
        Vec4 Color;
        uint32_t TextureHandle;
        uint32_t MaterialIndexHandle;
        uint32_t padding[2]; 
    };
}


#endif
