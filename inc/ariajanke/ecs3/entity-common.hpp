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

#include <ariajanke/ecs3/defs.hpp>
#include <ariajanke/ecs3/EntityRef.hpp>

/// @file entity-common.hpp
/// Each class here defines common methods for entity types.

namespace ecs {

/// Helps define method overloads for various public methods of a constant
/// entity type.
///
/// CRTP: FullEntity must implement (as private, with this base class being a
/// friend) the following methods:
/// - const Type * cptr_<Type>() const noexcept
/// - bool is_null_() const noexcept
/// - WeakPtr<const EntityBodyBase> as_weak_cptr_() const noexcept
template <typename FullEntity>
class ConstEntityBase {
public:
    /// @returns a ConstEntityRef from this entity
    /// @note It is preferred that the client use ConstEntityRef's constructor,
    ///       however there is no reason to restrict it.
    ConstEntityRef as_constant_reference() const noexcept
        { return ConstEntityRef{ static_cast<const FullEntity *>(this)->as_weak_cptr_() }; }

    // ---------------------------------- get ---------------------------------

    /// @returns a constant reference to the requested component
    /// @throws std::runtime_error if the entity has no such component
    template <typename T>
    const T & get() const;

    /// @returns a tuple of constant references to each requested component
    /// @throws std::runtime_error if the entity is missing any component
    template <typename T, typename U, typename ... FurtherTypes>
    Tuple<const T &, const U &, const FurtherTypes & ...> get() const
        { return get_impl_(TypeList<T, U, FurtherTypes...>{}); }

    // ---------------------------------- has ---------------------------------

    /// @returns true iff the entity has every given type as a component
    /// @note client must specify at least two types, or simply use the "has"
    ///       method
    template <typename T, typename U, typename ... FurtherTypes>
    bool has_all() const noexcept { return has_all_(TypeList<T, U, FurtherTypes...>{}); }

    /// @returns true iff the entity has any of the given types as a component
    /// @note client must specify at least two types, or simply use the "has"
    ///       method
    template <typename T, typename U, typename ... FurtherTypes>
    bool has_any() const noexcept { return has_any_(TypeList<T, U, FurtherTypes...>{}); }

    /// @returns true if the entity has the specified type as a component
    template <typename T>
    bool has() const noexcept { return has_all_(TypeList<T>{}); }

    /// @returns True if the entity cannot be used to retrieve/add/remove
    ///          components.
    bool is_null() const noexcept;

    // ---------------------------------- ptr ---------------------------------

    /// @returns a pointer to the requested constant component, or a nullptr if
    ///          the entity does not have a component of that type
    template <typename T>
    const T * ptr() const noexcept { return std::get<0>(ptr_impl_(TypeList<T>{})); }

    /// @returns a tuple of pointers for the requested constant components, a
    ///          nullptr will take the place of any component which is not
    ///          present
    template <typename T, typename U, typename ... FurtherTypes>
    Tuple<const T *, const U *, const FurtherTypes * ...> ptr() const noexcept
        { return ptr_impl_(TypeList<T, U, FurtherTypes...>{}); }

    /// @brief Evalates to true if the entity has not expired.
    /// @note on semantics: often used to "evaluate true, then access components"
    /// @warning Semantic change: changed to explicit, should be contextually
    ///          convertible still. There was some issues with this method
    ///          messing with tests for equality
    explicit operator bool () const noexcept { return !is_null(); }

#   ifndef DOXYGEN_SHOULD_SKIP_THIS
protected:
    template <typename ... Types>
    bool has_all_(TypeList<Types...>) const noexcept { return true; }

    template <typename T, typename ... Types>
    bool has_all_(TypeList<T, Types...>) const noexcept;

private:
    template <typename ... Types>
    Tuple<const Types & ...> get_impl_(TypeList<Types...>) const
        { return Tuple<const Types & ...>{}; }

    template <typename Head, typename ... Types>
    Tuple<const Head &, const Types & ...> get_impl_(TypeList<Head, Types...>) const;

    template <typename ... Types>
    bool has_any_(TypeList<Types...>) const noexcept { return true; }

    template <typename T, typename ... Types>
    bool has_any_(TypeList<T, Types...>) const noexcept;

