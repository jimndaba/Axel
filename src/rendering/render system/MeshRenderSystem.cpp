#include "axelpch.h"
#include "MeshRenderSystem.h"
#include <scene/Scene.h>
#include <assets/AssetManager.h>
#include <rendering/Mesh.h>
#include <rendering/Renderer.h>

void Axel::MeshRenderSystem::Submit(Scene& scene, SceneRenderDesc& desc)
{
    for (const auto& [id, transform, hierarchy] : scene.GetAllEntitiesWith<TransformComponent, HierachyComponent>())
    {
        transform.LocalTransform = glm::translate(Mat4(1.0f), transform.Position) *
            glm::toMat4(glm::quat(glm::radians(transform.Rotation))) *
            glm::scale(Mat4(1.0f), transform.Scale);
        // 3. Root entities: Local and World transforms are identical
        transform.WorldTransform = transform.LocalTransform;
    }

    for (const auto& [id, transform, mesh] : scene.GetAllEntitiesWith<TransformComponent, MeshComponent>())
    {
        if (mesh.bIsVisible)
        {
            float depth = ComputeDepth(transform.Position, desc);
            Renderer::SubmitMesh(mesh, transform, depth);
        }
    }
}

float  Axel::MeshRenderSystem::ComputeDepth(const Vec3& worldPos, const SceneRenderDesc& desc)
{
    // Transform world position into camera space
    Vec4 cameraSpacePos = desc.ViewMatrix * Vec4(worldPos, 1.0f);
    // Negate Z because in view space, forward is -Z
    // Normalise by far plane so result is 0..1
    float linearDepth = -cameraSpacePos.z;
    return glm::clamp(linearDepth / desc.FarPlane, 0.0f, 1.0f);
}