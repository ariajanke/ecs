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

#include <ecs/EntityRef.hpp>

#include <tuple>

namespace ecs {

template <typename ... Types>
class System;

/// Type alias for std::tuple
template <typename ... Types>
using Tuple = std::tuple<Types...>;

/// @brief
/// Entities are a means to point to/access a set of uniquely typed components.
///
/// This class has with it semantics that allow a back-and-forth between the
/// entity type and an entity reference. @n
///
/// This is the "unfiltered" type, where unlimited access to any component is
/// given. Any entity may create other entities (prone to revision).
template <typename ... Types>
class Entity final {
public:
    friend class detail::EntityAtt;
    friend class EntityRef;

    using ManagerType       = EntityManager<Types...>;
    using SystemType        = System       <Types...>;
    using ComponentTypeList = TypeList     <Types...>;

    /// Component table's size in bytes (includes "is present" bitfield)
    /// This may become deprecated in the future
    /// I've plans to handle very large number of components
    static constexpr const std::size_t k_component_table_size =
        sizeof(detail::ComponentTable<Types...>);

    /// Size of the table of components in bytes (excluding the "is present"
    /// bit field
    static constexpr const std::size_t k_components_size =
        detail::ComponentTableHead<Types...>::k_sizeof_components;

    /// The number of components inlined in the component table.
    ///
    /// Generally components three pointers or fewer in size are inlined into
    /// the table itself, skipping an allocation call when adding the component.
    /// Components can be inlined explicitly by inheriting from
    /// ecs::InlinedComponent.
    /// @note The bigger the component table, the more cache misses maybe
    ///       incurred. (this is not tested)
    static constexpr const std::size_t k_number_of_components_inlined =
        detail::ComponentTableHead<Types...>::CountInlined::k_count;

    static constexpr const std::size_t k_component_count = sizeof...(Types);

    /// @brief Creates a null entity, which may not be associated with any
    ///        components.
    Entity() {}

    /// @brief Completes an entity reference, allowing client code to access
    ///        the components associated with the entity.
    explicit Entity(const EntityRef &);

    /// @brief Completes an entity reference, allowing client code to access
    ///        the components associated with the entity.
    explicit Entity(EntityRef &&);

    /// @brief Creates another point of access to a set of components. It is
    ///        very important to note this does not create a new entity.
    /// @warning Does not create a new entity.
    /// @see Entity create_new_entity() const
    Entity(const Entity & rhs);

    /// @brief Creates another point of access to a set of components. It is
    ///        very important to note this does not create a new entity.
    /// @warning Does not create a new entity.
    /// @see Entity create_new_entity() const
    Entity(Entity && rhs) { swap(rhs); }

    /// @brief Does not necessarily delete all components associate with this
    ///        entity. Unless it is the last entity used to refer to the
    ///        components.
    ~Entity() { detail::decrement(m_table); }

    /// @brief Causes this entity to refer to the components of another entity.
    Entity & operator = (const Entity &);

    /// @brief Causes this entity to refer to the components of another entity.
    Entity & operator = (Entity &&);

    /// @returns True if two entities refer to the same components.
    bool operator == (const Entity & rhs) const { return m_table == rhs.m_table; }

    /// @returns True if two entities refer to different components.
    bool operator != (const Entity & rhs) const { return m_table != rhs.m_table; }

    /// @brief Creates a new entity using a reference manager provided as a
    ///        parameter.
    /// @returns An entirely new entity, which will have its own set of
    ///          components.
    /// @deprecated use "make_entity" instead, as per vocabulary limiting
    [[deprecated]] static Entity create_new_entity(detail::ReferenceManager &);

    /// @brief Creates a new entity using the refenece manager of this entity.
    /// @returns An entirely new entity, which will have its own set of
    ///          components.
    /// @deprecated use "make_entity" instead, as per vocabulary limiting
    [[deprecated]] Entity create_new_entity() const;

    /// @brief Creates a new entity using a reference manager provided as a
    ///        parameter.
    /// @returns An entirely new entity, which will have its own set of
    ///          components.
    /// @deprecated use "make_entity" instead, as per vocabulary limiting
    static Entity make_entity(detail::ReferenceManager &);

    /// @brief Creates a new entity using the refenece manager of this entity.
    /// @returns An entirely new entity, which will have its own set of
    ///          components.
    /// @deprecated use "make_entity" instead, as per vocabulary limiting
    Entity make_entity() const;

    /// @tparam T type of component to check
    /// @returns true if the component is present
    template <typename T>
    bool has() const { return (ptr<T>() != nullptr); }

