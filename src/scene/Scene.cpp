#include "axelpch.h"
#include "Scene.h"
#include "Entity.h"

Axel::Scene::Scene()
{
}

Axel::Scene::~Scene()
{
}

Axel::Entity Axel::Scene::CreateEntity(const std::string& name)
{
	return CreateEntityWithUUID(UUID(), name);
}

Axel::Entity Axel::Scene::CreateEntityWithUUID(UUID uuid, const std::string& name)
{
    entt::entity handle = m_Registry.create();

    // Every entity in Axel MUST have these
    m_Registry.emplace<IDComponent>(handle, uuid);
    m_Registry.emplace<TagComponent>(handle, name.empty() ? "Entity" : name);
    //m_Registry.emplace<TransformComponent>(handle);

    m_EntityMap[uuid] = handle;

    return { handle, this };
}

void Axel::Scene::DestroyEntity(Entity entity)
{

}

void Axel::Scene::OnRuntimeStart()
{
}

void Axel::Scene::OnRuntimeStop()
{
}

void Axel::Scene::OnUpdateRuntime(float dt)
{
}
