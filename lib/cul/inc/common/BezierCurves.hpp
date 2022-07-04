/****************************************************************************

    MIT License

    Copyright 2021 Aria Janke

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.

*****************************************************************************/

#pragma once

#include <common/Vector2.hpp>
#include <common/Vector2Util.hpp>

#include <tuple>

namespace cul {

/** @addtogroup bezierutils
 *  @{
 */

/** Loops for some number of points along a Bezier curve whose control points
 *  are defined by a given tuple.
 *  @tparam T Scalar type of the vector
 *  @tparam Func a function called for each point along the Bezier curve, which
 *          takes the following form: void(cul::Vector2<T>). It is called
 *          "number_of_points" number of times.
 *  @tparam Types remaining types of the control point tuple, must all be
 *          the same vector type.
 *  @param number_of_points the number of points to generate
 */
template <typename T, typename Func, typename ... Types>
void for_bezier_points
    (const std::tuple<Vector2<T>, Types...> &, int number_of_points, Func &&);

/** Loops for some number of lines along a Bezier curve whose control points
 *  are defined by a given tuple.
 *  @tparam T Scalar type of the vector
 *  @tparam Func a function called for each (straight) line along the Bezier
 *          curve, which takes the following form:
 *          void(cul::Vector2<T>, cul::Vector2<T>). It is called
 *          "line_count" number of times.
 *  @tparam Types remaining types of the control point tuple, must all be
 *          the same vector type.
 *  @param line_count the number of lines to generate
 */
template <typename T, typename Func, typename ... Types>
void for_bezier_lines
    (const std::tuple<Vector2<T>, Types...> &, int line_count, Func &&);

/** Computes a single point along a Bezier curve, whose control points are
 *  defined by a given tuple.
 *  @tparam T Scalar type of the vector
 *  @param t any real number between 0 and 1, where 0 represents being at the
 *         first control point, and 1 being at the last control point
 */
template <typename T, typename ... Types>
Vector2<T> find_bezier_point
    (T t, const std::tuple<Vector2<T>, Types...> &);

/** @returns n points on a bezier curve according to a given tuple */
template <std::size_t k_count, typename T, typename ... Types>
std::array<cul::Vector2<T>, k_count> make_bezier_array
    (const std::tuple<cul::Vector2<T>, Types...> & tuple);

/** Generates some number of triangles formed between two Bezier curves.
 *
 *  @note This is a fairly intense algorithm.
 *
 *  This uses a "binary search" to find triangles that are approximately the
 *  specified area (but within some given error). This algorithm will attempt
 *  to follow along self-intersecting curves.
 *
 *  @tparam Func a function called for each triangle along the two Bezier
 *          curves. It must take the following form:
 *          void(cul::Vector2<T>, cul::Vector2<T>, cul::Vector2<T>). It is
 *          called approximately n times where n is the total area of this
 *          "strip" divided by the "resolution"/given area of this call.
 *  @param tuple_a one side of the strip of triangles
 *  @param tuple_b another side of the strip of triangles
 *  @param area or "resolution" area, the area of triangle that is desired
 *         along this strip, note that the last triangle that the algorithm
 *         gives, won't neccessarily be approximately this area
 *  @param error the error margin this algorithm should use, larger values will
 *         make for a faster call, but at the expense of "sloppier" triangle
 *         areas
 *  @param f the given callback function which takes three vectors as its
 *         arguments (see Func)
 */
template <typename Func, typename T, typename ... Types>
void for_bezier_triangles
    (const std::tuple<cul::Vector2<T>, Types...> & tuple_a,
     const std::tuple<cul::Vector2<T>, Types...> & tuple_b,
     T area, T error, Func && f);

/** @}*/

// ----------------------------------------------------------------------------

namespace detail {

template <typename T>
class BezierCurveDetails {
    template <typename U, typename Func, typename ... Types>
    friend void cul::for_bezier_points
        (const std::tuple<cul::Vector2<U>, Types...> &, int, Func &&);

    template <typename U, typename Func, typename ... Types>
    friend void cul::for_bezier_lines
        (const std::tuple<cul::Vector2<U>, Types...> &, int, Func &&);

    template <typename U, typename ... Types>
    friend cul::Vector2<U> cul::find_bezier_point
        (U t, const std::tuple<cul::Vector2<U>, Types...> &);

    using VecT = cul::Vector2<T>;

    template <typename ... Types>
    using Tuple = std::tuple<Types...>;

