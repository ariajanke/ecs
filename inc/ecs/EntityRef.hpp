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

#include <ecs/ecsdefs.hpp>

namespace ecs {

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
    EntityRef() {}

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

/// Utiltiy structure that can be used for hash maps, essentially a definition
/// in the style of "std::hash".
struct EntityHasher {
    std::size_t operator () (const EntityRef & eref) const
        { return eref.hash(); }
};

// ----------------------------------------------------------------------------

template <typename ... Types>
EntityRef::EntityRef(const Entity<Types...> & rhs):
    m_identity(rhs.m_table)
{ detail::increment(m_identity); }

template <typename ... Types>
EntityRef::EntityRef(Entity<Types...> && rhs) {
    EntityRef eref_copy(/* as lvalue */ rhs);
    swap(eref_copy);
}

inline EntityRef::EntityRef(const EntityRef & rhs):
    m_identity(rhs.m_identity)
    { detail::increment(m_identity); }

inline EntityRef::EntityRef(EntityRef && rhs) { swap(rhs); }

inline EntityRef::~EntityRef() { detail::decrement(m_identity); }

inline EntityRef & EntityRef::operator = (const EntityRef & rhs) {
    EntityRef temp(rhs);
    swap(temp);
    return *this;
}

inline EntityRef & EntityRef::operator = (EntityRef && rhs) {
    swap(rhs);
    return *this;
}

template <typename ... Types>
EntityRef & EntityRef::operator = (const Entity<Types...> & rhs) {
    EntityRef temp(rhs);
    std::swap(temp.m_identity, m_identity);
    return *this;
}

template <typename ... Types>
EntityRef & EntityRef::operator = (Entity<Types...> && rhs) {
    detail::decrement(m_identity);
    m_identity = rhs.m_table;
    detail::increment(m_identity);
    return *this;
}

inline bool EntityRef::operator == (const EntityRef & rhs) const
    { return m_identity == rhs.m_identity; }

inline bool EntityRef::operator != (const EntityRef & rhs) const
    { return m_identity != rhs.m_identity; }

inline void EntityRef::swap(EntityRef & rhs)
    { std::swap(m_identity, rhs.m_identity); }

} // end of ecs namespace
