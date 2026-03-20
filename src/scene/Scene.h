#pragma once
#ifndef SCENE_H
#define SCENE_H

#include "core/Core.h"
#include "core/UUID.h"
#include "Components.h"
#include "entt/entt.hpp"

namespace Axel
{
	class Entity;


	class AX_API Scene
	{
	public:
    public:
        Scene();
        ~Scene();

        // Entity Lifecycle
        Entity CreateEntity(const std::string& name = std::string());
        Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
        void DestroyEntity(Entity entity);

        // Runtime Logic
        void OnRuntimeStart();
        void OnRuntimeStop();
        void OnUpdateRuntime(float dt); // Called by Application loop

        // System Access
        template<typename... Components>
        auto GetAllEntitiesWith() {
            return m_Registry.view<Components...>();
        }

    private:
        entt::registry m_Registry;
        // High-speed lookup for UUID to EnTT handle
        std::unordered_map<UUID, entt::entity> m_EntityMap;
        // We allow the Entity wrapper to access the private registry
        friend class Entity;

	};


}


#endif