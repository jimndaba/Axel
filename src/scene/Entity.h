#pragma once
#ifndef ENTITY_H
#define ENTITY_H

#include <entt/entt.hpp>
#include <tuple>


namespace Axel {

    class Scene;

    // -------------------------
    // EntityView
    // -------------------------

    template<typename... Components>
    class EntityView
    {
    public:
        using ViewType = entt::view<entt::get_t<Components...>>;

        EntityView(entt::registry& registry, Scene* scene);

        struct Iterator
        {
            // Instead of using ViewType::iterator here, 
            // we use a pointer or the underlying entt type directly 
            // to avoid the pack expansion in the alias.
            using EntityIterator = typename ViewType::iterator;

            Iterator(EntityIterator it, Scene* scene);

            Iterator& operator++();
            bool operator!=(const Iterator& other) const;

            // Return 'auto' so the tuple expansion happens in the .inl
            auto operator*() const;

        private:
            EntityIterator m_It;
            Scene* m_Scene;
        };

        Iterator begin();
        Iterator end();

    private:
        // EnTT 3.10+ uses get_t for views with multiple components
        entt::view<entt::get_t<Components...>> m_View;
        Scene* m_Scene;
    };

} // namespace Axel


#endif