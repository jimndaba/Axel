#pragma once
#ifndef SCENE_H
#define SCENE_H

#include "core/Core.h"
#include "entt/entt.hpp"
#include "core/UUID.h"
#include "Components.h"
#include <assets/ISerialisable.h>
#include "Entity.h"

namespace Axel
{
	class AX_API Scene : ISerialisable
	{
        using SerializeFn = std::function<void(Scene&, IArchive&, UUID)>;
        inline static std::unordered_map<size_t, SerializeFn> s_SerialiseMap;

        // Grant EntityView access to m_Registry
        template<typename... Comps>
        friend class EntityView;

	public:
    public:
        Scene();
        ~Scene();

        // Entity Lifecycle
        UUID CreateEntity(const std::string& name = std::string());
        UUID CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
        void DestroyEntity(UUID entity);

        // Runtime Logic
        void OnRuntimeStart();
        void OnRuntimeStop();
        void OnUpdateRuntime(float dt); // Called by Application loop

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

        void Serialize(IArchive& ar) override;

        template<typename T>
        void RegisterComponent();

        UUID GetUUID(entt::entity e);       
        std::string Name = "Untitled";
        UUID SceneAssetID;
        UUID RootEntityID;
        UUID ActiveCameraID;

    private:
        entt::registry m_Registry;
        // High-speed lookup for UUID to EnTT handle
        std::unordered_map<UUID, entt::entity> m_EntityMap;
        std::unordered_map<entt::entity, UUID> m_ReverseEntityMap; // ADD THIS
        // We allow the Entity wrapper to access the private registry
	};
  
    template<typename T>
    inline void Scene::RegisterComponent()
    {
        const size_t type = entt::type_hash<T>::value();

        if (s_SerialiseMap.contains(type))
            return;

        SerializeFn fn = [type](Scene& scene, IArchive& ar, UUID entity)
            {
                if (!scene.template HasComponent<T>(entity))
                    return;

                auto& component = scene.template GetComponent<T>(entity);
                ar.BeginStruct("Component");
                ar.Property("Type", type);
                component.Serialize(ar);
                ar.EndStruct();
            };

        s_SerialiseMap[type] = fn;
    }
}

#include "Entity.inl"

#endif