    template <typename ... Types>
    Tuple<const Types * ...> ptr_impl_(TypeList<Types...>) const noexcept
        { return Tuple<const Types * ...>{}; }

    template <typename Head, typename ... Types>
    Tuple<const Head *, const Types * ...> ptr_impl_(TypeList<Head, Types...>) const noexcept;
#   endif
};

/// Helps define method overloads for various public methods of a writable
/// entity type.
///
/// <em>CRTP</em> FullEntity must implement (make available) the following
/// methods:
///
/// - void remove_(TypeList<Types...>)
/// - Tuple<Types * ...> ptr_(TypeList<Types...>) noexcept
/// - T & add_with_args_(ArgTypes &&... args)
/// - Tuple<Types & ...> add_<Types...>(TypeList<Types...>)
///
/// This is usually done by making this class a friend of the derived class,
/// and then adding the methods as private methods. @n
/// @n
/// There are other methods which every entity is expected to implement. These
/// are typically trivial and have no overloads, therefore this class does not
/// require them. They are still required to count as a "full" Entity class.
/// - static EntityType make_sceneless_entity()
/// - EntityType make_entity()
/// - void request_deletion()
/// - auto as_const() const
/// - void swap(EntityType &)
/// - Size hash() const noexcept
/// - void set_home_scene(HomeScene &)
/// Further is must be copyable, movable, equality comparable, and be
/// constructed from EntityRefs.
template <typename FullEntity>
class EntityBase : public ConstEntityBase<FullEntity> {
public:
    // ---------------------------------- add ---------------------------------

    /// Adds a new default constructed component of type T
    /// @throws for any bad allocation, any exceptions thrown by T's default
    ///         constructor
    /// @throws std::runtime_error if a component of the given type is already
    ///         present
    /// @returns a writable reference to the newly added component
    template <typename T>
    T & add();

    /// Adds a new component of type T constructed with the given arguments
    /// passed to this function
    /// @throws for any bad allocation, any exceptions thrown by T's selected
    ///         constructor
    /// @throws std::runtime_error if a component of the given type is already
    ///         present
    /// @returns a writable reference to the newly added component
    template <typename T, typename ... ArgTypes>
    T & add(ArgTypes && ... args);

    /// Adds a set of default constructed components
    /// @note Many implementations may optimize this call by allocating space
    ///       for all components added by this method.
    /// @note if an exception is thrown no component is added to the entity
    /// @throws for any bad allocation, any exceptions thrown by any
    ///         constructor
    /// @throws std::runtime_error if a component of any given type is already
    ///         present
    /// @returns a tuple of writable references to each newly added component
    template <typename T, typename U, typename ... FurtherTypes>
    Tuple<T &, U &, FurtherTypes & ...> add();

    /// @returns a EntityRef from this entity
    /// @note It is preferred that the client use EntityRef's constructor,
    ///       however there is no reason to restrict it.
    EntityRef as_reference() const noexcept
        { return EntityRef{ static_cast<const FullEntity *>(this)->as_weak_ptr_() }; }

    // -------------------------------- ensure --------------------------------

    /// Adds a component if not already present, then "get"s it
    /// @throws for any bad allocation, any exceptions thrown by T's default
    ///         constructor (if it is called)
    /// @returns a writable reference to the component, whether old or new
    template <typename T>
    T & ensure();

    /// Adds any component that is not present, then "get"s the entire set
    /// @throws for any bad allocation, any exceptions thrown by T's default
    ///         constructor (if it is called)
    /// @note Many implementations may optimize this call by allocating space
    ///       for all components added by this method.
    /// @note if an exception is thrown no component is added to the entity
    /// @warning Under current implementation: this method generates O(n^2)
    ///          branches where n is the number of types in order to call "add"
    ///          only once
    /// @returns a tuple of references to each component
    template <typename T, typename U, typename ... FurtherTypes>
    Tuple<T &, U &, FurtherTypes & ...> ensure()
        { return ensure_impl_(TypeList<T, U, FurtherTypes...>{}); }

    // ---------------------------------- get ---------------------------------

    using ConstEntityBase<FullEntity>::get;

    /// @returns a writable reference to the requested component
    /// @throws if this entity does not have the requested component
    template <typename T>
    T & get();

