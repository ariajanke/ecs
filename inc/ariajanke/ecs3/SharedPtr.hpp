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

#include <ariajanke/ecs3/detail/SharedPtr.hpp>

/// @file SharedPtr.hpp
///
/// Some commentary on having to write this: @n
/// Having to reimplement this has been painful. Here's some insight into the
/// process. I needed a shared_ptr/weak_ptr facilities that offers hashing. The
/// reason I need hashing is simple: it has always been a part of entity's
/// interface, and a feature I consider important, especially in replacing the
/// old implementation. The standard library does not have a hash function for
/// both shared/weak pointer types. Apparently this was a planned feature that
/// was scrapped due to time constraints (by SO post somewhere). Now there are
/// a few implementations online. Both of which I cannot use. One, the boost
/// library: in order to use I need to seperate shared_ptr, then include
/// <em>hundreds</em> of other extra headers in my library! The other was a
/// minimalistic single header library, sounds promising right? Well I can't
/// use that either, as there are no weak pointers. I need weak pointers since
/// "entity ref" types may not own the entity's components. @n
///
/// So what am I left to do?... rewrite it myself. And thus this abomination
/// you see before you. There are a battery of unit tests which should ensure
/// that it is at least working well enough for this library.

namespace ecs {

/// A shared pointer has shared owner ship of an object
///
/// Unlike the STL this smart pointer is meant to provide a larger interface
/// for greater flexibility and additional feature not available.
template <typename T>
class SharedPtr final {
#   ifndef DOXYGEN_SHOULD_SKIP_THIS
    struct Dummy final {};
    friend class SwPtrAttn;
    using Attn = SwPtrAttn;
#   endif
public:
    /// owned object's type
    using Element = std::remove_const_t<T>;

    /// true if the object is constant
    static constexpr const bool k_is_const_type = !std::is_same_v<T, Element>;

#   ifndef DOXYGEN_SHOULD_SKIP_THIS
    using EnableConstPtr = std::conditional_t<k_is_const_type, SharedPtr<const Element>, Dummy>;

    template <typename U>
    using EnableOtherPtr = std::conditional_t<!std::is_same_v<U, Element>, SharedPtr<U>, Dummy>;

    template <typename U>
    using EnableConstOtherPtr = std::conditional_t<k_is_const_type && !std::is_same_v<U, Element>, SharedPtr<const U>, Dummy>;
#   endif

    SharedPtr() {}

    template <typename U>
    explicit SharedPtr(const SharedPtr<U> & rhs):
        SharedPtr(rhs.template cast_to<T>([] (U * up) { return static_cast<T *>(up); } ))
    {}

    /* implicit */ SharedPtr(const SharedPtr<Element> & rhs);

    /* implicit */ SharedPtr(const EnableConstPtr & rhs);

    /* implicit */ SharedPtr(SharedPtr<Element> && rhs);

    /* implicit */ SharedPtr(EnableConstPtr && rhs);

    ~SharedPtr();

    template <typename ... ArgTypes>
    static SharedPtr make(ArgTypes && ... args);

    /// Creates a vector of shared pointers
    ///
    /// @note
    /// The advantage of using this function is simple: sometimes you want a
    /// whole bunch of shared pointers all at once. This function allows you
    /// to do this using a fixed O(1) number of allocations rather than O(n).
    /// The costs are as follows:
    ///
    /// - an additional pointer per object
    /// - an additional decrement per object deletion
    /// - overall hit on SharedPtr, using a virtual method call when an object
    ///   is deleted
    ///
    /// @param size number of shared pointers to make
    /// @param constructor_args each object is constructed using these
    ///                         arguments
    template <typename ... ArgTypes>
    static std::vector<SharedPtr> vector_make
        (int size, ArgTypes && ... constructor_args);

    SharedPtr & operator = (const SharedPtr & rhs);

    SharedPtr & operator = (SharedPtr && rhs);

    Size owner_hash() const noexcept
        { return RefCounterPtrHash{}(m_ref); }

    T * operator -> () const
        { return verify_object_not_null("operator ->"); }

    T & operator * () const
        { return *verify_object_not_null("operator *"); }

    T * get() const
        { return verify_object_not_null("operator ->"); }

    void swap(SharedPtr & rhs) noexcept;

    /// performs a static_cast on the underlying pointer
    template <typename U, typename Func>
    SharedPtr<U> cast_to(Func && f) const;

    /// dynamically casts the shared pointer to another type
    /// @returns shared ptr to an object to a different type, nullptr if the
    ///          dynamic cast fails
    template <typename U>
    SharedPtr<U> dynamically_cast_to() const
        { return cast_to<U>([] (T * tp) { return dynamic_cast<U *>(tp); }); }

