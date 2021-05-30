/****************************************************************************

    MIT License

    Copyright (c) 2020 Aria Janke

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

#include <ecs/ecsdefs.hpp>

#include <vector>
#include <unordered_map>
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

template <typename ... Types>
class Entity;

/// @brief Entity references are like weak pointers. The actual entity's life
///        time is controlled by its manager object.
///
/// This definition is designed for easy use with components. Sometimes it's
/// desirable to refer to other entities, references allow this to happen. Care
/// and proper precautions WILL be necessary if multi-threading a component
/// that has these weak references, as the manager cannot know which components
/// have references and how to stop data races. The best appoarch is to filter
/// for all required components and make the system an non-chunkable system.
/// (more on what that means later)
///
/// @note Entity references should still work without a manager, it just has
///       no one to tell in the event deletion is requested. This should not
///       result in a leak, the entity just will be useless for component
///       access.
class EntityRef final {
public:
    friend class detail::EntityRefAtt;
    using ReferenceManager = detail::ReferenceManager;
    using ReferenceCounter = detail::ReferenceCounter;

    /// @brief The default entity reference, is a null reference.
    /// As such there may not be any components or entity managers associated
    /// with it.
    EntityRef();

    /// @brief Creates a reference to an entity, from a complete entity. This
    ///        is implicit because detail is "decayed" with this call.
    /// @note This in no way creates a new entity.
    /// @see EntityRef
    template <typename ... Types>
    EntityRef(const Entity<Types...> &);

    /// @brief Creates a reference to an entity, from a complete entity. This
    ///        is implicit because detail is "decayed" with this call.
    /// @note This in no way creates a new entity.
    /// @see EntityRef
    template <typename ... Types>
    EntityRef(Entity<Types...> &&);

    /// @brief Creates a new refenece to an entity. This new reference will
    ///        point to the same set of components
    /// @note This in no way creates a new entity.
    EntityRef(const EntityRef &);

    /// @brief Creates a new refenece to an entity. This new reference will
    ///        point to the same set of components
    /// @note This in no way creates a new entity.
    EntityRef(EntityRef &&);

    ~EntityRef();

    /// Creates a new entity, returning a reference to it. Components maybe
    /// added/deleted/read/writen to when converted to a complete entity type.
    /// If no manager is provided, the new entity will not be able to have any
    /// components added to it.
    /// @return reference to new entity
    static EntityRef create_new_entity(ReferenceManager &);

    /// Creates a new entity, using the reference manager for this entity
    /// reference. Components maybe added/deleted/read/writen to when
    /// converted to a complete entity type.
    /// @return reference to new entity
    EntityRef create_new_entity();

    /// @brief Replaces the underlying reference to a particular set of
    ///        components to the given reference.
    EntityRef & operator = (const EntityRef &);

    /// @brief Replaces the underlying reference to a particular set of
    ///        components to the given reference.
    EntityRef & operator = (EntityRef &&);

    /// @brief Causes this reference to refer to another entity, which may
    ///       refer to a different set of components.
    template <typename ... Types>
    EntityRef & operator = (const Entity<Types...> &);

    /// @brief Causes this reference to refer to another entity, which may
    ///       refer to a different set of components.
    template <typename ... Types>
    EntityRef & operator = (Entity<Types...> &&);

    /// @returns True if both reference refer to the same set of components.
    bool operator == (const EntityRef &) const;

    /// @returns True if both reference refer to a different set of components.
    bool operator != (const EntityRef &) const;

    /// An expired reference is something which points to an entity that is no
    /// longer there.
    /// @return true if the entity has expired
    bool has_expired() const { return m_identity ? m_identity->expired : true; }

    /// Requested that the refered entity be deleted by the owning manager
    /// object. Entities cannot delete themselves.
    void request_deletion() { m_identity->requesting_deletion = true; }

    /// @returns true if the referred entity is requesting deletion
    /// @note An entity which is requesting deletion is not an expired entity.
    ///       (yet!)
    bool is_requesting_deletion() const { return m_identity->requesting_deletion; }

    void swap(EntityRef &);

    void swap(EntityRef &&);

    /// @note hash code cannnot be guaranteed to be unique if the code outlives
    ///       it's original entity
    /// @returns a unique hash code that identifies this entity reference
    std::size_t hash() const { return reinterpret_cast<std::size_t>(m_identity); }

    /// @brief Evalates to true if this refers to an entity with accessible
    ///        components.
    /// @note on semantic issue, entity being used like weak_ptr
    operator bool () const { return m_identity ? !m_identity->expired : false; }

private:
    ReferenceCounter * m_identity = nullptr;
};

// ----------------------------------------------------------------------------

template <typename ... Types>
class System;
#if 0
template <typename ... Types>
class DefineDefineTupleSystem;
#endif
template <typename TeType>
struct Optional {
    using Type           = TeType;
    using AsPointer      = Type *;
    using AsConstPointer = const Type *;
};

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
#   if 0
    using DefineTupleSystem = DefineDefineTupleSystem<Types...>;
#   endif

    /// Component table's size in bytes
    static constexpr const std::size_t k_component_table_size =
        sizeof(detail::ComponentTable<Types...>);

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
    static Entity create_new_entity(detail::ReferenceManager &);

    /// @brief Creates a new entity using the refenece manager of this entity.
    /// @returns An entirely new entity, which will have its own set of
    ///          components.
    Entity create_new_entity() const;

    /// @tparam T type of component to get
    /// @returns true if the component is present
    template <typename T>
    bool has() const { return (ptr<T>() != nullptr); }

    /// @brief Gets a writable component by type.
    /// @tparam T type of component to get
    /// @throws if the requested component has not been added
    template <typename T>
    T & get();

    /// @brief Gets a read only component by type.
    /// @tparam T type of component to get
    /// @throws if the requested component has not been added
    template <typename T>
    const T & get() const;

    /// @brief Gets a pointer to a writable component by type.
    /// @tparam T type of component to get
    /// @returns a nullptr if the component is not present
    template <typename T>
    T * ptr() { return m_table->template get_ptr<T>(); }

    /// @brief Gets a pointer to a read only component by type.
    /// @tparam T type of component to get
    /// @returns a nullptr if the component is not present
    template <typename T>
    const T * ptr() const { return m_table->template get_ptr<T>(); }

    /// @brief Adds a new component by type.
    /// @tparam T type of component to get
    /// @throws if component of type T is already present
    template <typename T>
    T & add() { return m_table->template add<T>(); }

    /// @brief Gets a writable reference to a component by type, if it is not
    /// present, it is added.
    /// @tparam T type of component to get
    template <typename T>
    T & ensure() { return ptr<T>() ? get<T>() : add<T>(); }

    /// @brief Removes a component by type.
    /// @tparam T type of component to remove
    /// @throws if the component is not present
    template <typename T>
    void remove() { m_table->template remove<T>(); }

    /// Requested that the refered entity be deleted by the owning manager
    /// object. Entities cannot delete themselves.
    void request_deletion() { m_table->requesting_deletion = true; }

    /// @returns true if the entity is requesting deletion
    /// @note An entity which is requesting deletion is not an expired entity.
    ///       (yet!)
    bool is_requesting_deletion() const { return m_table->requesting_deletion; }

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

    using ComponentsTable = detail::ComponentTableHead<Types...>;

    /// @throws  If the component table associated with the reference is of a
    ///          difference type than this Entity type. It may also throw
    ///          the given entity reference is empty.
    /// @returns The full component table from a given entity reference. This
    ///          function will never return nullptr
    static ComponentsTable * get_fulltable(const EntityRef &);

    ComponentsTable * m_table = nullptr;
};

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

/// Unique to system component defines a managed way to add a new component type
/// to an entity that is unique to the inheriter.
template <typename T>
class UniqueToSystemComponent : public detail::UscDelNotifier {
    using UscContainer = std::unordered_map<std::size_t, T>;
protected:
    template <bool k_is_const>
    class IterImpl {
    public:
        using ReferenceType = typename std::conditional<k_is_const, const T &, T &>::type;
        using PointerType   = typename std::conditional<k_is_const, const T *, T *>::type;
        using UnderIter     = typename std::conditional<k_is_const, typename UscContainer::const_iterator, typename UscContainer::iterator>::type;
        IterImpl() {}
        IterImpl(UnderIter uitr): m(uitr) {}
        void operator ++ () { ++m; }
        ReferenceType operator * () const { return m->second; }
        PointerType operator -> () const { return &m->second; }
        bool operator != (const IterImpl & itr) const { return m != itr.m; }
    private:
        UnderIter m;
    };

    using ComponentIterator      = IterImpl <false>;
    using ComponentConstIterator = IterImpl <true >;
    using Range                  = detail::Range<ComponentIterator>;
    using ConstRange             = detail::Range<ComponentConstIterator>;

    UniqueToSystemComponent() {}
    UniqueToSystemComponent(const UniqueToSystemComponent &) = delete;
    ~UniqueToSystemComponent() override;

    UniqueToSystemComponent & operator = (const UniqueToSystemComponent &) = delete;

    T & ucomponent_for(const EntityRef & er);

    const T & ucomponent_for(const EntityRef & er) const;

    bool has_ucomponent(const EntityRef & er) const
        { return m_compdb.find(er.hash()) != m_compdb.end(); }

    T & add_ucomponent(const EntityRef & er);

    void remove_ucomponent(const EntityRef & er);

    Range range_for_ucomponents()
        { return Range(m_compdb.begin(), m_compdb.end()); }

    ConstRange range_for_ucomponents() const
        { return ConstRange(m_compdb.begin(), m_compdb.end()); }

private:
    static constexpr const char * const k_comp_not_found_msg =
        ": this component was not found and therefore cannot be ";
    void notify_deletion(std::size_t k) final
        { m_compdb.erase(k); }
    UscContainer m_compdb;
};

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

    EntityManager() {}
    EntityManager(const EntityManager &) = delete;
    EntityManager & operator = (const EntityManager &) = delete;
    ~EntityManager();

    /// Creates an entirely new entity pointing to a different set of
    /// components.
    EntityType create_new_entity();

    /// Registers a system for use by the manager. Any "Unique to system"
    /// component databases should also be fed to this member function.
    ///
    /// @note Does not in anyway take ownership of the system. All systems must
    ///       outlive the manager object.
    ///
    /// @throws if nullptr is provided
    void register_system(SystemType *);

    /// Removes all systems currently in use by the manager.
    void clear_systems_list();

    /// Runs update calls to all registered systems.
    void update_systems();

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

private:
    detail::ReferenceCounter * provide_new_identity() override;

    void append_new_entities();

    std::vector<EntityType> m_unfiltered_entities;
    std::vector<EntityType> m_new_entities;
    std::vector<SystemType *> m_systems;
    std::vector<detail::UscDelNotifier *> m_ucsystems;
};

// -------------------------- end public interface ------------------------- //

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

// ----------------------------------------------------------------------------

template <typename ... Types>
EntityRef::EntityRef(const Entity<Types...> & rhs):
    m_identity(rhs.m_table)
{ detail::increment(m_identity); }


template <typename ... Types>
EntityRef::EntityRef(Entity<Types...> && rhs)
    { swap(rhs); }

template <typename ... Types>
EntityRef & EntityRef::operator = (const Entity<Types...> & rhs) {
    EntityRef temp(rhs);
    swap(rhs);
    return *this;
}

template <typename ... Types>
EntityRef & EntityRef::operator = (Entity<Types...> && rhs) {
    detail::decrement(m_identity);
    m_identity = rhs.m_table;
    detail::increment(m_identity);
    return *this;
}

// ----------------------------------------------------------------------------

template <typename ... Types>
/* implicit */ Entity<Types ...>::Entity(const EntityRef & eref)
{
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
{
    using RtError = std::runtime_error;
    auto * counter = manager.provide_new_identity();
    if (!counter) {
        throw RtError("Entity<Types...>::create_new_entity(): failed to create "
                      "new entity.");
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
Entity<Types ...> Entity<Types ...>::create_new_entity() const {
    if (!m_table) {
        throw std::runtime_error("Entity::create_new_entity(): uninitialized entities cannot create new entities.");
    }
    return Entity<Types ...>::create_new_entity(*m_table->reference_manager);
}

template <typename ... Types>
template <typename T>
T & Entity<Types ...>::get() {
    if (auto * rv = ptr<T>()) return *rv;
    throw std::runtime_error(k_comp_not_present_msg);
}

template <typename ... Types>
template <typename T>
const T & Entity<Types ...>::get() const {
    if (const auto * rv = ptr<T>()) return *rv;
    throw std::runtime_error(k_comp_not_present_msg);
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

// ----------------------------------------------------------------------------

template <typename T>
UniqueToSystemComponent<T>::~UniqueToSystemComponent() {}

template <typename T>
T & UniqueToSystemComponent<T>::ucomponent_for(const EntityRef & er) {
    const auto & cthis = *this;
    return *const_cast<T *>(&cthis.ucomponent_for(er));
}

template <typename T>
const T & UniqueToSystemComponent<T>::ucomponent_for(const EntityRef & er) const {
    auto itr = m_compdb.find(er.hash());
    if (itr == m_compdb.end()) {
        throw std::runtime_error(std::string("UniqueToSystemComponent<T>::ucomponent_for")
                                 + k_comp_not_found_msg + "gotten.");
    }
    return itr->second;
}

template <typename T>
T & UniqueToSystemComponent<T>::add_ucomponent(const EntityRef & er) {
    if (has_ucomponent(er)) {
        throw std::runtime_error("UniqueToSystemComponent<T>::add_ucomponent: "
                                 "could not add component which is already present.");
    }
    return m_compdb[er.hash()];
}

template <typename T>
void UniqueToSystemComponent<T>::remove_ucomponent(const EntityRef & er) {
    auto itr = m_compdb.find(er.hash());
    if (itr == m_compdb.end()) {
        throw std::runtime_error(std::string("UniqueToSystemComponent<T>::remove_ucomponent")
                                 + k_comp_not_found_msg + "removed.");
    }
    m_compdb.erase(itr);
}

// ----------------------------------------------------------------------------

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
{ return EntityType::create_new_entity(*this); }

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
void EntityManager<Types...>::process_deletion_requests() {
    using namespace detail;
    append_new_entities();
    for (auto & ent : m_unfiltered_entities) {
        if (!EntityRefAtt::is_requesting_deletion(ent)) continue;
        for (auto * ucs : m_ucsystems) {
            ucs->notify_deletion(ent);
        }

        EntityAtt::expire_entity(ent);
    }
    auto del_beg = std::remove_if(m_unfiltered_entities.begin(), m_unfiltered_entities.end(),
                                  EntityRefAtt::is_requesting_deletion);
    m_unfiltered_entities.erase(del_beg, m_unfiltered_entities.end());
}

template <typename ... Types>
detail::ReferenceCounter * EntityManager<Types...>::provide_new_identity() {
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

        ReferenceCounter * provide_new_identity() override {
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

inline ReferenceCounter::~ReferenceCounter() {}

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

// -----------------------------------------------------------------------------

inline ReferenceManager::~ReferenceManager() {}

// -----------------------------------------------------------------------------

inline UscDelNotifier::~UscDelNotifier() {}

} // end of detail namespace

inline EntityRef::EntityRef() {}

inline EntityRef::EntityRef(const EntityRef & rhs):
    m_identity(rhs.m_identity)
    { detail::increment(m_identity); }

inline EntityRef::EntityRef(EntityRef && rhs)
    { swap(rhs); }

inline EntityRef::~EntityRef()
    { detail::decrement(m_identity); }

inline /* static */ EntityRef EntityRef::create_new_entity
    (ReferenceManager & reference_manager)
{
    EntityRef er;
    er.m_identity = reference_manager.provide_new_identity();
    return er;
}

inline EntityRef EntityRef::create_new_entity() {
    if (!m_identity) {
        throw std::runtime_error("EntityRef::create_new_entity(): cannot "
                                 "create a new entity reference from a null "
                                 "reference.");
    }
    return EntityRef::create_new_entity(*m_identity->reference_manager);
}

inline EntityRef & EntityRef::operator = (const EntityRef & rhs) {
    EntityRef temp(rhs);
    swap(temp);
    return *this;
}

inline EntityRef & EntityRef::operator = (EntityRef && rhs) {
    swap(rhs);
    return *this;
}

inline bool EntityRef::operator == (const EntityRef & rhs) const
    { return m_identity == rhs.m_identity; }

inline bool EntityRef::operator != (const EntityRef & rhs) const
    { return m_identity != rhs.m_identity; }

inline void EntityRef::swap(EntityRef & rhs)
    { std::swap(m_identity, rhs.m_identity); }

inline void EntityRef::swap(EntityRef && rhs)
    { std::swap(m_identity, rhs.m_identity); }

} // end of ecs namespace