    /// @tparam T one type of component to check
    /// @tparam U another type of component to check
    /// @tparam FurtherTypes any further component types to check
    /// @returns true if all components are present, false otherwise
    template <typename T, typename U, typename ... FurtherTypes>
    bool has_all() const;

    /// @tparam T one type of component to check
    /// @tparam U another type of component to check
    /// @tparam FurtherTypes any further component types to check
    /// @returns true if any component is present, false only if all are absent
    template <typename T, typename U, typename ... FurtherTypes>
    bool has_any() const;

    /// @brief Gets a writable component by type.
    /// @tparam T type of component to get
    /// @throws if the requested component has not been added
    template <typename T>
    T & get();

    /// @brief Gets a tuple of writable component references by their types.
    /// @tparam T one type of component to get
    /// @tparam U another type of component to get
    /// @tparam FurtherTypes any further component types to get
    /// @throws if any requested component has not been added
    template <typename T, typename U, typename ... FurtherTypes>
    Tuple<T &, U &, FurtherTypes & ...> get();

    /// @brief Gets a read only component by type.
    /// @tparam T type of component to get
    /// @throws if the requested component has not been added
    template <typename T>
    const T & get() const;

    /// @brief Gets a tuple of read only component references by their types.
    /// @tparam T one type of component to get
    /// @tparam U another type of component to get
    /// @tparam FurtherTypes any further component types to get
    /// @throws if any requested component has not been added
    template <typename T, typename U, typename ... FurtherTypes>
    Tuple<const T &, const U &, const FurtherTypes & ...> get() const;

    /// @brief Gets a pointer to a writable component by type.
    /// @tparam T type of component to get
    /// @returns a nullptr if the component is not present
    template <typename T>
    T * ptr() { return m_table->template get_ptr<T>(); }

    /// @brief Gets a tuple of pointers to writable components by their types.
    /// @tparam T one type of component to get
    /// @tparam U another type of component to get
    /// @tparam FurtherTypes any further component types to get
    /// @throws if any requested component has not been added
    template <typename T, typename U, typename ... FurtherTypes>
    Tuple<T *, U *, FurtherTypes * ...> ptr();

    /// @brief Gets a pointer to a read only component by type.
    /// @tparam T type of component to get
    /// @returns a nullptr if the component is not present
    template <typename T>
    const T * ptr() const { return m_table->template get_ptr<T>(); }

    template <typename T, typename U, typename ... FurtherTypes>
    Tuple<const T *, const U *, const FurtherTypes * ...> ptr() const;

    /// @brief Adds a new component by type.
    /// @tparam T type of component to get
    /// @throws if component of type T is already present
    template <typename T>
    T & add() { return m_table->template add<T>(); }

    template <typename T, typename U, typename ... FurtherTypes>
    Tuple<T &, U &, FurtherTypes & ...> add();

    /// @brief Gets a writable reference to a component by type, if it is not
    /// present, it is added.
    /// @tparam T type of component to get
    template <typename T>
    T & ensure() { return ptr<T>() ? get<T>() : add<T>(); }

    template <typename T, typename U, typename ... FurtherTypes>
    Tuple<T &, U &, FurtherTypes & ...> ensure();

    /// @brief Removes a component by type.
    /// @tparam T type of component to remove
    /// @throws if the component is not present
    template <typename T>
    void remove() { m_table->template remove<T>(); }

    template <typename T, typename U, typename ... FurtherTypes>
    void remove();

    /// Requested that the refered entity be deleted by the owning manager
    /// object. Entities cannot delete themselves.
    void request_deletion() { m_table->requesting_deletion = true; }

    /// @returns true if the entity is requesting deletion
    ///
    /// @note An entity which is requesting deletion is not an expired entity.
    ///       (yet!)
    /// @warning there should be no behavior/system that depends on knowing
    ///          whether an entity is about to be delete or not
    [[deprecated]] bool is_requesting_deletion() const { return m_table->requesting_deletion; }

    /// @brief Evalates to true if the entity has not expired.
    /// @note on semantics: often used to "evaluate true, then access components"
    operator bool () const { return !has_expired(); }

    /// @returns True if the entity can no longer be used to
    ///          retrieve/add/remove components.
    bool has_expired() const { return m_table ? m_table->expired : true; }

    /// @brief Swaps component tables between two entities.
    void swap(Entity & rhs) { std::swap(m_table, rhs.m_table); }

    /// @brief Swaps component tables between an entity and a reference to an
    ///        entity.
    /// @throws If the entity reference is not derived from the same entity
    ///         type as this one.
    void swap(EntityRef &);