    template <typename Func, typename ... Types>
    static void for_points(const Tuple<Types...> & tuple, int step_count, Func && f) {
        verify_step_count(step_count, "for_points");
        for (int i = 0; i != step_count; ++i) {
            T v = T(i) / T(step_count);
            f(compute_point_tuple(v, tuple));
        }
        f(compute_point_tuple(1, tuple));
    }

    template <typename Func, typename ... Types>
    static void for_lines(const Tuple<Types...> & tuple, int line_count, Func && f) {
        verify_step_count(line_count, "for_lines");
        for (int i = 0; i != line_count; ++i) {
            auto v = T(i) / T(line_count + 1);
            T next = std::min(T(1), T(i + 1) / T(line_count + 1));
            f(compute_point_tuple(v, tuple), compute_point_tuple(next, tuple));
        }
    }

    template <typename ... Types>
    static VecT compute_point_tuple(T t, const Tuple<Types...> & tuple) {
        return compute_point_tuple<sizeof...(Types)>(t, tuple, std::index_sequence_for<Types...>());
    }

    static void verify_step_count(int t, const char * caller) {
        if (t >= 0) return;
        throw std::invalid_argument(std::string(caller)
                + ": step must be in [0 1].");
    }

    template <std::size_t k_tuple_size, typename TupleT, std::size_t ... kt_indicies>
    static VecT compute_point_tuple(T t, const TupleT & tuple, std::index_sequence<kt_indicies...>) {
        return compute_point<k_tuple_size>(t, std::get<kt_indicies>(tuple)...);
    }

    template <std::size_t k_tuple_size, typename ... Types>
    static VecT compute_point(T, Types ...) { return VecT(); }

    template <std::size_t k_tuple_size, typename ... Types>
    static VecT compute_point(T t, const VecT & r, Types ... args) {
        static_assert(k_tuple_size > sizeof...(Types), "");

        static constexpr const auto k_degree = k_tuple_size - 1;
        static constexpr const auto k_0p_degree = ( k_degree - sizeof...(Types) );
        static constexpr const auto k_1m_degree = k_degree - k_0p_degree;

        static constexpr const T k_scalar
            = ( k_0p_degree == k_degree || k_1m_degree == k_degree )
            ? T(1) : T(k_degree);

        return k_scalar*interpolate<k_1m_degree, k_0p_degree>(t)*r
               + compute_point<k_tuple_size>(t, std::forward<Types>(args)...);
    }

    template <std::size_t k_degree>
    static T interpolate_1m([[maybe_unused]] T t) {
        if constexpr (k_degree == 0)
            { return 1; }
        else
            { return (1 - t)*interpolate_1m<k_degree - 1>(t); }
    }

    template <std::size_t k_degree>
    static T interpolate_0p([[maybe_unused]] T t) {
        if constexpr (k_degree == 0)
            { return 1; }
        else
            { return t*interpolate_0p<k_degree - 1>(t); }
    }

    template <std::size_t k_m1_degree, std::size_t k_0p_degree>
    static T interpolate(T t)
        { return interpolate_1m<k_m1_degree>(t)*interpolate_0p<k_0p_degree>(t); }
};

template <typename T>
class BezierTriangleDetails {
    template <typename Func, typename U, typename ... Types>
    friend void cul::for_bezier_triangles
        (const std::tuple<cul::Vector2<U>, Types...> & tuple_a,
         const std::tuple<cul::Vector2<U>, Types...> & tuple_b,
         U area, U error, Func && f);

    template <typename Func, typename ... Types>
    static void for_bezier_triangles
        (const std::tuple<cul::Vector2<T>, Types...> & tuple_a,
         const std::tuple<cul::Vector2<T>, Types...> & tuple_b,
         T area, T error, Func && f)
    {

        using TupleType = std::tuple<cul::Vector2<T>, Types...>;
        const TupleType * work_side  = &tuple_a;
        const TupleType * other_side = &tuple_b;

        T os_pos = 0;
        T ws_pos = 0;
        // edge case where the two points at the top meet
        if (magnitude(  find_bezier_point(T(0), *work_side )
                      - find_bezier_point(T(0), *other_side)) < error)
        {
            std::tie(os_pos, ws_pos) = progress_from_pinch(
                *work_side, *other_side, area, error);
            auto tip_pt = find_bezier_point(T(0), *work_side);
            auto os_pt = find_bezier_point(os_pos, *other_side);
            auto ws_pt = find_bezier_point(ws_pos, *work_side );
            f(tip_pt, os_pt, ws_pt);
        }

        // main loop
        bool last_iter_finished = false;
        while (true) {
            auto pt_low_os = find_bezier_point(os_pos, *other_side);
            auto pt_low_ws = find_bezier_point(ws_pos, *work_side );
            auto gv = find_next_position(
                *work_side, pt_low_os, pt_low_ws, ws_pos, area, error);
            f(pt_low_ws, pt_low_os, find_bezier_point(gv.next_pos, *work_side));

            if (gv.finishes && last_iter_finished) return;
            last_iter_finished = gv.finishes;
            ws_pos = gv.next_pos;
            std::swap(work_side, other_side);
            std::swap(os_pos   , ws_pos    );
        }
    }

