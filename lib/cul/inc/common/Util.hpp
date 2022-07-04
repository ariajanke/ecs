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

#include <common/TypeList.hpp>

#include <iterator>
#include <type_traits>
#include <stdexcept>
#include <tuple>
#include <array>

#include <cmath> // include cmath you coward

namespace cul {

namespace exceptions_abbr {

using RtError  = std::runtime_error;
using InvArg   = std::invalid_argument;
using OorError = std::out_of_range;

} // end of exceptions_abbr namespace -> into ::cul

template <typename Func, typename Iter>
void quad_range(Iter beg, Iter end, Func && func);

template <typename Func, typename Cont>
void quad_range(Cont & container, Func && func);

template <typename T, typename Func>
void quad_range(std::initializer_list<T> && ilist, Func && f);

/** Describes a sequence view, similar to how they're described in C++20.
 *
 *  @note I've included this, because I keep writting this as a helper
 *        structure.
 */
template <typename IterType>
class View {
public:
    View(IterType b_, IterType e_): m_b(b_), m_e(e_) {}

    IterType begin() const { return m_b; }

    IterType end  () const { return m_e; }

private:
    IterType m_b, m_e;
};

namespace fc_signal {

enum FlowControlSignal_e { k_continue, k_break };

} // end of fc_signal namespace -> into ::cul

using FlowControlSignal = fc_signal::FlowControlSignal_e;

template <typename Func, typename ... Types>
inline typename std::enable_if_t<
    std::is_same_v<typename std::result_of<Func && (Types && ...)>::type, FlowControlSignal>,
    FlowControlSignal> adapt_to_flow_control_signal(Func && f, Types &&... args)
    { return f(std::forward<Types>(args)...); }

template <typename Func, typename ... Types>
inline typename std::enable_if_t<
    std::is_same_v<typename std::result_of<Func && (Types && ...)>::type, void>,
    FlowControlSignal> adapt_to_flow_control_signal(Func && f, Types &&... args)
{
    f(std::forward<Types>(args)...);
    return fc_signal::k_continue;
}

// ------------------------------- math utils ---------------------------------

template <typename T>
using EnableArithmetic = std::enable_if_t<std::is_arithmetic_v<T>, T>;

// macros are going too far for me
template <typename T>
constexpr const typename std::enable_if_t<std::is_arithmetic_v<T>, T>
    k_pi_for_type = T(3.141592653589793238462643383279);

template <typename T>
    inline EnableArithmetic<T> magnitude(T t)
    { return (t < T(0)) ? -t : t; }

template <typename T>
EnableArithmetic<T> normalize(T t);

template <typename T>
inline std::enable_if_t<std::is_arithmetic_v<T>, bool>
    are_within(T a, T b, T error)
{ return magnitude(a - b) < error; }

template <typename T>
    inline typename std::enable_if_t<std::is_floating_point_v<T>,
    bool> is_nan(T a)
{ return a != a; }

template <typename T>
    inline typename std::enable_if_t<
    std::is_arithmetic_v<T> && !std::is_floating_point_v<T>,
    bool> is_nan(T)
{ return false; }

template <typename T>
inline std::enable_if_t<std::is_floating_point_v<T>, bool> is_real(T x) {
    using Lims = std::numeric_limits<T>;
    return x == x && Lims::infinity() != x && -Lims::infinity() != x;
}

template <typename T>
inline std::enable_if_t<std::is_arithmetic_v<T> && !std::is_floating_point_v<T>,
    bool> is_real(T) { return true; }

template <typename T, typename U>
inline std::enable_if_t<std::is_arithmetic_v<T> && std::is_floating_point_v<U>,
    T> round_to(U u)
{ return T(std::round(u)); }

template <typename ... Types>
using Tuple = std::tuple<Types...>;

// first  low  bounds where f(first ) = false
// second high bounds where f(second) = true
template <typename T, typename Func>
std::enable_if_t<std::is_floating_point_v<T>,
    Tuple<T, T>> find_smallest_diff
    (Func && f, T hint = T(0.5), T error = T(0.0005));

template <typename T, typename Func>
inline std::enable_if_t<std::is_floating_point_v<T>, T> find_highest_false
    (Func && f, T hint = T(0.5), T error = T(0.0005))
{ return std::get<0>(find_smallest_diff(std::move(f), hint, error)); }

template <typename T, typename Func>
inline std::enable_if_t<std::is_floating_point_v<T>, T>
    find_lowest_true(Func && f, T hint = T(0.5), T error = T(0.0005))
{ return std::get<1>(find_smallest_diff(std::move(f), hint, error)); }

/** Makes an array for a single value. Template arguments are flipped since
 *  size is least likely to be deducible.
 *  @tparam kt_size number of elements in the array
 *  @tparam T type of element in the array
 *  @tparam U type of the copied object, must be convertible to T
 *  @param obj The initial value for every member in the array
 */
template <std::size_t kt_size, typename T, typename U>
constexpr std::enable_if_t<!std::is_same_v<T, U> && std::is_convertible_v<U, T>,
    std::array<T, kt_size>>
    make_filled_array(const U & obj)
{
    std::array<T, kt_size> rv;
    std::fill(rv.begin(), rv.end(), obj);
    return rv;
}

/** Makes an array for a single value. Template arguments are flipped since
 *  size is least likely to be deducible.
 *  @tparam kt_size number of elements in the array
 *  @tparam T type of element in the array
 *  @param obj The initial value for every member in the array
 */
template <std::size_t kt_size, typename T>
constexpr std::array<T, kt_size> make_filled_array(const T & obj) {
    std::array<T, kt_size> rv;
    std::fill(rv.begin(), rv.end(), obj);
    return rv;
}

/** For each element in a tuple who derives from "Base", function "f" will be
 *  called.
 *
 *  @tparam Base The base class which
 *
 *  @param f function to call, must take either form of:
 *  - void(Base &)
 *  - void(CommonBase &) where "CommonBase" is a type from which all tuple
 *    elements are derived from
 *
 *  @note
 *  There are presently no plans to allow short-cutting out of this "loop" by
 *  returning a "fc_signal" enum. @n
 *  The order in which the tuple iteratored maybe any order or "implementation
 *  defined".
 *
 *  @note [for implementor] const, and writable must be tested
 */
template <typename Base, typename Func, typename ... Types>
void for_all_of_base(Tuple<Types...> &, Func && f);

/** For each element in a tuple who derives from "Base", function "f" will be
 *  called.
 *
 *  @tparam Base The base class which
 *
 *  @param f function to call, must take the form: void(const Base &)
 */
template <typename Base, typename Func, typename ... Types>
void for_all_of_base(const Tuple<Types...> &, Func && f);

/** For each element in a tuple who derives from "Base", function "f" will be
 *  called.
 *
 *  This overload checks if elements are references, and if so will call allow
 *  f to take a writable reference as an argument.
 *
 *  @tparam Base The base class which
 *
 *  @param f function to call, must take the form: void(Base &)
 */
template <typename Base, typename Func, typename ... Types>
void for_all_of_base(const Tuple<Types & ...> &, Func && f);

// ----------------------- Implementation Details -----------------------------

template <typename Func, typename Iter>
void quad_range(Iter beg, Iter end, Func && func) {
    using namespace fc_signal;
    for (Iter itr = beg; itr != end; ++itr) {
    for (Iter jtr = beg; jtr != itr; ++jtr) {
        auto & i_obj = *itr;
        auto & j_obj = *jtr;
        if (adapt_to_flow_control_signal(std::move(func), i_obj, j_obj) == k_break)
            return;
    }}
}

template <typename Func, typename Cont>
void quad_range(Cont & container, Func && func)
    { quad_range(std::begin(container), std::end(container), std::move(func)); }

template <typename T, typename Func>
void quad_range(std::initializer_list<T> && ilist, Func && f)
    { quad_range(ilist.begin(), ilist.end(), std::move(f)); }

template <typename T>
EnableArithmetic<T> normalize(T t) {
    using namespace exceptions_abbr;
    if (t == 0) {
        throw InvArg("normalize: attempted to normalize a zero 'vector'.");
    }
    if (!is_real(t)) {
        throw InvArg("normalize: attempted to normalize a non real number.");
    }
    return t / magnitude(t);
}

template <typename T, typename Func>
std::enable_if_t<std::is_floating_point_v<T>,
    Tuple<T, T>> find_smallest_diff(Func && f, T hint, T error)
{
    using namespace exceptions_abbr;
    if (f(0)) {
        throw InvArg("find_smallest_diff: f(0) is true.");
    }
    if (!f(1)) {
        throw InvArg("find_smallest_diff: f(1) is false.");
    }

    bool fg   = f(hint);
    T    low  = fg ? 0    : hint;
    T    high = fg ? hint : 1   ;

    while ((high - low) > error) {
        T t = low + (high - low)*T(0.5);
        if (f(t)) {
            high = t;
        } else {
            low = t;
        }
    }
    return std::make_tuple(low, high);
}

namespace detail {

class ForAllOfBasePriv {
    template <typename Base, typename Func, typename ... Types>
    friend void ::cul::for_all_of_base(Tuple<Types...> &, Func &&);