    /// @note hash code cannnot be guaranteed to be unique if the code outlives
    ///       it's original entity
    /// @returns a unique hash code that identifies this entity
    std::size_t hash() const noexcept;

private:
    static constexpr const auto k_comp_not_present_msg =
        "Entity::get<T>: component of this type is not present.";

    template <typename ... OtherTypes>
    struct HasAllAsComponents {
        static constexpr const bool k_value = true;
    };

    template <typename HeadType, typename ... OtherTypes>
    struct HasAllAsComponents<HeadType, OtherTypes...> :
        public HasAllAsComponents<OtherTypes...>
    {
        static constexpr const bool k_value =
               TypeList<Types...>::template HasType<HeadType>::k_value
            && HasAllAsComponents<OtherTypes...>::k_value;
    };

    template <typename ... OtherTypes>
    static constexpr const bool kt_all_unique_and_all_components =
           cul::UniqueTypes<OtherTypes...>::k_value
        && HasAllAsComponents<OtherTypes...>::k_value;

    template <typename ... OtherTypes>
    struct AssertAllCompsAreUniqueAndPresent {
        static_assert(kt_all_unique_and_all_components<OtherTypes...>,
            "All listed types must be a parameter pack where each parameter is a "
            "unique type, and each type is a component type used to define this "
            "Entity.");
        constexpr AssertAllCompsAreUniqueAndPresent() {}
    };

    using ComponentsTable = detail::ComponentTableHead<Types...>;

    /// @throws  If the component table associated with the reference is of a
    ///          difference type than this Entity type. It may also throw
    ///          the given entity reference is empty.
    /// @returns The full component table from a given entity reference. This
    ///          function will never return nullptr
    static ComponentsTable * get_fulltable(const EntityRef &);

    template <typename T, typename U, typename ... FurtherTypes>
    Tuple<T &, U &, FurtherTypes & ...> add_impl() noexcept;

    template <typename T, typename U, typename ... FurtherTypes>
    void remove_impl() noexcept;

    ComponentsTable * m_table = nullptr;
};

// ----------------------------------------------------------------------------

namespace detail {

class EntityRefAtt {
    template <typename ... Types>
    friend class ::ecs::Entity;

    static ReferenceManager * get_reference_manager(const EntityRef & er) {
        if (!er.m_identity) return nullptr;
        return er.m_identity->reference_manager;
    }

    static ReferenceCounter * get_counter(const EntityRef & er)
        { return er.m_identity; }

    static void set_counter(EntityRef & er, ReferenceCounter * counter)
        { er.m_identity = counter; }

    template <typename ... Types>
    friend class ::ecs::EntityManager;

    static bool is_requesting_deletion(const EntityRef & er)
        { return er.m_identity ? er.m_identity->requesting_deletion : true; }
};

class EntityAtt {
    template <typename ... Types>
    friend class ::ecs::EntityManager;

    template <typename ... Types>
    friend class ::ecs::Entity;

    template <typename ... Types>
    static void expire_entity(Entity<Types...> & e) {
        // destructors of components first with the entity yet to expire...
        e.m_table->remove_all();
        e.m_table->expired = true;
    }