    struct WbcStep { // walk bezier curve step
        bool finishes = false;
        T next_pos;
    };

    template <typename ... Types>
    static std::tuple<T, T> progress_from_pinch
        (const std::tuple<cul::Vector2<T>, Types...> & work_side,
         const std::tuple<cul::Vector2<T>, Types...> & other_side,
         T area, T error)
    {
        const auto & any_side = work_side;
        // I'm going to need a combination things for my
        // resolution criteria
        auto tip_pt = find_bezier_point(T(0), any_side);
        auto os_pt = find_bezier_point(T(1), other_side);
        auto ws_pt = find_bezier_point(T(1), work_side );

        if (area_of_triangle(tip_pt, os_pt, ws_pt) < area) {
            return std::make_tuple(T(1), T(1));
        }
        T low = 0, high = 1;
        while (true) {
            auto mid = (low + high) / T(2);
            auto os_pt = find_bezier_point(mid, other_side);
            auto ws_pt = find_bezier_point(mid, work_side );
            auto tri_con_area = area_of_triangle(tip_pt, os_pt, ws_pt);
            if (magnitude(tri_con_area - area) < error) {
                return std::make_tuple(mid, mid);
            }
            *((tri_con_area > area) ? &high : &low) = mid;
        }
    }

    template <typename ... Types>
    static WbcStep find_next_position
        (const std::tuple<cul::Vector2<T>, Types...> & work_side,
         const cul::Vector2<T> & pt_low_os, const cul::Vector2<T> & pt_low_ws,
         T ws_pos, T area, T error)
    {
        // so now on the work side, we find another point such that we're close
        // to the given area (which is the "resolution" argument)
        WbcStep rv;
        auto pt_con_ws = find_bezier_point(T(1), work_side);
        if (area_of_triangle(pt_low_os, pt_low_ws, pt_con_ws) < area) {
            rv.finishes = true;
            rv.next_pos = T(1);
            return rv;
        }
        T low = ws_pos, high = 1;
        while (true) {
            pt_con_ws = find_bezier_point((low + high) / T(2), work_side);
            auto con_tri_area = area_of_triangle(pt_low_os, pt_low_ws, pt_con_ws);
            if (magnitude(con_tri_area - area) < error) {
                rv.finishes = false;
                rv.next_pos = (low + high) / T(2);
                return rv;
            }
            *((con_tri_area > area) ? &high : &low) = (low + high) / T(2);
        }
    }
};

} // end of detail namespace -> into cul

template <typename T, typename Func, typename ... Types>
void for_bezier_points
    (const std::tuple<Vector2<T>, Types...> & tuple, int step_count, Func && f)
{ return detail::BezierCurveDetails<T>::for_points(tuple, step_count, std::move(f)); }

template <typename T, typename Func, typename ... Types>
void for_bezier_lines
    (const std::tuple<Vector2<T>, Types...> & tuple, int line_count, Func && f)
{ return detail::BezierCurveDetails<T>::for_lines(tuple, line_count, std::move(f)); }

template <typename T, typename ... Types>
cul::Vector2<T> find_bezier_point
    (T t, const std::tuple<Vector2<T>, Types...> & tuple)
{ return detail::BezierCurveDetails<T>::compute_point_tuple(t, tuple); }

template <std::size_t k_count, typename T, typename ... Types>
std::array<cul::Vector2<T>, k_count> make_bezier_array
    (const std::tuple<cul::Vector2<T>, Types...> & tuple)
{
    static constexpr const T k_step = T(1) / T(k_count);
    std::array<cul::Vector2<T>, k_count> arr;
    T t = T(0);
    for (auto & v : arr) {
        v = find_bezier_point(std::min(T(1), t), tuple);
        t += k_step;
    }
    return arr;
}

template <typename Func, typename T, typename ... Types>
void for_bezier_triangles
    (const std::tuple<cul::Vector2<T>, Types...> & tuple_a,
     const std::tuple<cul::Vector2<T>, Types...> & tuple_b,
     T area, T error, Func && f)
{
    detail::BezierTriangleDetails<T>::for_bezier_triangles(
        tuple_a, tuple_b, area, error, std::move(f));
}

} // end of cul namespace
