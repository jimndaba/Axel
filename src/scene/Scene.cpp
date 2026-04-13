#include "axelpch.h"
#include "Scene.h"
#include <core/Logger.h>
#include <rendering/Renderer.h>


Axel::Scene::Scene()
{
    RegisterComponent<TagComponent>();
    RegisterComponent<TransformComponent>();
    RegisterComponent<SpriteComponent>();
    RegisterComponent<CameraComponent>();
    RegisterComponent<HierarchyComponent>();
}

Axel::Scene::~Scene()
{
}

Axel::UUID Axel::Scene::CreateEntity(const std::string& name)
{
	return CreateEntityWithUUID(UUID(), name);
}

Axel::UUID Axel::Scene::CreateEntityWithUUID(UUID uuid, const std::string& name)
{
    entt::entity handle = m_Registry.create();

    // Every entity in Axel MUST have these

	IDComponent idComp;
	idComp.ID = uuid;
    m_Registry.emplace<IDComponent>(handle,idComp);

	TagComponent tagComp;
	tagComp.Name = name.empty() ? "Entity" : name;
    m_Registry.emplace<TagComponent>(handle, tagComp);

    m_ReverseEntityMap[handle] = uuid;
    m_EntityMap[uuid] = handle;

    return uuid;
}


void Axel::Scene::DestroyEntity(UUID entity)
{
    auto it = m_EntityMap.find(entity);
    if (it == m_EntityMap.end())
        return;

    entt::entity e = it->second;

    m_ReverseEntityMap.erase(e);
    m_EntityMap.erase(it);

    m_Registry.destroy(e);
}

void Axel::Scene::OnRuntimeStart()
{
}

void Axel::Scene::OnRuntimeStop()
{
}

void Axel::Scene::OnUpdateRuntime(float dt)
{
    //Update Transforms
    auto transforms = GetAllEntitiesWith<TransformComponent>();
    for (auto [id, transform] : transforms)
    {
        // 1. Start with Identity
        Mat4 transformMatrix = Mat4(1.0f);

        // 2. Apply TRS (Translation * Rotation * Scale)
        transformMatrix = Math::Translate(transformMatrix, transform.Position);
        // Rotate X, then Y, then Z (Euler YXZ or XYZ is standard)
        transformMatrix = Math::Rotate(transformMatrix, Math::Radians(transform.Rotation.x), { 1, 0, 0 });
        transformMatrix = Math::Rotate(transformMatrix, Math::Radians(transform.Rotation.y), { 0, 1, 0 });
        transformMatrix = Math::Rotate(transformMatrix, Math::Radians(transform.Rotation.z), { 0, 0, 1 });
        transformMatrix = Math::Scale(transformMatrix, transform.Scale);

        // 3. Update the component data
        transform.WorldTransform = transformMatrix;
    }
}

void Axel::Scene::Serialize(IArchive& ar)
{
    ar.BeginStruct("Scene");
    ar.Property("Name", Name);
    ar.Property("AssetID", SceneAssetID);
    ar.Property("RootEntity", RootEntityID);
    ar.Property("ActiveCamera", ActiveCameraID);

    if (ar.GetMode() == ArchiveModeOptions::Load)
    {      
    }
    else
    {
        for (auto [uuid, handle] : m_EntityMap)
        {
            ar.BeginStruct("Entity");

            ar.Property("ID",uuid);

            for (auto [id, storage] : m_Registry.storage())
            {
                if (storage.contains(handle))
                {
                    // Check if we registered a serialiser for this component type
                    if (auto it = s_SerialiseMap.find(id); it != s_SerialiseMap.end())
                    {
                        it->second(*this, ar, uuid);
                    }
                }
            }          

            ar.EndStruct();
        }
    }
    

    ar.EndStruct();
}

Axel::UUID Axel::Scene::GetUUID(entt::entity e)
{
    return m_ReverseEntityMap.at(e);
}