    /// @returns a tuple of writable references to each requested component
    /// @throws if any requested component is not present
    template <typename T, typename U, typename ... Types>
    Tuple<T &, U &, Types & ...> get()
        { return get_impl_(TypeList<T, U, Types...>{}); }

    // ---------------------------------- ptr ---------------------------------

    using ConstEntityBase<FullEntity>::ptr;

    /// @returns a pointer to the requested writable component, or a nullptr if
    ///          the entity does not have a component of that type
    template <typename T>
    T * ptr() { return std::get<0>(ptr_impl_(TypeList<T>{})); }

    /// @returns a tuple of pointers for the requested writable components, a
    ///          nullptr will take the place of any component which is not
    ///          present
    template <typename T, typename U, typename ... FurtherTypes>
    Tuple<T *, U *, FurtherTypes * ...> ptr()
        { return ptr_impl_(TypeList<T, U, FurtherTypes...>{}); }

    // -------------------------------- remove --------------------------------

    /// Removes all components for each given type
    /// @throws if the entity is already missing any requested component, at
    ///         which point no components are removed
    template <typename T, typename ... FurtherTypes>
    void remove() { remove_<T, FurtherTypes...>(); }

    // ----------------------------- INTERFACE END ----------------------------

#ifndef DOXYGEN_SHOULD_SKIP_THIS
private:
    template <typename ... Types>
    void check_new_types(TypeList<Types...>) {}

    template <typename Head, typename ... Types>
    void check_new_types(TypeList<Head, Types...>) {
        MetaFunctions::check_if_new_component_type<Head>();
        check_new_types(TypeList<Types...>{});
    }

    FullEntity & as_fe()
        { return *static_cast<FullEntity *>(this); }

    const FullEntity & as_fe() const
        { return *static_cast<const FullEntity *>(this); }

    // ptr impls
    template <typename ... Types>
    Tuple<Types * ...> ptr_impl_(TypeList<Types...>)
        { return Tuple<Types * ...>{}; }

    template <typename Head, typename ... Types>
    Tuple<Head *, Types * ...> ptr_impl_(TypeList<Head, Types...>) {
        using std::tuple_cat;
        using std::make_tuple;
        auto ptr = as_fe().template ptr_<Head>();
        return tuple_cat(make_tuple(ptr), ptr_impl_(TypeList<Types...>{}));
    }

    // gets

    template <typename ... Types>
    Tuple<Types & ...> get_impl_(TypeList<Types...>)
        { return Tuple<Types & ...>{}; }

    template <typename Head, typename ... Types>
    Tuple<Head &, Types & ...> get_impl_(TypeList<Head, Types...>) {
        using std::tuple_cat;
        using std::make_tuple;
        auto ptr = as_fe().template ptr_<Head>();
        return tuple_cat(std::tie(*ptr), get_impl_(TypeList<Types...>{}));
    }

    // ensure

    // this may generate "too much" code
    // generates O(n^2) branches... :c where n is number of types
    // should take O(log n) branches
    template <typename ... Types>
    struct Ensurer {
        explicit Ensurer(TypeList<Types...>) {}

        template <typename ... CollectedTypes>
        void operator () (EntityBase<FullEntity> & base, TypeList<CollectedTypes...>) const {
            (void)static_cast<FullEntity &>(base).template add_(TypeList<CollectedTypes...>{});
        }
    };

    template <typename Head, typename ... Types>
    struct Ensurer<Head, Types...> {
        explicit Ensurer(TypeList<Head, Types...>) {}

        template <typename ... CollectedTypes>
        void operator () (EntityBase<FullEntity> & base, TypeList<CollectedTypes...>) const {
            Ensurer<Types...> f{TypeList<Types...>{}};
            if (base.template has<Head>()) {
                // do not collect Head type
                f(base, TypeList<CollectedTypes...>{});
            } else {
                f(base, TypeList<Head, CollectedTypes...>{});
            }
        }
    };

    template <typename ... Types>
    friend struct Ensurer;

    template <typename Head, typename ... FurtherTypes>
    Tuple<Head &, FurtherTypes & ...> ensure_impl_(TypeList<Head, FurtherTypes...>) {
        Ensurer<Head, FurtherTypes...>{TypeList<Head, FurtherTypes...>{}}(*this, TypeList<>{});
        return get_impl_(TypeList<Head, FurtherTypes...>{});
    }

