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
#include <ariajanke/ecs3/detail/AvlTreeEntity.hpp>

namespace ecs {

class ConstAvlTreeEntity;

class AvlTreeEntity final : public EntityBase<AvlTreeEntity> {
public:
    using HomeScene   = HomeSceneBase<AvlTreeEntity>;
    using ConstEntity = ConstAvlTreeEntity;

    AvlTreeEntity() {}

    /// @brief Completes an entity reference, allowing client code to access
    ///        the components associated with the entity.
    explicit AvlTreeEntity(const EntityRef & ref):
        m_body(ref.get_body<AvlTreeEntityBody>(AvlTreeEntityBody::get_safety()))
    {}

    /// @brief Completes an entity reference, allowing client code to access
    ///        the components associated with the entity.
    explicit AvlTreeEntity(EntityRef && ref):
        m_body(ref.get_body<AvlTreeEntityBody>(AvlTreeEntityBody::get_safety()))
    {}

    AvlTreeEntity(const AvlTreeEntity &) = default;

    AvlTreeEntity(AvlTreeEntity &&) = default;

    static AvlTreeEntity make_sceneless_entity()
        { return AvlTreeEntity{SharedPtr<AvlTreeEntityBody>::make()}; }

    AvlTreeEntity & operator = (const AvlTreeEntity &) = default;

    AvlTreeEntity & operator = (AvlTreeEntity &&) = default;

    /// @returns True if two entities refer to the same components.
    bool operator == (const AvlTreeEntity & rhs) const { return m_body == rhs.m_body; }

    /// @returns True if two entities refer to different components.
    bool operator != (const AvlTreeEntity & rhs) const { return m_body != rhs.m_body; }

    AvlTreeEntity make_entity() const;

    ConstAvlTreeEntity as_constant() const;

    /// Requested that the refered entity be deleted by the owning manager
    /// object. Entities cannot delete themselves.
    void request_deletion()
        { m_body->on_deletion_request(*this); }

    /// Swaps component tables between two entities.
    void swap(AvlTreeEntity & rhs) { std::swap(m_body, rhs.m_body); }

    /// @note hash code cannnot be guaranteed to be unique if the code outlives
    ///       it's original entity
    /// @returns a unique hash code that identifies this entity
    Size hash() const noexcept
        { return m_body.owner_hash(); }

    /// <strong>Not intended for client use.</strong>
    /// This method sets the home scene component.
    void set_home_scene(HomeScene & home_scene)
        { m_body->set_home(home_scene); }

#   ifndef DOXYGEN_SHOULD_SKIP_THIS
private:
    friend class EntityBase<AvlTreeEntity>;
    friend class ConstEntityBase<AvlTreeEntity>;

    AvlTreeEntity(SharedPtr<AvlTreeEntityBody> && body_ptr):
        m_body(std::move(body_ptr)) {}

    template <typename ... Types>
    static Tuple<Types & ...> tuple_from_multinode
        (NodeOwningPtr * beg, NodeOwningPtr * end, TypeList<Types...>);

    template <typename Head, typename ... Types>
    static Tuple<Head &, Types & ...> tuple_from_multinode
        (NodeOwningPtr * beg, NodeOwningPtr * end, TypeList<Head, Types...>);

    // id creation fails... on ambiguous types, as multiple types may map to the same storage type...
    template <typename T, typename ... ArgTypes>
    T & add_with_args_(ArgTypes &&... args);

    bool has_expired_() const noexcept { return m_body.owners() > 0; }

    template <typename ... Types>
    void remove_(TypeList<Types...>) { /* should be noexcept */ }

    template <typename Head, typename ... Types>
    void remove_(TypeList<Head, Types...>);

    template <typename Type>
    Type * ptr_() noexcept
        { return m_body->root ? const_cast<Type *>(cptr_<Type>()) : nullptr; }

    template <typename ... Types>
    Tuple<Types & ...> add_(TypeList<Types...>);

    template <typename Type>
    const Type * cptr_() const noexcept;

    bool is_null_() const noexcept { return !m_body; }

    auto as_weak_cptr_() const noexcept
        { return WeakPtr<const EntityBodyBase>{m_body}; }

    auto as_weak_ptr_() const noexcept
        { return WeakPtr<EntityBodyBase>{m_body}; }

    SharedPtr<AvlTreeEntityBody> m_body;
#   endif
};

class ConstAvlTreeEntity final : public ConstEntityBase<ConstAvlTreeEntity> {
public:
    ConstAvlTreeEntity() {}

    explicit ConstAvlTreeEntity(const SharedPtr<const AvlTreeEntityBody> & body_ptr):
        m_body(body_ptr) {}

    /// @brief Completes an entity reference, allowing client code to access
    ///        the components associated with the entity.
    explicit ConstAvlTreeEntity(const EntityRef & ref):
        m_body(ref.get_body<const AvlTreeEntityBody>(AvlTreeEntityBody::get_safety()))
    {}

