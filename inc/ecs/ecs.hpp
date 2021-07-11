/****************************************************************************

    MIT License

    Copyright (c) 2021 Aria Janke

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

*****************************************************************************/

#pragma once

#include <ecs/Entity.hpp>

#include <vector>
#include <algorithm>

namespace ecs {

/// @brief All inheriters of this struct will cause this ecs library to "inline"
///        the component directly inside the pointer table.
/// @note The downside to doing this, is it could make your pointer table MUCH
///       larger. The updside is it makes adding/removing the component much
///       faster. This maybe useful for large components most entities use.
/// @note Structures less than or equal to three pointers in size will be
///       inlined automatically, this may change in future releases.
struct InlinedComponent;

template <typename T>
struct DefineWouldInline;

// ----------------------------------------------------------------------------

/// @brief System class defines a behavior for entities in ecs architecture.
///
/// The update method defines this behavior and is called by the entity
/// manager. Since it's unfiltered it will have unfettered access to all
/// components.
template <typename ... Types>
class System {
public:
    using EntityContainer = std::vector<Entity<Types...>>;
    using ContainerView   = detail::Range<typename EntityContainer::iterator>;
    virtual ~System() {}
    virtual void update(const ContainerView &) = 0;
};

// ----------------------------------------------------------------------------

/// @brief An entity manager is a proper owner of a set entities. A manager
///        maintains the life span of the entities, and stream lines a set
///        of systems and calls to their update functions.
template <typename ... Types>
class EntityManager final : public detail::ReferenceManager {
public:
    using EntityType        = Entity  <Types...>;
    using SystemType        = System  <Types...>;
    using ComponentTypeList = TypeList<Types...>;

    template <typename T>
    struct HasType {
        static constexpr const bool k_value = TypeList<Types...>::template HasType<T>::k_value;
    };

    struct OnEntityDelete {
        virtual ~OnEntityDelete() {}
        virtual void operator () (EntityType &) const = 0;
    };

    EntityManager() {}
    EntityManager(const EntityManager &) = delete;
    EntityManager & operator = (const EntityManager &) = delete;
    ~EntityManager();

    /// Creates an entirely new entity pointing to a different set of
    /// components.
    /// @deprecated use "make_entity" instead (as per vocabulary limiting)
    [[deprecated]] EntityType create_new_entity();

    /// Creates an entirely new entity pointing to a different set of
    /// components.
    EntityType make_entity();

    /// Registers a system for use by the manager. Any "Unique to system"
    /// component databases should also be fed to this member function.
    ///
    /// @note Does not in anyway take ownership of the system. All systems must
    ///       outlive the manager object.
    ///
    /// @throws if nullptr is provided
    [[deprecated]] void register_system(SystemType *);

    /// Removes all systems currently in use by the manager.
    [[deprecated]] void clear_systems_list();

    /// Runs update calls to all registered systems.
    [[deprecated]] void update_systems();

    void run_system(SystemType &);

    /// @brief Responds to all deletion requests.
    ///
    /// @note When deleting entities, their components are deleted in the
    ///       order that they appear in the type list.
    ///       For instance Entity<A, B, C>, will first delete component A,
    ///       then B, and lastly C.
    /// @note This entity can be accessed in these destructors.
    /// @note The order in which individual entities are deleted, is left
    ///       undefined.
    ///
    /// This does not necessarily delete all references to deleted entities
    /// themselves should still maintain a valid state. All these entity
    /// references and entities however, will become "expired".
    void process_deletion_requests();

    ///
    /// @note Something I've noticed, I don't logic in destructors, but without
    ///       a way to inject behavior, destructor logic becomes unavoidable!
    ///
    void process_deletion_requests(OnEntityDelete &);

private:
    detail::ReferenceCounter * create_identity() override;

    void append_new_entities();

