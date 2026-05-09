#include "axelpch.h"
#include "Scene.h"
#include <core/Logger.h>
#include <rendering/Renderer.h>

Axel::Scene::Scene()
{
    RegisterComponent<IDComponent>("IDComponent");
    RegisterComponent<TagComponent>("TagComponent");
    RegisterComponent<TransformComponent>("TransformComponent");
    RegisterComponent<MeshComponent>("MeshComponent");
    RegisterComponent<CameraComponent>("CameraComponent");
    RegisterComponent<HierachyComponent>("HierachyComponent");
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

void Axel::Scene::Clear()
{
    m_Registry.clear();
    m_EntityMap.clear();
    m_ReverseEntityMap.clear();
}

std::vector<Axel::ISerialisable*> Axel::Scene::GetAllComponents(UUID entity)
{
    std::vector<Axel::ISerialisable*> components;

    if (m_EntityMap.find(entity) != m_EntityMap.end())
    {
        auto e_entity = m_EntityMap[entity];

        // We iterate through all component storages in the registry
        for (auto [id, storage] : m_Registry.storage()) {
            if (storage.contains(e_entity)) {
                // We cast the raw pointer to our base interface. 
                // This assumes all your components inherit ISerializable.
                void* rawData = storage.value(e_entity);
                components.push_back(static_cast<ISerialisable*>(rawData));
            }
        }
    }
    return components;
}

void Axel::Scene::Serialize(IArchive& ar)
{
    if (ar.BeginStruct("Scene"))
    {
        VisitProperty("Name", Name, ar);
        VisitProperty("AssetID", SceneAssetID, ar);
        VisitProperty("RootEntity", RootEntityID, ar);
        VisitProperty("ActiveCamera", ActiveCameraID, ar);

        if (ar.GetMode() == ArchiveModeOptions::Load)
        {
            VisitProperty("EntityCount", EntityCount, ar);
            if (ar.BeginCollection("Entities", EntityCount))
            {

                while (ar.HasNext())
                {
                    
                    if (ar.BeginStruct("Entity"))
                    {
                        UUID entityID{};
                        VisitProperty("ID", entityID, ar);
                        auto entity = CreateEntityWithUUID(entityID);
                        if (ar.BeginCollection("Components", EntityCount))
                        {
                            while (ar.HasNext())
                            {
                                for (auto& [type, fn] : s_SerialiseMap)
                                {
                                    fn(*this, ar, entityID);
                                }
                                ar.NextItem();
                            }
                           
                            ar.EndCollection(); // Components
                        }
                        ar.EndStruct(); //entity                       
                    }
                    ar.NextItem();
                }
                ar.EndCollection();
            }
        }
        else
        {
            ar.BeginCollection("Entities", EntityCount);
            for (auto& [uuid, handle] : m_EntityMap)
            {
                ar.BeginStruct("Entity");
                VisitProperty("ID", uuid, ar);
                uint32_t size = 0;
                ar.BeginCollection("Components",size);
                for (auto [id, storage] : m_Registry.storage())
                {
                    if (!storage.contains(handle))
                        continue;

                    if (auto it = s_SerialiseMap.find(id);
                        it != s_SerialiseMap.end())
                    {

                        it->second(*this, ar, uuid);
                    }
                }
                ar.EndCollection();
                ar.EndStruct();
            }
            ar.EndCollection();
        }

        ar.EndStruct();
    }
}

Axel::UUID Axel::Scene::GetUUID(entt::entity e)
{
    return m_ReverseEntityMap.at(e);
}