    template <typename Base, typename Func, typename ... Types>
    friend void ::cul::for_all_of_base(const Tuple<Types...> &, Func &&);

    template <typename Type>
    using Bare = std::remove_const_t<std::remove_reference_t<Type>>;

    template <typename Base, typename Derived>
    static constexpr const bool kt_bare_is_base_of =
        std::is_base_of_v<Base, Bare<Derived>>;

    template <typename Base, typename Func, typename TupleType, std::size_t k_index>
    static void for_all_of_base_impl(TupleType & tuple, Func & f) {
        using GetRt = decltype(std::get<k_index>(tuple));
        if constexpr (kt_bare_is_base_of<Base, GetRt>) {
            Bare<GetRt> & head = std::get<k_index>(tuple);
            f(head);
        }
        if constexpr (k_index != 0) {
            for_all_of_base_impl<Base, Func, TupleType, k_index - 1>(tuple, f);
        }
    }

    template <typename Base, typename Func, typename TupleType, std::size_t k_index>
    static void for_all_of_base_impl(const TupleType & tuple, Func & f) {
        using GetRt = decltype(std::get<k_index>(tuple));
        if constexpr (kt_bare_is_base_of<Base, GetRt>) {
            const Bare<GetRt> & head = std::get<k_index>(tuple);
            f(head);
        }
        if constexpr (k_index != 0) {
            for_all_of_base_impl<Base, Func, TupleType, k_index - 1>(tuple, f);
        }
    }
};

} // end of detail namespace -> into ::cul

template <typename Base, typename Func, typename ... Types>
void for_all_of_base(Tuple<Types...> & tuple, Func && f) {
    if constexpr (sizeof...(Types) != 0) {
        detail::ForAllOfBasePriv::for_all_of_base_impl
            <Base, Func, Tuple<Types...>, sizeof...(Types) - 1>
            (tuple, f);
    }
}

template <typename Base, typename Func, typename ... Types>
void for_all_of_base(const Tuple<Types...> & tuple, Func && f) {
    if constexpr (sizeof...(Types) != 0) {
        detail::ForAllOfBasePriv::for_all_of_base_impl
            <Base, Func, Tuple<Types...>, sizeof...(Types) - 1>
            (tuple, f);
    }
}

template <typename Base, typename Func, typename ... Types>
void for_all_of_base(const Tuple<Types & ...> & tuple, Func && f) {
    auto copy = tuple;
    for_all_of_base<Base>(copy, std::move(f));
}

} // end of cul namespace