    template <typename ... Types>
    static void set_counter(Entity<Types...> & e, ReferenceCounter * counter) {
        if (!counter || e.m_table) {
            throw std::invalid_argument("[library error] counter must not be nullptr, and entity must be empty");
        }

        using ComponentsTable = typename Entity<Types...>::ComponentsTable;
        e.m_table = reinterpret_cast<ComponentsTable *>(counter->full_downcast(ComponentsTable::id_func));

        if (!e.m_table) {
            throw std::runtime_error("[library error] failed to down cast counter");
        }
        detail::increment(counter);
    }
};

} // end of detail namespace

template <typename ... Types>
/* implicit */ Entity<Types ...>::Entity(const EntityRef & eref) {
    if (!eref) return;
    auto * fulltable = get_fulltable(eref);
    detail::increment(fulltable);
    m_table = fulltable;
}

template <typename ... Types>
/* implicit */ Entity<Types ...>::Entity(EntityRef && eref)
    { swap(eref); }

template <typename ... Types>
Entity<Types ...>::Entity(const Entity & rhs):
    m_table(rhs.m_table)
    { detail::increment(m_table); }

template <typename ... Types>
Entity<Types...> & Entity<Types...>::operator = (const Entity & rhs) {
    Entity temp(rhs);
    swap(temp);
    return *this;
}

template <typename ... Types>
Entity<Types...> & Entity<Types...>::operator = (Entity && rhs) {
    swap(rhs);
    return *this;
}

template <typename ... Types>
/* static */ Entity<Types...> Entity<Types...>::create_new_entity
    (detail::ReferenceManager & manager)
{ return make_entity(manager); }

template <typename ... Types>
Entity<Types ...> Entity<Types ...>::create_new_entity() const
    { return make_entity(); }

template <typename ... Types>
/* static */ Entity<Types...> Entity<Types...>::make_entity
    (detail::ReferenceManager & manager)
{
    using RtError = std::runtime_error;
    auto * counter = manager.create_identity();
    if (!counter) {
        throw RtError("Entity<Types...>::make_entity(): failed to create new "
                      "entity. [this maybe a bug in *this* library, sorry]");
    }

    try {
        Entity e;
        detail::EntityAtt::set_counter(e, counter);
        return e;
    } catch (...) {
        delete counter;
        throw;
    }
}

template <typename ... Types>
Entity<Types...> Entity<Types...>::make_entity() const {
    if (m_table) {
        return Entity<Types ...>::make_entity(*m_table->reference_manager);
    }
    throw std::runtime_error("Entity::make_entity(): uninitialized entities "
                             "cannot create new entities.");
}

// new as of 2021-0711
template <typename ... Types>
template <typename T, typename U, typename ... FurtherTypes>
bool Entity<Types ...>::has_all() const {
    AssertAllCompsAreUniqueAndPresent<T, U, FurtherTypes...>{};
    bool has_t = has<T>();
    if constexpr (sizeof...(FurtherTypes) > 0) {
        return has_t && has_all<U, FurtherTypes...>();
    } else {
        return has_t && has<U>();
    }
}

// new as of 2021-0711
template <typename ... Types>
template <typename T, typename U, typename ... FurtherTypes>
bool Entity<Types ...>::has_any() const {
    AssertAllCompsAreUniqueAndPresent<T, U, FurtherTypes...>{};
    bool has_t = has<T>();
    if constexpr (sizeof...(FurtherTypes) > 0) {
        return has_t || has_any<U, FurtherTypes...>();
    } else {
        return has_t || has<U>();
    }
}

template <typename ... Types>
template <typename T>
T & Entity<Types ...>::get() {
    if (auto * rv = ptr<T>()) return *rv;
    throw std::runtime_error(k_comp_not_present_msg);
}

// new as of 2021-0711
template <typename ... Types>
template <typename T, typename U, typename ... FurtherTypes>
Tuple<T &, U &, FurtherTypes & ...> Entity<Types ...>::get() {
    AssertAllCompsAreUniqueAndPresent<T, U, FurtherTypes...>{};
    auto & t = get<T>();
    if constexpr (sizeof...(FurtherTypes) > 0) {
        // should produce a single element tuple of a reference to T
        return std::tuple_cat(Tuple<T &>(t), get<U, FurtherTypes...>());
    } else {
        return Tuple<T &, U &>(t, get<U>());
    }
}

template <typename ... Types>
template <typename T>
const T & Entity<Types ...>::get() const {
    if (const auto * rv = ptr<T>()) return *rv;
    throw std::runtime_error(k_comp_not_present_msg);
}

// new as of 2021-0711
template <typename ... Types>
template <typename T, typename U, typename ... FurtherTypes>
Tuple<const T &, const U &, const FurtherTypes & ...>
    Entity<Types ...>::get() const
{
    AssertAllCompsAreUniqueAndPresent<T, U, FurtherTypes...>{};
    using std::make_tuple, std::tuple_cat;
    if constexpr (sizeof...(FurtherTypes) > 0) {
        return tuple_cat(make_tuple(get<T>()), get<U, FurtherTypes...>());
    } else {
        return make_tuple(get<T>(), get<U>());
    }
}

// new as of 2021-0711
template <typename ... Types>
template <typename T, typename U, typename ... FurtherTypes>
Tuple<T *, U *, FurtherTypes * ...> Entity<Types ...>::ptr() {
    AssertAllCompsAreUniqueAndPresent<T, U, FurtherTypes...>{};
    using std::make_tuple, std::tuple_cat;
    T * t = ptr<T>();
    if constexpr (sizeof...(FurtherTypes) > 0) {
        return tuple_cat(make_tuple(t), ptr<U, FurtherTypes...>());
    } else {
        return make_tuple(t, ptr<U>());
    }
}

// new as of 2021-0711
template <typename ... Types>
template <typename T, typename U, typename ... FurtherTypes>
Tuple<const T *, const U *, const FurtherTypes * ...>
    Entity<Types ...>::ptr() const
{
    AssertAllCompsAreUniqueAndPresent<T, U, FurtherTypes...>{};
    using std::make_tuple, std::tuple_cat;
    const T * t = ptr<T>();
    if constexpr (sizeof...(FurtherTypes) > 0) {
        return tuple_cat(make_tuple(t), ptr<U, FurtherTypes...>());
    } else {
        return make_tuple(t, ptr<U>());
    }
}

// new as of 2021-0711
template <typename ... Types>
template <typename T, typename U, typename ... FurtherTypes>
Tuple<T &, U &, FurtherTypes & ...> Entity<Types ...>::add() {
    AssertAllCompsAreUniqueAndPresent<T, U, FurtherTypes...>{};
    if (!has_any<T, U, FurtherTypes...>()) {
        return add_impl<T, U, FurtherTypes...>();
    }
    throw std::runtime_error("Entity::add: cannot add component(s) that are "
                             "already present.");
}

// new as of 2021-0711
template <typename ... Types>
template <typename T, typename U, typename ... FurtherTypes>
Tuple<T &, U &, FurtherTypes & ...> Entity<Types ...>::ensure() {
    AssertAllCompsAreUniqueAndPresent<T, U, FurtherTypes...>{};
    auto & t = ensure<T>();
    if constexpr (sizeof...(FurtherTypes) > 0) {
        return std::tuple_cat(Tuple<T &>(t), ensure<U, FurtherTypes...>());
    } else {
        return Tuple<T &, U &>(t, ensure<U>());
    }
}

// new as of 2021-0711
template <typename ... Types>
template <typename T, typename U, typename ... FurtherTypes>
void Entity<Types ...>::remove() {
    AssertAllCompsAreUniqueAndPresent<T, U, FurtherTypes...>{};
    if (has_all<T, U, FurtherTypes...>()) {
        return remove_impl<T, U, FurtherTypes...>();
    }
    throw std::runtime_error("Entity::remove: cannot remove all components, "
                             "unless all are present.");
}

template <typename ... Types>
void Entity<Types ...>::swap(EntityRef & rhs) {
    using namespace detail;
    auto full_table = rhs ? get_fulltable(rhs) : nullptr;
    EntityRefAtt::set_counter(rhs, m_table);
    m_table = full_table;
}

template <typename ... Types>
std::size_t Entity<Types ...>::hash() const noexcept {
    auto identity = static_cast<detail::ReferenceCounter *>(m_table);
    return reinterpret_cast<std::size_t>(identity);
}

template <typename ... Types>
/* private static */ typename Entity<Types ...>::ComponentsTable *
    Entity<Types ...>::get_fulltable(const EntityRef & entity_ref)
{
    using namespace detail;
    using InvArg = std::invalid_argument;
    if (!entity_ref) {
        throw InvArg("Entity::get_fulltable: Given entity reference must not "
                     "be null when retrieving component table.");
    }

    auto * counter   = EntityRefAtt::get_counter(entity_ref);
    auto * fulltable =
        // if type ids are equal, we can do this "downcast" safely
        // (logic handled by the function call)
        reinterpret_cast<ComponentsTable *>(counter->full_downcast(ComponentsTable::id_func));

    if (!fulltable || entity_ref.has_expired()) {
        throw InvArg("Entity::get_fulltable: This entity reference has already "
                     "expired, or is a null reference.");
    }
    return fulltable;
}

template <typename ... Types>
template <typename T, typename U, typename ... FurtherTypes>
/* private */ Tuple<T &, U &, FurtherTypes & ...>
    Entity<Types ...>::add_impl() noexcept
{
    // throwing here is a library error, and I'm staking my honor on it
    // with a "noexcept" as a thrown exception will result in an altered
    // state of the entity (where it's not desired in that exceptional
    // case)
    if constexpr (sizeof...(FurtherTypes) > 0) {
        return tuple_cat(Tuple<T &>(add<T>()), add_impl<U, FurtherTypes...>());
    } else {
        return Tuple<T &, U &>(add<T>(), add<U>());
    }
}

template <typename ... Types>
template <typename T, typename U, typename ... FurtherTypes>
/* private */ void Entity<Types ...>::remove_impl() noexcept {
    // remove impl:
    remove<T>();
    if constexpr (sizeof...(FurtherTypes) > 0) {
        remove_impl<U, FurtherTypes...>();
    } else {
        remove<U>();
    }
}

} // end of ecs namespace
