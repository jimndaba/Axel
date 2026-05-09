#pragma once
#ifndef SCENE_H
#define SCENE_H

#include "core/Core.h"
#include "entt/entt.hpp"
#include "core/UUID.h"
#include "core/Logger.h"
#include <core/Reflection.h>
#include <assets/ISerialisable.h>
#include <assets/AssetBase.h>
#include "Entity.h"

namespace Axel
{
	class AX_API Scene :public IAsset, public ISerialisable
	{
        using SerializeFn = std::function<void(Scene&, IArchive&, UUID)>;
        inline static std::unordered_map<size_t, SerializeFn> s_SerialiseMap;

        // Grant EntityView access to m_Registry
        template<typename... Comps>
        friend class EntityView;

	public:

        virtual const char* GetName() { return Name.c_str(); }
    public:
        Scene();
        ~Scene();

        // Entity Lifecycle
        UUID CreateEntity(const std::string& name = std::string());
        UUID CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
        void DestroyEntity(UUID entity);
        void Clear();

        // System Access
        template<typename... Components>
        EntityView<Components...> GetAllEntitiesWith()
        {
            return EntityView<Components...>(m_Registry, this);
        }
 
        // Component Management
        template<typename T, typename... Args>
        T& AddComponent(UUID entity, Args&&... args)
        {
            return m_Registry.emplace<T>(
                m_EntityMap.at(entity),
                std::forward<Args>(args)...);
        }

        template<typename T>
        void RemoveComponent(UUID entity) {
            auto it = m_EntityMap.find(entity);
            if (it != m_EntityMap.end())
            {
                m_Registry.remove<T>(it->second);
            }
        }

        template<typename T>
        bool HasComponent(UUID entity)
        {
            auto it = m_EntityMap.find(entity);
            if (it != m_EntityMap.end())
            {
                return m_Registry.all_of<T>(it->second);
            }
            return false;
        }

        template<typename T>
        T& GetComponent(UUID id)
        {
            return m_Registry.get<T>(m_EntityMap.at(id));
        }

        template<typename T>
        T* TryGetComponent(UUID entityID)
        {
            // 1. Convert our UUID to the internal EnTT entity handle
            if (m_EntityMap.find(entityID) == m_EntityMap.end())
                return nullptr;

            auto entt_entity = m_EntityMap.at(entityID);

            // 2. Use EnTT's try_get which returns a pointer or nullptr
            return m_Registry.try_get<T>(entt_entity);
        }

        std::vector<ISerialisable*> GetAllComponents(UUID entity);



        void Serialize(IArchive& ar) override;
        virtual AssetTypeOptions GetType() const override{ return AssetTypeOptions::Scene; }

        template<typename T>
        void RegisterComponent(const std::string& name = "Component");

        std::unordered_map<size_t, SerializeFn>  GetSerialiseMap() { return s_SerialiseMap; }

        UUID GetUUID(entt::entity e);       
        std::string Name = "Untitled";
       
        UUID SceneAssetID;
        UUID RootEntityID;
        UUID ActiveCameraID;
		UINT32 EntityCount = 0;

    private:
        entt::registry m_Registry;
        // High-speed lookup for UUID to EnTT handle
        std::unordered_map<UUID, entt::entity> m_EntityMap;
        std::unordered_map<entt::entity, UUID> m_ReverseEntityMap; // ADD THIS
        // We allow the Entity wrapper to access the private registry
	};
  
    template<typename T>
    inline void Scene::RegisterComponent(const std::string& name)
    {
        const size_t type = entt::type_hash<T>::value();

        if (s_SerialiseMap.contains(type))
            return;


        SerializeFn fn = [type,name](Scene& scene, IArchive& ar, UUID entity)
            {
                if (ar.GetMode() == ArchiveModeOptions::Save)
                {
                    if (!scene.template HasComponent<T>(entity))
                        return;

                    auto& component = scene.template GetComponent<T>(entity);
                    if (ar.BeginStruct(name.c_str()))
                    {
                        ReflectComponent(component, ar);
                        ar.EndStruct(); // end sequence item
                    }
                }
                else // LOAD
                {
                    // Only proceed if this component exists in YAML
                    if (ar.BeginStruct(name.c_str()))
                    {
                        AXLOG_INFO("Loading component : {} -> {} ",(uint64_t)entity, name);
                        // Create component BEFORE deserializing
                        auto& component = scene.template AddComponent<T>(entity);         
                        ReflectComponent(component,ar);
                        ar.EndStruct();
                    }
                }
            };

        s_SerialiseMap[type] = std::move(fn);
    }
}

#include "Entity.inl"

#endif