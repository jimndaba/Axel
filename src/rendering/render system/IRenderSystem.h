#pragma once
#ifndef IRENDERSYSTEM_H
#define IRENDERSYSTEM_H

#include <math/Math.h>

namespace Axel
{
    class Scene;

    struct SceneUBOData{
        Mat4 ViewProjection;
        Mat4 ViewMatrix;
        Mat4 ProjectionMatrix;
        Vec3 CameraWorldPos;
        float Time;
        float NearPlane;
        float FarPlane;
        float FogDensity;
        float WindStrength;
    };

    struct SceneRenderDesc : SceneUBOData {  // CPU adds these
        uint32_t ViewportWidth;
        uint32_t ViewportHeight;
    };


    class IRenderSystem {
    public:
        virtual ~IRenderSystem() = default;
        // Called every frame
        virtual void Submit(Scene& scene, SceneRenderDesc& desc) = 0;
    };
}


#endif
