#pragma once
#ifndef SCENEPACKET_H
#define SCENEPACKET_H

#include "core/Core.h"
#include "math/Math.h"
#include <vector>



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
}


#endif
