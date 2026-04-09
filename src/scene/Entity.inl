#pragma once

#include "Scene.h"

namespace Axel {
    // =========================
    // EntityView
    // =========================

    template<typename... Components>
    EntityView<Components...>::EntityView(entt::registry& registry, Scene* scene)
        : m_View(registry.view<Components...>()), m_Scene(scene) {
    }

    template<typename... Components>
    EntityView<Components...>::Iterator::Iterator(EntityIterator it, Scene* scene)
        : m_It(it), m_Scene(scene) {
    }

    template<typename... Components>
    auto EntityView<Components...>::Iterator::operator*() const
    {
        entt::entity e = *m_It;

        // Convert to UUID
        UUID id = m_Scene->GetUUID(e);

        return std::tuple<UUID, Components&...>(
            id,
            m_Scene->m_Registry.template get<Components>(e)...);
    }

    template<typename... Components>
    typename EntityView<Components...>::Iterator& EntityView<Components...>::Iterator::operator++() {
        ++m_It;
        return *this;
    }

    template<typename... Components>
    bool EntityView<Components...>::Iterator::operator!=(const Iterator& other) const {
        return m_It != other.m_It;
    }

    template<typename... Components>
    typename EntityView<Components...>::Iterator EntityView<Components...>::begin() {
        return Iterator(m_View.begin(), m_Scene);
    }

    template<typename... Components>
    typename EntityView<Components...>::Iterator EntityView<Components...>::end() {
        return Iterator(m_View.end(), m_Scene);
    }

}