    std::vector<EntityType> m_unfiltered_entities;
    std::vector<EntityType> m_new_entities;
    std::vector<SystemType *> m_systems;
    std::vector<detail::UscDelNotifier *> m_ucsystems;
};

// -------------------------- end public interface ------------------------- //

template <typename ... Types>
EntityManager<Types...>::~EntityManager() {
    using namespace detail;
    for (auto & ent : m_unfiltered_entities)
        { EntityAtt::expire_entity(ent); }
    for (auto & ent : m_new_entities)
        { EntityAtt::expire_entity(ent); }
}

template <typename ... Types>
typename EntityManager<Types...>::EntityType
    EntityManager<Types...>::create_new_entity()
{ return make_entity(); }

template <typename ... Types>
typename EntityManager<Types...>::EntityType
    EntityManager<Types...>::make_entity()
{ return EntityType::make_entity(*this); }

template <typename ... Types>
void EntityManager<Types...>::register_system(SystemType * sysptr) {
    if (!sysptr) {
        throw std::invalid_argument("EntityManager: cannot accept nullptr for "
                                    "a system pointer parameter.");
    }
    m_systems.push_back(sysptr);
    if (auto * ucs = dynamic_cast<detail::UscDelNotifier *>(sysptr)) {
        m_ucsystems.push_back(ucs);
    }
}

template <typename ... Types>
void EntityManager<Types...>::clear_systems_list() {
    m_systems.clear();
    m_ucsystems.clear();
}

template <typename ... Types>
void EntityManager<Types...>::update_systems() {
    detail::Range<decltype (m_unfiltered_entities.begin())> crange
        { m_unfiltered_entities.begin(), m_unfiltered_entities.end() };
    for (auto * sys : m_systems) {
        sys->update(crange);
    }

    append_new_entities();
}

template <typename ... Types>
void EntityManager<Types...>::run_system(SystemType & syncro_system) {
    detail::Range<decltype (m_unfiltered_entities.begin())> crange
        { m_unfiltered_entities.begin(), m_unfiltered_entities.end() };
    syncro_system.update(crange);
}

template <typename ... Types>
void EntityManager<Types...>::process_deletion_requests() {
    class NullOnEntityDelete final : public OnEntityDelete {
        void operator () (EntityType &) const final {}
    };
    NullOnEntityDelete inst;
    process_deletion_requests(inst);
}

template <typename ... Types>
void EntityManager<Types...>::process_deletion_requests(OnEntityDelete & on_delete) {
    using namespace detail;
    append_new_entities();
    for (auto & ent : m_unfiltered_entities) {
        if (!EntityRefAtt::is_requesting_deletion(ent)) continue;
        on_delete(ent);
        for (auto * ucs : m_ucsystems) {
            ucs->notify_deletion(ent);
        }

        EntityAtt::expire_entity(ent);
    }
    auto del_beg = std::remove_if(m_unfiltered_entities.begin(), m_unfiltered_entities.end(),
                                  EntityRefAtt::is_requesting_deletion);
    m_unfiltered_entities.erase(del_beg, m_unfiltered_entities.end());
    append_new_entities();
}

template <typename ... Types>
detail::ReferenceCounter * EntityManager<Types...>::create_identity() {
    static_assert(std::is_base_of<detail::ReferenceCounter, detail::ComponentTableHead<Types...>>::value, "");

    auto * identity = new detail::ComponentTableHead<Types...>;
    identity->reference_manager = this;

    EntityType entity;
    try {
        detail::EntityAtt::set_counter(entity, identity);
    } catch (...) {
        // we'd prefer RAII
        delete identity;
        throw;
    }
    // not thread safe!
    m_new_entities.push_back(entity);

    return identity;
}

template <typename ... Types>
void EntityManager<Types...>::append_new_entities() {
    m_unfiltered_entities.insert(m_unfiltered_entities.end(),
                                 m_new_entities.begin(), m_new_entities.end());
    m_new_entities.clear();
}

namespace detail {

inline /* static */ ReferenceManager & ReferenceManager::null_instance() {
    class NullManaCounter final : public ecs::detail::ReferenceCounter {
        static void id_func() {}
        virtual void * full_downcast(IdFuncPtr id_ptr) noexcept override
            { return id_ptr == id_func ? this : nullptr; }
    };

    class NullReferenceManager final : public ecs::detail::ReferenceManager {
        using ReferenceCounter = ecs::detail::ReferenceCounter;

        ReferenceCounter * create_identity() override {
            auto * counter = new NullManaCounter;
            counter->count = 1;
            counter->reference_manager = this;
            return counter;
        }
    };

    static NullReferenceManager instance;
    return instance;
}

// -----------------------------------------------------------------------------

inline void decrement(ReferenceCounter * counter) {
    if (!counter) return;
    if (--counter->count == 0) {
        delete counter;
    }
}

inline void increment(ReferenceCounter * counter) {
    if (!counter) return;
    ++counter->count;
}

} // end of detail namespace

} // end of ecs namespace