    /// @brief Completes an entity reference, allowing client code to access
    ///        the components associated with the entity.
    explicit ConstAvlTreeEntity(EntityRef && ref):
        m_body(ref.get_body<const AvlTreeEntityBody>(AvlTreeEntityBody::get_safety()))
    {}

    /// @brief Completes an entity reference, allowing client code to access
    ///        the components associated with the entity.
    explicit ConstAvlTreeEntity(const ConstEntityRef & ref):
        m_body(ref.get_body<const AvlTreeEntityBody>(AvlTreeEntityBody::get_safety()))
    {}

    /// @brief Completes an entity reference, allowing client code to access
    ///        the components associated with the entity.
    explicit ConstAvlTreeEntity(ConstEntityRef && ref):
        m_body(ref.get_body<const AvlTreeEntityBody>(AvlTreeEntityBody::get_safety()))
    {}

    /// @returns True if two entities refer to the same components.
    bool operator == (const ConstAvlTreeEntity & rhs) const { return m_body == rhs.m_body; }

    /// @returns True if two entities refer to different components.
    bool operator != (const ConstAvlTreeEntity & rhs) const { return m_body != rhs.m_body; }

private:
#   ifndef DOXYGEN_SHOULD_SKIP_THIS
    friend class ConstEntityBase<ConstAvlTreeEntity>;

    // I do not want double implementation!

    template <typename T>
    const T * cptr_() const {
        if (!m_body->root) return nullptr;
        return reinterpret_cast<T *>(m_body->root->ptr<StorageFor<T>>());
    }

    auto as_weak_cptr_() const noexcept
        { return WeakPtr<const EntityBodyBase>{m_body}; }

    bool is_null_() const noexcept { return !m_body; }

    SharedPtr<const AvlTreeEntityBody> m_body;
#   endif
};


// ------------------------------- INTERFACE END ------------------------------

#ifndef DOXYGEN_SHOULD_SKIP_THIS

inline ConstAvlTreeEntity AvlTreeEntity::as_constant() const
    { return ConstAvlTreeEntity{m_body}; }

inline AvlTreeEntity AvlTreeEntity::make_entity() const {
    auto rv = AvlTreeEntity{SharedPtr<AvlTreeEntityBody>::make(*m_body)};
    rv.m_body->on_create(rv);
    return rv;
}

template <typename ... Types>
/* private static */ Tuple<Types & ...> AvlTreeEntity::tuple_from_multinode
    (NodeOwningPtr * beg, NodeOwningPtr * end, TypeList<Types...>)
{
    assert(beg == end);
    return Tuple<>{};
}

template <typename Head, typename ... Types>
/* private static */ Tuple<Head &, Types & ...> AvlTreeEntity::tuple_from_multinode
    (NodeOwningPtr * beg, NodeOwningPtr * end, TypeList<Head, Types...>)
{
    assert(end - beg == sizeof...(Types) + 1);
    using std::tuple_cat, std::tie;
    auto ptr = (**beg).ptr<Head>();
    assert(ptr);
    return tuple_cat(tie(*ptr), tuple_from_multinode(beg + 1, end, TypeList<Types...>{}));
}

// id creation fails... on ambiguous types, as multiple types may map to the same storage type...
template <typename T, typename ... ArgTypes>
/* private */ T & AvlTreeEntity::add_with_args_(ArgTypes &&... args) {
    using std::move;
    auto node = make_single_type_node<T>(std::forward<ArgTypes>(args)...);
    auto & rv = *node->template ptr<T>();
    auto res = NodeInstance::avl_insert(move(m_body->root), move(node));
    if (res.given) {
        throw RtError("");
    }
    m_body->root = move(res.root);
    return rv;
}

template <typename Head, typename ... Types>
/* private */ void AvlTreeEntity::remove_(TypeList<Head, Types...>) {
    assert(m_body->root);
    auto key = MetaFunctions::key_for_type<Head>();
    auto [root, removed] = NodeInstance::avl_remove(move(m_body->root), key); {}
    assert(removed);
    // removed's destructor handles destructing of the datum, as it should
    m_body->root = move(root);
    remove_(TypeList<Types...>{});
}

template <typename ... Types>
/* private */ Tuple<Types & ...> AvlTreeEntity::add_(TypeList<Types...>) {
    using std::move;
    auto newnodes = make_multiple_type_nodes<Types...>();
    auto insert = [](NodeOwningPtr root, NodeOwningPtr newnode) {
        auto [nroot_, reject] = NodeInstance::avl_insert(move(root), move(newnode)); {}
        assert(!reject);
        return move(nroot_);
    };
    auto rv = tuple_from_multinode(&newnodes[0], &newnodes[0] + sizeof...(Types),
        TypeList<Types...>{});
    for (auto & node : newnodes) {
        m_body->root = insert(move(m_body->root), move(node));
    }
    return rv;
}

template <typename Type>
/* private */ const Type * AvlTreeEntity::cptr_() const noexcept {
    if (!m_body->root) return nullptr;
    // this isn't right...
    return reinterpret_cast<const Type *>(m_body->root->ptr<StorageFor<Type>>());
}

#endif // DOXYGEN_SHOULD_SKIP_THIS

} // end of ecs namespace