    explicit operator bool () const noexcept { return m_ptr; }

    /// @returns number of observing weak pointers
    int observers() const noexcept { return m_ref ? int(m_ref->observers) : 0; }

    /// @returns number of owners of the object
    int owners() const noexcept { return m_ref ? int(m_ref->owners) : 0; }

    /// true if this and rhs are the same pointer
    bool operator == (const SharedPtr<T> & rhs) const noexcept
        { return equal_to(rhs); }

    /// true if this and rhs are different pointers
    bool operator != (const SharedPtr<T> & rhs) const noexcept
        { return !equal_to(rhs); }

#   ifndef DOXYGEN_SHOULD_SKIP_THIS
private:
    using RefCounter = SwPtrPriv::RefCounter;
    using RefCounterPtrHash = SwPtrPriv::RefCounterPtrHash;

    template <typename U>
    friend class WeakPtr;

    explicit SharedPtr(RefCounter * refc, T * ptr): m_ptr(ptr), m_ref(refc) {}

    T * verify_object_not_null(const char * caller) const;

    bool equal_to(const SharedPtr<T> & rhs) const noexcept
        { return m_ref == rhs.m_ref; }

    T * m_ptr = nullptr;
    RefCounter * m_ref = nullptr;
#   endif
};

template <typename T>
class WeakPtr final {
#   ifndef DOXYGEN_SHOULD_SKIP_THIS
    struct Dummy final {};
    friend class SwPtrAttn;
    using Attn = SwPtrAttn;
#   endif
public:
    using Element = std::remove_const_t<T>;

    static constexpr const bool k_is_const_type = !std::is_same_v<T, Element>;

    using EnableConstShrPtr = std::conditional_t<k_is_const_type, SharedPtr<const Element>, Dummy>;

    using EnableConstWkPtr =  std::conditional_t<k_is_const_type, WeakPtr<const Element>, Dummy>;

    WeakPtr() {}

    /* implicit */ WeakPtr(const WeakPtr<Element> & rhs);

    template <typename U>
    explicit WeakPtr(const SharedPtr<U> & rhs);

    /* implicit */ WeakPtr(const EnableConstWkPtr & rhs);

    /* implicit */ WeakPtr(WeakPtr<Element> && rhs);

    /* implicit */ WeakPtr(EnableConstWkPtr && rhs);

    ~WeakPtr();

    WeakPtr & operator = (const WeakPtr & rhs);

    WeakPtr & operator = (WeakPtr && rhs);

    Size owner_hash() const noexcept
        { return RefCounterPtrHash{}(m_ref); }

    SharedPtr<T> lock() const;

    void swap(WeakPtr & rhs) noexcept;

    bool has_expired() const noexcept
        { return m_ref->owners == 0; }

    explicit operator bool () const noexcept { return m_ptr; }

    int observers() const noexcept { return m_ref ? int(m_ref->observers) : 0; }

    int owners() const noexcept { return m_ref ? int(m_ref->owners) : 0; }

    bool operator == (const WeakPtr<T> & rhs) const noexcept
        { return equal_to(rhs); }

    bool operator != (const WeakPtr<T> & rhs) const noexcept
        { return !equal_to(rhs); }

private:
#   ifndef DOXYGEN_SHOULD_SKIP_THIS
    using RefCounter = SwPtrPriv::RefCounter;
    using RefCounterPtrHash = SwPtrPriv::RefCounterPtrHash;

    bool equal_to(const WeakPtr<T> & rhs) const noexcept
        { return m_ref == rhs.m_ref; }

    T * m_ptr = nullptr;
    RefCounter * m_ref = nullptr;
#   endif
};

// -------------------------- Implementation Details --------------------------

#ifndef DOXYGEN_SHOULD_SKIP_THIS

class SwPtrAttn final {
    using RefCounter = SwPtrPriv::RefCounter;

    template <typename T>
    friend class WeakPtr;

    template <typename T>
    friend class SharedPtr;

    template <typename T>
    using ParentVectorHolder = SwPtrPriv::ParentVectorHolder<T>;

    template <typename T>
    using VectorObjectHolder = SwPtrPriv::VectorObjectHolder<T>;

    template <typename T>
    static RefCounter * get_counter(const SharedPtr<T> &);

    template <typename T>
    static T * get_pointer(const SharedPtr<T> &);

    template <typename T>
    static void set_counter(SharedPtr<T> &, RefCounter *);

    template <typename T>
    static void set_pointer(SharedPtr<T> &, T *);

    template <typename T>
    static SharedPtr<T> construct(RefCounter * refc, T * ptr);

    template <typename T>
    static RefCounter * get_counter(const WeakPtr<T> &);

    template <typename T>
    static T * get_pointer(const WeakPtr<T> &);

