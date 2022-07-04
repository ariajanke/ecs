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

/// @file defs.hpp
/// Contains basic definitions and configurations for the library. Since it's
/// a "header only" library, it should be easier to modify. I'm not sure how to
/// get it to play nice version control. (That is allowing the client to specify
/// their own values.)
///
/// In many of the headers, there is a boundry at which the interface ends.

#include <ariajanke/ecs3/detail/defs.hpp>

#include <atomic>

namespace ecs {

// ------------------------------ configuration -------------------------------

/// if true, reporting functionings are called; false means fewer function
/// calls made by this library
constexpr const bool k_report_new_types_added = true;
constexpr const bool k_report_allocations = false;

/// default string passed to the "new types reporting" function
constexpr const auto k_default_component_name = "<UNKNOWN COMPONENT>";

/// used to indicate that a type has no "preffered_id" and therefore it's value
/// is implementation defined
///
/// This is used for unit testing, and not generally recommended for client use.
/// @note the actual type for ids is "Size" aka "std::size_t", using int here
///       allows for an easy sentinel value, and I'd hardly imagine there being
///       more than two billion types, especially if ids are manually specified
constexpr const int k_metafunction_has_no_preferred_id = -1;

/// Explicit specializations of this type may specify a "preferred_id".
///
/// If more than one type specifies the same "preferred_id", then undefined
/// behavior will occur.
template <typename T>
struct MetaFunctionPreferredId final {
    static constexpr const int k_preferred_id = k_metafunction_has_no_preferred_id;
};

/// Explicit specializations of this type will "name" a component type which is
/// then passed to the "new type" report function.
template <typename T>
struct MetaFunctionPreferredName final {
    static constexpr const auto k_name = k_default_component_name;
};

/// MetaFunctions are a set of functions for handling objects without knowing
/// their type.
///
/// As you can probably figure: there is going to be a lot of unsafe code later.
/// @note This class is not really made for client use.
class MetaFunctions {
public:
    virtual ~MetaFunctions() {}

    /// Type of function called whenever a new type is instantiated
    using ReportFunc = void (*)(const char *, void *);

    /// Move constructs a new instance of the type.
    /// @warning unsafe code; follow documentation as exactly as possible
    /// @param src An existing instance of type, it's "moved" into the new
    ///            instance. This instance will remain a valid object following
    ///            this call.
    /// @param dest_addr A properly aligned space where an instance of the type
    ///                  may live.
    /// @returns address to the new instance, it should not taken that this will
    ///          be equal to "dest_addr"
    virtual void * move(void * src, void * dest_addr) const = 0;

    /// Destroys an instance of type at addr. An object <em>must</em> exist at
    /// addr, or the behavior is undefined.
    /// @param addr address to an existing object
    virtual void destroy(void * addr) const = 0;

    /// @returns object's size in bytes
    virtual Size object_size() const = 0;

    /// @returns object's alignment in bytes
    virtual Size object_align() const = 0;

    /// @returns object type's key
    virtual Size key() const = 0;

    /// Sets the function to call whenever a new type of component is created
    /// for the entire program's run.
    /// @param report_f function pointer to a "report function"; It will
    ///                 receive the type's name as specified/specialized with
    ///                 the MetaFunctionPreferredName class.
    /// @param user_data Also passed to the report function as the second
    ///                  argument
    static void set_component_addition_tracker
        (ReportFunc report_f, void * user_data = nullptr);

    /// Called by entities to check if a component type is new or not
    template <typename T>
    static void check_if_new_component_type();

    /// Central place where keys are generated for types.
    /// @returns a unique key value for each type
    template <typename T, int k_type_pref = MetaFunctionPreferredId<T>::k_preferred_id>
    static Size key_for_type();

    /// @returns a set of "meta functions" used for information, and methods on
    ///          handling a type
    template <typename T>
    static const MetaFunctions & for_type();
#   ifndef DOXYGEN_SHOULD_SKIP_THIS
private:
    struct Dummy final {};

    template <typename T>
    struct CallbackHolder final {
        static void * s_userdata;
        static ReportFunc s_report_func;
    };
#   endif // DOXYGEN_SHOULD_SKIP_THIS
};

// ------------------------------ INTERFACE ENDS ------------------------------
#ifndef DOXYGEN_SHOULD_SKIP_THIS

template <typename T, int k_type_pref = MetaFunctionPreferredId<T>::k_preferred_id>
struct KeyForType final {
    Size operator() () const noexcept;
};

template <typename T, int k_type_pref>
struct KeyForType<StorageFor<T>, k_type_pref> final {
    Size operator() () const noexcept;
};

/* static */ inline void MetaFunctions::set_component_addition_tracker
    (ReportFunc report_f, void * user_data)
{
    CallbackHolder<Dummy>::s_report_func = report_f;
    CallbackHolder<Dummy>::s_userdata = user_data;
}

template <typename T>
/* static */ void MetaFunctions::check_if_new_component_type() {
    if constexpr (!k_report_new_types_added) return;
    static std::atomic_bool s_new_type = true;
    static constexpr auto k_name = MetaFunctionPreferredName<T>::k_name;
    if (s_new_type) {
        CallbackHolder<Dummy>::s_report_func(k_name, CallbackHolder<Dummy>::s_userdata);
        s_new_type = false;
    }
}

template <typename T, int k_type_pref>
/* static */ Size MetaFunctions::key_for_type()
    { return KeyForType<T, k_type_pref>{}(); }

template <typename T>
/* static */ const MetaFunctions & MetaFunctions::for_type() {
    class Impl final : public MetaFunctions {
        void * move(void * from, void * to_space) const final {
            return new (to_space) T{ std::move(*reinterpret_cast<T *>(from)) };
        }

        void destroy(void * obj) const final
            { reinterpret_cast<T *>(obj)->~T(); }

        Size object_size() const final { return sizeof(T); }

        Size object_align() const final { return alignof(T); }

        Size key() const final { return key_for_type<T>(); }
    };
    static Impl impl;
    return impl;
}

template <typename T>
/* static */ void * MetaFunctions::CallbackHolder<T>::s_userdata = nullptr;

template <typename T>
/* static */ MetaFunctions::ReportFunc MetaFunctions::CallbackHolder<T>::s_report_func =
    [](const char *, void *) {};

// ------------------------------ Helpers Level 1 -----------------------------

template <typename T, int k_type_pref>
Size KeyForType<T, k_type_pref>::operator() () const noexcept {
    static constexpr const int k_no_pref = k_metafunction_has_no_preferred_id;
    // in order to minimize consumed static space, and streamline this idea
    // this should be the only implementation of a type to key source
    if constexpr (k_type_pref != k_no_pref)
        return k_type_pref;
    static uint8_t a = 0;
    return reinterpret_cast<Size>(&a);
}

template <typename T, int k_type_pref>
Size KeyForType<StorageFor<T>, k_type_pref>::operator() () const noexcept
    { return KeyForType<T>{}(); }

#endif // DOXYGEN_SHOULD_SKIP_THIS

} // end of ecs namespace