    template <typename ... Types>
    void remove_() {
        if (ConstEntityBase<FullEntity>::has_all_(TypeList<Types...>{})) {
            return static_cast<FullEntity *>(this)->template remove_(TypeList<Types...>{});
        }
        throw RtError("");
    }
};

template <typename FullEntity>
template <typename T>
const T & ConstEntityBase<FullEntity>::get() const {
    static constexpr auto k_cannot_get_missing =
        "ConstEntityBase::get: cannot get missing component.";
    const auto * rv = ptr<T>();
    if (rv) return *rv;
    throw RtError(k_cannot_get_missing);
}

template <typename FullEntity>
template <typename T, typename ... Types>
bool ConstEntityBase<FullEntity>::has_all_
    (TypeList<T, Types...>) const noexcept
{
    return    static_cast<const FullEntity *>(this)->template cptr_<T>()
           && has_all_(TypeList<Types...>{});
}

template <typename FullEntity>
bool ConstEntityBase<FullEntity>::is_null() const noexcept
    { return static_cast<const FullEntity *>(this)->is_null_(); }

template <typename FullEntity>
template <typename Head, typename ... Types>
/* private */ Tuple<const Head &, const Types & ...>
    ConstEntityBase<FullEntity>::get_impl_(TypeList<Head, Types...>) const
{
    using std::tuple_cat;
    using std::make_tuple;
    auto ptr = static_cast<const FullEntity *>(this)->template cptr_<Head>();
    return tuple_cat(std::tie(*ptr), get_impl_(TypeList<Types...>{}));
}

template <typename FullEntity>
template <typename T, typename ... Types>
/* private */ bool ConstEntityBase<FullEntity>::has_any_
    (TypeList<T, Types...>) const noexcept
{
    return    static_cast<const FullEntity *>(this)->template cptr_<T>()
           || has_all_(TypeList<Types...>{});
}

template <typename FullEntity>
template <typename Head, typename ... Types>
/* private */ Tuple<const Head *, const Types * ...>
    ConstEntityBase<FullEntity>::ptr_impl_(TypeList<Head, Types...>) const noexcept
{
    using std::tuple_cat;
    using std::make_tuple;
    auto ptr = static_cast<const FullEntity *>(this)->template cptr_<Head>();
    return tuple_cat(make_tuple(ptr), ptr_impl_(TypeList<Types...>{}));
}

// --- EntityBase ---

template <typename FullEntity>
template <typename T>
T & EntityBase<FullEntity>::add() {
    check_new_types(TypeList<T>{});
    return std::get<0>(as_fe().template add_(TypeList<T>{}));
}

template <typename FullEntity>
template <typename T, typename ... ArgTypes>
T & EntityBase<FullEntity>::add(ArgTypes && ... args) {
    check_new_types(TypeList<T>{});
    return as_fe().template add_with_args_<T>(std::forward<ArgTypes>(args)...);
}

template <typename FullEntity>
template <typename T, typename U, typename ... FurtherTypes>
Tuple<T &, U &, FurtherTypes & ...> EntityBase<FullEntity>::add() {
    check_new_types(TypeList<T, U, FurtherTypes...>{});
    return as_fe().template add_(TypeList<T, U, FurtherTypes...>{});
}

template <typename FullEntity>
template <typename T>
T & EntityBase<FullEntity>::get() {
    static constexpr auto k_cannot_get_missing =
        "EntityBase::get: cannot get missing component.";
    auto * rv = ptr<T>();
    if (rv) return *rv;
    throw RtError(k_cannot_get_missing);
}

template <typename FullEntity>
template <typename T>
T & EntityBase<FullEntity>::ensure() {
    auto rv = ptr<T>();
    if (rv) return *rv;
    return std::get<0>( as_fe().template add_(TypeList<T>{}) );
}

// outside of file

template <typename FullEntity>
/* explicit */ EntityRef::EntityRef(const EntityBase<FullEntity> & base):
    EntityRef(base.as_reference()) {}

template <typename FullEntity>
/* explicit */ ConstEntityRef::ConstEntityRef
    (const ConstEntityBase<FullEntity> & base):
    ConstEntityRef(base.as_constant_reference()) {}

#endif // DOXYGEN_SHOULD_SKIP_THIS

} // end of ecs namespace