    template <typename T>
    static void set_counter(WeakPtr<T> &, RefCounter *);

    template <typename T>
    static void set_pointer(WeakPtr<T> &, T *);

    static void inc_observers(RefCounter * ref) noexcept {
        if (ref)
            { ++ref->observers; }
    }

    static void inc_owners(RefCounter * ref) noexcept {
        if (ref)
            { ++ref->owners; }
    }
};

// ----------------------------------------------------------------------------

template <typename T>
/* static */ SwPtrPriv::RefCounter *
    SwPtrAttn::get_counter(const SharedPtr<T> & sptr)
{ return sptr.m_ref; }

template <typename T>
/* static */ T * SwPtrAttn::get_pointer(const SharedPtr<T> & sptr)
    { return sptr.m_ptr; }

template <typename T>
/* static */ void SwPtrAttn::set_counter
    (SharedPtr<T> & sptr, RefCounter * counter)
{ sptr.m_ref = counter; }

template <typename T>
/* static */ void SwPtrAttn::set_pointer
    (SharedPtr<T> & sptr, T * ptr)
{ sptr.m_ptr = ptr; }

template <typename T>
/* static */ SharedPtr<T> SwPtrAttn::construct
    (RefCounter * refc, T * ptr)
{ return SharedPtr<T>{refc, ptr}; }

template <typename T>
/* static */ SwPtrPriv::RefCounter * SwPtrAttn::get_counter
    (const WeakPtr<T> & wptr)
{ return wptr.m_ref; }

template <typename T>
/* static */ T * SwPtrAttn::get_pointer(const WeakPtr<T> & wptr)
    { return wptr.m_ptr; }

template <typename T>
/* static */ void SwPtrAttn::set_counter
    (WeakPtr<T> & wptr, SwPtrPriv::RefCounter * ref)
{ wptr.m_ref = ref; }

template <typename T>
/* static */ void SwPtrAttn::set_pointer(WeakPtr<T> & wptr, T * ptr)
{ wptr.m_ptr = ptr; }

// ----------------------------------------------------------------------------

template <typename T>
/* implicit */ SharedPtr<T>::SharedPtr(const SharedPtr<Element> & rhs):
    SharedPtr(Attn::get_counter(rhs), Attn::get_pointer(rhs))
{ Attn::inc_owners(m_ref); }

template <typename T>
/* implicit */ SharedPtr<T>::SharedPtr(const EnableConstPtr & rhs):
    SharedPtr(Attn::get_counter(rhs), Attn::get_pointer(rhs))
{ Attn::inc_owners(m_ref); }

template <typename T>
/* implicit */ SharedPtr<T>::SharedPtr(SharedPtr<Element> && rhs):
    m_ptr(Attn::get_pointer(rhs)),
    m_ref(Attn::get_counter(rhs))
{
    Attn::set_pointer<Element>(rhs, nullptr);
    Attn::set_counter<Element>(rhs, nullptr);
}

template <typename T>
/* implicit */ SharedPtr<T>::SharedPtr(EnableConstPtr && rhs):
    m_ptr(Attn::get_pointer(rhs)),
    m_ref(Attn::get_counter(rhs))
{
    Attn::set_pointer<const Element>(rhs, nullptr);
    Attn::set_counter<const Element>(rhs, nullptr);
}

template <typename T>
SharedPtr<T>::~SharedPtr() {
    if (!m_ref) return;
    if (m_ref->owners == 1) {
        // when an object runs out of owners, it must be deleted
        m_ptr->~T();
        if (m_ref->observers == 0) {
            // m_ref may go bad after delete_this();
            m_ref->delete_this();
            return;
        }
    }
    --m_ref->owners;
}

template <typename T>
template <typename ... ArgTypes>
/* static */ SharedPtr<T> SharedPtr<T>::make(ArgTypes && ... args) {
    struct Impl final : public RefCounter {
        void delete_this() final
            { return delete this; }

        StorageFor<T> storage;
    };
    auto counter_and_storage = new Impl{};
    T * ptr = nullptr;
    try {
        ptr = new (&counter_and_storage->storage)
            T{std::forward<ArgTypes>(args)...};
    } catch (...) {
        delete counter_and_storage;
        throw;
    }

    auto rv = SharedPtr{static_cast<RefCounter *>(counter_and_storage), ptr};
    Attn::inc_owners(counter_and_storage);
    return rv;
}

template <typename T>
template <typename ... ArgTypes>
/* static */ std::vector<SharedPtr<T>>
    SharedPtr<T>::vector_make
    (int size, ArgTypes && ... constructor_args)
{
    using ParentHolder = Attn::ParentVectorHolder<T>;
    using ObjectHolder = Attn::VectorObjectHolder<T>;

    auto parent_holder = new ParentHolder{};
    try {
        std::vector<SharedPtr<T>> rv;
        rv.reserve(size);
        parent_holder->counter_and_storages.resize
            (size, ObjectHolder{parent_holder});
        parent_holder->counter = size;
        for (auto & counter_and_storage : parent_holder->counter_and_storages) {
            // arguments must be copied, and not moved
            rv.emplace_back(SharedPtr<T>{
                static_cast<RefCounter *>(&counter_and_storage),
                new (&counter_and_storage.storage) T{constructor_args...}});
            Attn::inc_owners(&counter_and_storage);
        }
        return rv;
    } catch (...) {
        delete parent_holder;
        throw;
    }
}

template <typename T>
SharedPtr<T> & SharedPtr<T>::operator = (const SharedPtr & rhs) {
    if (this != &rhs) {
        SharedPtr t{rhs};
        swap(t);
    }
    return *this;
}

template <typename T>
SharedPtr<T> & SharedPtr<T>::operator = (SharedPtr && rhs) {
    if (this != &rhs)
        { swap(rhs); }
    return *this;
}

template <typename T>
void SharedPtr<T>::swap(SharedPtr & rhs) noexcept {
    using std::swap;
    swap(m_ptr, rhs.m_ptr);
    swap(m_ref, rhs.m_ref);
}

template <typename T>
template <typename U, typename Func>
SharedPtr<U> SharedPtr<T>::cast_to(Func && f) const {
    auto cpptr = m_ptr;
    U * convptr = f(cpptr);
    Attn::inc_owners(m_ref);
    return Attn::construct<U>(m_ref, convptr);
}

template <typename T>
/* private */ T * SharedPtr<T>::verify_object_not_null
    (const char * caller) const
{
    if (m_ptr) return m_ptr;
    throw RtError{"SharedPtr::" + std::string{caller} + ":cannot use on null."};
}

// ----------------------------------------------------------------------------

template <typename T>
/* implicit */ WeakPtr<T>::WeakPtr(const WeakPtr<Element> & rhs):
    m_ptr(Attn::get_pointer(rhs)),
    m_ref(Attn::get_counter(rhs))
{ Attn::inc_observers(m_ref); }

template <typename T>
template <typename U>
/* explicit */ WeakPtr<T>::WeakPtr(const SharedPtr<U> & rhs):
    m_ptr(Attn::get_pointer(rhs)),
    m_ref(Attn::get_counter(rhs))
{ Attn::inc_observers(m_ref); }

template <typename T>
/* implicit */ WeakPtr<T>::WeakPtr(const EnableConstWkPtr & rhs):
    m_ptr(Attn::get_pointer(rhs)),
    m_ref(Attn::get_counter(rhs))
{ Attn::inc_observers(m_ref); }

template <typename T>
/* implicit */ WeakPtr<T>::WeakPtr(WeakPtr<Element> && rhs):
    m_ptr(Attn::get_pointer(rhs)),
    m_ref(Attn::get_counter(rhs))
{
    Attn::set_pointer<Element>(rhs, nullptr);
    Attn::set_counter<Element>(rhs, nullptr);
}

template <typename T>
/* implicit */ WeakPtr<T>::WeakPtr(EnableConstWkPtr && rhs):
    m_ptr(Attn::get_pointer(rhs)),
    m_ref(Attn::get_counter(rhs))
{
    Attn::set_pointer<const Element>(rhs, nullptr);
    Attn::set_counter<const Element>(rhs, nullptr);
}

template <typename T>
WeakPtr<T>::~WeakPtr() {
    if (!m_ref) return;
    if (m_ref->observers == 1 && m_ref->owners == 0) {
        m_ref->delete_this();
        return;
    }
    --m_ref->observers;
}

template <typename T>
WeakPtr<T> & WeakPtr<T>::operator = (const WeakPtr & rhs) {
    if (this != &rhs) {
        WeakPtr t{rhs};
        swap(t);
    }
    return *this;
}

template <typename T>
WeakPtr<T> & WeakPtr<T>::operator = (WeakPtr && rhs) {
    if (this != &rhs)
        { swap(rhs); }
    return *this;
}

template <typename T>
SharedPtr<T> WeakPtr<T>::lock() const {
    if (has_expired()) {
        throw RtError{"WeakPtr::lock: cannot lock expired pointer."};
    }
    ++m_ref->owners;
    return SharedPtr<T>{m_ref, m_ptr};
}

template <typename T>
void WeakPtr<T>::swap(WeakPtr & rhs) noexcept {
    using std::swap;
    std::swap(m_ptr, rhs.m_ptr);
    std::swap(m_ref, rhs.m_ref);
}

#endif

} // end of ecs namespace
