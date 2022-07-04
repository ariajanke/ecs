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

#include <ariajanke/ecs3/SharedPtr.hpp>
#include <ariajanke/ecs3/detail/EntityRef.hpp>

#include <ariajanke/ecs3/defs.hpp>

#include <memory>
#include <atomic>
#include <vector>
#include <algorithm>

#include <cassert>

namespace ecs {

// this is probably more of a "detail"

// ----------------------------------------------------------------------------

class EntityRef final {
public:
    // EntityRef maybe converted into a regular entity or a "const" entity
    // ConstEntityRef may only be converted into a "const" entity

    // copy & move constructors should be okay?
    EntityRef() {}

    explicit EntityRef(WeakPtr<EntityBodyBase> && body_base_ptr):
        m_body_base(std::move(body_base_ptr)) {}

    // implementation not here!
    template <typename FullEntity>
    explicit EntityRef(const EntityBase<FullEntity> &);

    bool operator == (const EntityRef & rhs) const noexcept
        { return equal_to(rhs); }

    bool operator != (const EntityRef & rhs) const noexcept
        { return !equal_to(rhs); }

    Size hash() const noexcept { return m_body_base.owner_hash(); }

    bool has_expired() const noexcept
        { return m_body_base.has_expired(); }

    template <typename T>
    SharedPtr<T> get_body(Size safety) const {
        if (!m_body_base) return SharedPtr<T>{};
        return m_body_base.lock().cast_to<T>([safety] (EntityBodyBase * base)
            { return reinterpret_cast<T *>(base->downcast(safety)); });
    }

    explicit operator bool () const noexcept { return !!m_body_base; }

private:
    friend class ConstEntityRef;

    bool equal_to(const EntityRef & rhs) const noexcept
        { return m_body_base == rhs.m_body_base; }

    WeakPtr<EntityBodyBase> m_body_base;
};

// I feel this is a poor name...
class ConstEntityRef final {
public:
    // EntityRef maybe converted into a regular entity or a "const" entity
    // ConstEntityRef may only be converted into a "const" entity

    // copy & move constructors should be okay?
    ConstEntityRef() {}

    ConstEntityRef(const EntityRef & rhs):
        m_body_base(rhs.m_body_base) {}

    ConstEntityRef(EntityRef && rhs):
        m_body_base(std::move(rhs.m_body_base)) {}

    ConstEntityRef(const ConstEntityRef & rhs) = default;

    ConstEntityRef(ConstEntityRef && rhs) = default;

    explicit ConstEntityRef(WeakPtr<const EntityBodyBase> && body_base_ptr):
        m_body_base(std::move(body_base_ptr)) {}

    // should take EntityBase<> as well
    // implementation not here!
    template <typename FullEntity>
    explicit ConstEntityRef(const ConstEntityBase<FullEntity> &);

    ConstEntityRef & operator = (const ConstEntityRef & rhs) = default;

    ConstEntityRef & operator = (ConstEntityRef && rhs) = default;

    bool operator == (const ConstEntityRef & rhs) const noexcept
        { return equal_to(rhs); }

    bool operator != (const ConstEntityRef & rhs) const noexcept
        { return !equal_to(rhs); }

    Size hash() const noexcept { return m_body_base.owner_hash(); }

    bool has_expired() const noexcept
        { return m_body_base.has_expired(); }

    template <typename T>
    SharedPtr<const T> get_body(Size safety) const {
        if (!m_body_base) return SharedPtr<const T>{};
        return m_body_base.lock().cast_to<T>([safety] (const EntityBodyBase * base)
            { return reinterpret_cast<const T *>(base->downcast(safety)); });
    }

    explicit operator bool () const noexcept { return !!m_body_base; }

private:
    bool equal_to(const ConstEntityRef & rhs) const noexcept
        { return m_body_base == rhs.m_body_base; }

    WeakPtr<const EntityBodyBase> m_body_base;
};

// --------------------------- END PUBLIC INTERFACE ---------------------------

template <typename EntityType>
class EntityBodyIntr : public EntityBodyBase {
public:
    using HomeScene = HomeSceneBase<EntityType>;

    EntityBodyIntr() {}

    EntityBodyIntr(const EntityBodyIntr & body):
        m_home(body.m_home) {}

    explicit EntityBodyIntr(HomeScene * home): m_home(home) {}

    void on_create(const EntityType &) const;

    // just a dumb setter
    void set_home(HomeScene & scene) noexcept { m_home = &scene; }

    void on_deletion_request(const EntityType &) const;

    static Size get_safety() {
        struct Dummy final {};
        return MetaFunctions::key_for_type<Dummy>();
    }

private:
    HomeScene * m_home = &HomeScene::no_scene();
};

// ----------------------------------------------------------------------------

inline void * EntityBodyBase::downcast(Size safety) noexcept
    { return const_cast<void *>(downcast_(safety)); }

inline const void * EntityBodyBase::downcast(Size safety) const noexcept
    { return downcast_(safety); }

template <typename EntityType>
/* static */ HomeSceneBase<EntityType> &
    HomeSceneBase<EntityType>::no_scene()
{
    class Impl final : public HomeSceneBase<EntityType> {
    public:
        void on_create(const EntityType &) final {}
        void on_deletion_request(const EntityType &) final {}
    };
    static Impl inst;
    return inst;
}

template <typename EntityType>
void EntityBodyIntr<EntityType>::on_create(const EntityType & entity) const
    { m_home->on_create(entity); }

template <typename EntityType>
void EntityBodyIntr<EntityType>::on_deletion_request(const EntityType & entity) const
    { m_home->on_deletion_request(entity); }

} // end of ecs namespace
