/****************************************************************************

    MIT License

    Copyright (c) 2022 Aria Janke

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

#include <ariajanke/ecs3/entity-common.hpp>
#include <ariajanke/ecs3/detail/HashTableEntity.hpp>

namespace ecs {

class ConstHashTableEntity;

// some interface parts are not fully implemented!
class HashTableEntity final : public EntityBase<HashTableEntity> {
public:
    using HomeScene   = HomeSceneBase<HashTableEntity>;
    using ConstEntity = ConstHashTableEntity;

    HashTableEntity() {}

    /// @brief Completes an entity reference, allowing client code to access
    ///        the components associated with the entity.
    explicit HashTableEntity(const EntityRef & rhs):
        m_body(rhs.get_body<HashTableEntityBody>(HashTableEntityBody::get_safety()))
    {}

    /// @brief Completes an entity reference, allowing client code to access
    ///        the components associated with the entity.
    explicit HashTableEntity(EntityRef && rhs):
        // ugh... this *does* inc+dec owner counter
        m_body(rhs.get_body<HashTableEntityBody>(HashTableEntityBody::get_safety()))
    {}

    HashTableEntity(const HashTableEntity &) = default;

    HashTableEntity(HashTableEntity &&) = default;

    static HashTableEntity make_sceneless_entity() {
        HashTableEntity rv;
        rv.m_body = SharedPtr<HashTableEntityBody>::make();
        return rv;
    }

    HashTableEntity & operator = (const HashTableEntity &) = default;

    HashTableEntity & operator = (HashTableEntity &&) = default;

    /// @returns True if two entities refer to the same components.
    bool operator == (const HashTableEntity & rhs) const { return m_body == rhs.m_body; }

    /// @returns True if two entities refer to different components.
    bool operator != (const HashTableEntity & rhs) const { return m_body != rhs.m_body; }

    // explicit operator bool () const noexcept { return !is_null(); }

    HashTableEntity make_entity() const {
        HashTableEntity rv{SharedPtr<HashTableEntityBody>::make(*m_body)};
        rv.m_body->on_create(rv);
        return rv;
    }

    ConstHashTableEntity as_constant() const;

    /// Requested that the refered entity be deleted by the owning manager
    /// object. Entities cannot delete themselves.
    void request_deletion()
        { m_body->on_deletion_request(*this); }

    /// @brief Swaps component tables between two entities.
    void swap(HashTableEntity & rhs) { std::swap(m_body, rhs.m_body); }

    /// @note hash code cannnot be guaranteed to be unique if the code outlives
    ///       it's original entity
    /// @returns a unique hash code that identifies this entity
    Size hash() const noexcept
        { return m_body.owner_hash(); }

    void remove_all() { m_body->table.remove_all(); }

    void set_home_scene(HomeScene & home_scene)
        { m_body->set_home(home_scene); }

private:
    friend class EntityBase<HashTableEntity>;
    friend class ConstEntityBase<HashTableEntity>;

    explicit HashTableEntity(SharedPtr<HashTableEntityBody> && body_ptr):
        m_body(std::move(body_ptr)) {}

    template <typename T, typename ... ArgTypes>
    T & add_with_args_(ArgTypes &&... args)
        { return m_body->table.append<T>(std::forward<ArgTypes>(args)...); }

    template <typename ... Types>
    Tuple<Types & ...> add_(TypeList<Types...>)
        { return Tuple<Types & ...>{}; }

    template <typename T, typename ... Types>
    Tuple<T &, Types & ...> add_(TypeList<T, Types...> tl) {
        // do an ahead time reserve
        m_body->table.reserve_for_more(tl);
        return add_impl(tl);
    }

    template <typename T>
    T * ptr_() { return m_body->table.get<T>(); }

    template <typename T>
    const T * cptr_() const { return m_body->table.get<T>(); }

    template <typename Head, typename ... Types>
    void remove_(TypeList<Head, Types...>) {
        (void)m_body->table.remove<Head>();
        remove_(TypeList<Types...>{});
    }

    template <typename ... Types>
    void remove_(TypeList<Types...>) {}

    bool is_null_() const noexcept { return !m_body; }

    auto as_weak_ptr_() const noexcept
        { return WeakPtr<EntityBodyBase>{m_body}; }

    auto as_weak_cptr_() const noexcept
        { return WeakPtr<const EntityBodyBase>{m_body}; }

    template <typename ... Types>
    Tuple<Types & ...> add_impl(TypeList<Types...>) { return Tuple<Types & ...>{}; }

    template <typename Head, typename ... Types>
    Tuple<Head &, Types & ...> add_impl(TypeList<Head, Types...>) {
        using std::tuple_cat, std::tie;
        return tuple_cat(
            tie(m_body->table.append<Head>()),
            add_impl(TypeList<Types...>{}));
    }


    template <typename T, typename ... Types>
    Tuple<T &, Types & ...> get_impl() {
        using std::make_tuple;
        return make_tuple(get<T>());
    }

    template <typename T, typename U, typename ... Types>
    Tuple<T &, U &, Types & ...> get_impl() {
        using std::make_tuple, std::tuple_cat;
        return tuple_cat(make_tuple(get<T>()), get_impl<U, Types...>());
    }

    SharedPtr<HashTableEntityBody> m_body;
};

class ConstHashTableEntity final : public ConstEntityBase<ConstHashTableEntity> {
public:
    ConstHashTableEntity() {}

    explicit ConstHashTableEntity(const SharedPtr<const HashTableEntityBody> & body_ptr):
        m_body(body_ptr) {}

    explicit ConstHashTableEntity(const EntityRef & eref):
        m_body(eref.get_body<const HashTableEntityBody>(HashTableEntityBody::get_safety()))
    {}

    explicit ConstHashTableEntity(EntityRef && eref):
        m_body(eref.get_body<const HashTableEntityBody>(HashTableEntityBody::get_safety()))
    {}

    explicit ConstHashTableEntity(const ConstEntityRef & eref):
        m_body(eref.get_body<const HashTableEntityBody>(HashTableEntityBody::get_safety()))
    {}

    explicit ConstHashTableEntity(ConstEntityRef && eref):
        m_body(eref.get_body<const HashTableEntityBody>(HashTableEntityBody::get_safety()))
    {}

    /// @returns True if two entities refer to the same components.
    bool operator == (const ConstHashTableEntity & rhs) const { return m_body == rhs.m_body; }

    /// @returns True if two entities refer to different components.
    bool operator != (const ConstHashTableEntity & rhs) const { return m_body != rhs.m_body; }

protected:
    friend class ConstEntityBase<ConstHashTableEntity>;

    // I do not want double implementation!

    template <typename T>
    const T * cptr_() const { return m_body->table.get<T>(); }

    bool is_null_() const noexcept { return !m_body; }

    auto as_weak_cptr_() const noexcept
        { return WeakPtr<const EntityBodyBase>{m_body}; }

private:
    SharedPtr<const HashTableEntityBody> m_body;
};

// ------------------------------- INTERFACE END ------------------------------

#ifndef DOXYGEN_SHOULD_SKIP_THIS

inline ConstHashTableEntity HashTableEntity::as_constant() const
    { return ConstHashTableEntity{m_body}; }

#endif // DOXYGEN_SHOULD_SKIP_THIS

} // end of ecs namespace
