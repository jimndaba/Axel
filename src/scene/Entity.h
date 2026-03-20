#pragma once
#ifndef ENTITY_H
#define ENTITY_H

#include "Scene.h"

namespace Axel {
    class Entity {
    public:
        Entity(entt::entity handle, Scene* scene)
            : m_Handle(handle), m_Scene(scene) {}

        // Simple enough to copy and pass around the DLL
        bool operator==(const Entity& other) const { return m_Handle == other.m_Handle && m_Scene == other.m_Scene; }

        template<typename T>
        T& GetComponent() { return m_Scene->m_Registry.get<T>(m_Handle); }

        template<typename T>
        bool HasComponent() { return m_Scene->m_Registry.all_of<T>(m_Handle); }

        // Conversion to bool to check if entity is valid
        operator bool() const { return m_Handle != entt::null && m_Scene != nullptr; }

    private:
        entt::entity m_Handle{ entt::null };
        Scene* m_Scene = nullptr;
    };
}


#endif