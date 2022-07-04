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

#include <common/Util.hpp>
#include <common/Vector2.hpp>

#include <tuple>

/** @defgroup vec2utils Vector2 Utility Functions */

namespace cul {

template <typename T>
using EnableRectangle = std::enable_if_t<std::is_arithmetic_v<T>, Rectangle<T>>;

template <typename T>
constexpr const bool k_is_convertible_vector2
    = std::is_same_v<typename Vector2Scalar<T>::Type, void> ?
      false :
         Vector2Traits<typename Vector2Scalar<T>::Type, T>::k_is_vector_type
      && !std::is_same_v<T, Vector2<typename Vector2Scalar<T>::Type>>;

template <typename T>
constexpr const bool k_is_vector2_util_suitable
    = k_is_convertible_vector2<T> ?
        std::is_arithmetic_v<typename Vector2Scalar<T>::Type> :
        false;

template <typename Vec, typename Rt>
using EnableVec2Util
    = std::enable_if_t<
         k_is_vector2_util_suitable<Vec>
      || std::is_same_v<Vec, Vector2<typename Vector2Scalar<Vec>::Type>>
      , Rt>;

template <typename Vec>
using EnableVec2UtilRetScalar
    = std::enable_if_t<
         k_is_vector2_util_suitable<Vec>
      || std::is_same_v<Vec, Vector2<typename Vector2Scalar<Vec>::Type>>
      , typename Vector2Scalar<Vec>::Type>;

/** @addtogroup vec2utils
 *  @{
 */

/** @returns magnitude of v
 *
 *  @throws if r does not have real components
 *  @tparam Vec must be either the builtin Vector2 type or a type convertible
 *          to and from one
 *  @param r must be a vector of real components
 */
template <typename Vec>
EnableVec2UtilRetScalar<Vec> magnitude(const Vec & r);

/** @returns normal vector of r
 *
 *  @throws if r does not have real components or is the zero vector
 *  @tparam Vec must be either the builtin Vector2 type or a type convertible
 *          to and from one
 *  @param r must be a vector of real components
 */
template <typename Vec>
EnableVec2Util<Vec, Vec> normalize(const Vec & r);

/** @returns true if both vectors are within some number of units within each
 *           other.
 *  @throws if any are not real numbers: either vector's component, the error
 *          parameter
 *  @tparam Vec must be either the builtin Vector2 type or a type convertible
 *          to and from one
 *  @param a any real vector
 *  @param b any real vector
 *  @param error any non-negative real number, providing a negative number will
 *         cause this function to always return true
 */
template <typename Vec>
EnableVec2Util<Vec, bool> are_within
    (const Vec & a, const Vec & b, typename Vector2Scalar<Vec>::Type error);

/** Rotates vector r along the unit circle of some given number of radians rot.
 *
 *  @throws if rot or the components of r are not real numbers
 *  @tparam Vec must be either the builtin Vector2 type or a type convertible
 *          to and from one
 *  @param r must be a vector of real components
 *  @param rot must be a real number, in radians
 *  @return the rotated vector
 */
template <typename Vec>
EnableVec2Util<Vec, Vec> rotate_vector
    (const Vec & r, typename Vector2Scalar<Vec>::Type rot);

/** @returns the dot product of two vectors
 *
 *  @tparam Vec must be either the builtin Vector2 type or a type convertible
 *          to and from one
 *  @param v
 *  @param u
 *  @note no checks if components are real numbers
 */
template <typename Vec>
EnableVec2UtilRetScalar<Vec> dot(const Vec & v, const Vec & u) noexcept;

/** @returns the "z" component of the cross product of two 2D vectors.
 *
 *  @tparam Vec must be either the builtin Vector2 type or a type convertible
 *          to and from one
 *  @param v
 *  @param u
 *  @note no checks if components are real numbers
 */
template <typename Vec>
EnableVec2UtilRetScalar<Vec> cross(const Vec & v, const Vec & u) noexcept;

/** @returns the magnitude of the angle between two vectors, in radians
 *
 *  @throws if any component of either vector is not a real number
 *  @note if direction is also desired directed_angle_between can be used
 *  @see rotate_vector, directed_angle_between
 *  @tparam Vec must be either the builtin Vector2 type or a type convertible
 *          to and from one
 *  @param v
 *  @param u
 */
template <typename Vec>
EnableVec2UtilRetScalar<Vec> angle_between(const Vec & v, const Vec & u);

/** @returns the angle between the "from" and "to" vectors such that,
 *           rotate_vector(from, this_return_value) is roughly equal to vector
 *           "to".
 *
 *  @throws if any component of either vector is not a real number
 *  @see rotate_vector, angle_between
 *  @tparam Vec must be either the builtin Vector2 type or a type convertible
 *          to and from one
 *  @param from starting vector prerotation
 *  @param to   destination vector of any magnitude
 */
template <typename Vec>
EnableVec2UtilRetScalar<Vec> directed_angle_between(const Vec & from, const Vec & to);

/** @returns the projection of vector a onto vector b
 *
 *  @throws if either a or b have non real components or if b is the zero
 *          vector
 *  @tparam Vec must be either the builtin Vector2 type or a type convertible
 *          to and from one
 *  @param a
 *  @param b
 */
template <typename Vec>
EnableVec2Util<Vec, Vec> project_onto(const Vec & a, const Vec & b);

/** @returns The intersection between two lines "a" and "b", or a special
 *           sentinel value if no intersection can be found.
 *
 *  @note You can find the "no intersection" sentinel value by calling the
 *        "get_no_solution_sentinel"
 *  @see get_no_solution_sentinel
 *
 *  @throws if any vector as a non real component
 *  @tparam Vec must be either the builtin Vector2 type or a type convertible
 *          to and from one
 *  @param a_first the first point of line "a"
 *  @param a_second the second point of line "a"
 *  @param b_first the first point of line "b"
 *  @param b_second the second point of line "b"
 */
template <typename Vec>
EnableVec2Util<Vec, Vec> find_intersection
    (const Vec & a_first, const Vec & a_second,
     const Vec & b_first, const Vec & b_second);

/** @returns A vector rounded from a floating point to an integer type.
 *
 *  This just calls "round_to" for each respective component, no regard for
 *  whether they are real numbers or not.
 */
template <typename T, typename U>
inline std::enable_if_t<std::is_integral_v<T> && std::is_floating_point_v<U>, Vector2<T>>
    round_to(const Vector2<U> & r)
{ return Vector2<T>(round_to<T>(r.x), round_to<T>(r.y)); }

/** Computes velocties in which a projectile can reach a target from a source
 *  given speed and influencing_acceleration (like gravity).
 *
 *  @returns a tuple with two solutions to this problem. Special sentinel
 *           values are returned if there is no solution
 *           (@see get_no_solution_sentinel), and if there is only one, then
 *           both values returned will be the same
 *
 *  @note much like find_intersection, the special "no solutions" sentinel
 *        value can be found by calling "get_no_solution_sentinel"
 *
 *  @throws if any component of any vector or scalar is not a real number
 *  @tparam Vec must be either the builtin Vector2 type or a type convertible
 *          to and from one
 *  @param source projectile starting point
 *  @param target projectile's target
 *  @param influencing_acceleration any influencing force (without the mass
 *         portion, e.g. gravity)
 *  @param speed The speed at which to fire the projectile
 */
template <typename Vec>
EnableVec2Util<Vec, std::tuple<Vec, Vec>>
    find_velocities_to_target
    (const Vec & source, const Vec & target,
     const Vec & influencing_acceleration, typename Vector2Scalar<Vec>::Type speed);

/** @returns the closest point on the line defined by points "a" and "b" to
 *           some given external point.
 *  @tparam Vec must be either the builtin Vector2 type or a type convertible
 *          to and from one
 *  @throws if any component of any vector is not a real number
 *  @param a one extreme point of the line segment
 *  @param b another extreme point of the line segment
 *  @param external_point some point outside of the line
 */
template <typename Vec>
EnableVec2Util<Vec, Vec> find_closest_point_to_line
    (const Vec & a, const Vec & b, const Vec & external_point);

/** @returns true if both components are real numbers, false otherwise.
 *  @tparam Vec must be either the builtin Vector2 type or a type convertible
 *          to and from one
 */
template <typename Vec>
EnableVec2Util<Vec, bool> is_real(const Vec & r);

/** @returns the "no solution" sentinel vector returned by many functions which
 *           may not have a solution for any given set of parameters.
 *
 *  @tparam Vec must be either the builtin Vector2 type or a type convertible
 *          to and from one
 */
template <typename Vec>
EnableVec2Util<Vec, Vec> get_no_solution_sentinel();

/** @returns the "no solution" sentinel scalar value use for each component
 *           in the "no solution" vector.
 *
 *  @tparam T must be an arithmetic type. (non arithmetic must define their
 *            own)
 */
template <typename T>
constexpr std::enable_if_t<std::is_arithmetic_v<T>, T> get_no_solution_scalar();

/** The "no solution" sentinel vector returned by many functions.
 *  This constant's name is constrained for 2d vectors only.
 */
template <typename T>
const constexpr Vector2<T> k_no_solution_for_v2 = Vector2<T>{
    get_no_solution_scalar<T>(), get_no_solution_scalar<T>()};

/** @returns the area of a triangle as defined by three points.
 *
 *  @tparam Vec must be either the builtin Vector2 type or a type convertible
 *          to and from one
 */
template <typename Vec>
EnableVec2UtilRetScalar<Vec>
    area_of_triangle(const Vec & a, const Vec & b, const Vec & c)
{ return magnitude(cross(a - b, c - b)); }

/** @returns true if a test point is inside a triangle defined by three points.
 *  @tparam Vec must a 2d vector for which vector traits are defined.
 *  @param a A single end point defining the perimeter of the triangle.
 *  @param b A single end point defining the perimeter of the triangle.
 *  @param c A single end point defining the perimeter of the triangle.
 *  @param test_point a point to test whether it is inside the triangle or not.
 */
template <typename Vec>
EnableVec2Util<Vec, bool> is_inside_triangle
    (const Vec & a, const Vec & b, const Vec & c, const Vec & test_point);
/** @}*/

// ------------------ everything pretaining to rectangles ---------------------

template <typename T>
std::enable_if_t<std::is_arithmetic_v<T>, Size2<T>>
    make_size(T width_, T height_);

template <typename T>
void set_top_left_of(EnableRectangle<T> & rect, T left, T top);

template <typename T>
void set_size_of(EnableRectangle<T> & rect, T width, T height);

template <typename T>
inline void set_top_left_of(EnableRectangle<T> & rect, const Vector2<T> & r)
    { set_top_left_of(rect, r.x, r.y); }

template <typename T>
void set_size_of(EnableRectangle<T> & rect, const Size2<T> & r)
    { set_size_of(rect, r.width, r.height); }

/** @returns top left location of a rectangle as a vector. */
template <typename T>
Vector2<T> top_left_of(const Rectangle<T> & rect)
    { return Vector2<T>(rect.left, rect.top); }

/** @returns top right location of a rectangle as a vector. */
template <typename T>
Vector2<T> top_right_of(const Rectangle<T> & rect)
    { return Vector2<T>(rect.left + rect.width, rect.top); }

/** @returns bottom left location of a rectangle as a vector. */
template <typename T>
Vector2<T> bottom_left_of(const Rectangle<T> & rect)
    { return Vector2<T>(rect.left, rect.top + rect.height); }

/** @returns bottom right location of a rectangle as a vector. */
template <typename T>
Vector2<T> bottom_right_of(const Rectangle<T> & rect)
    { return Vector2<T>(rect.left + rect.width, rect.top + rect.height); }

template <typename T>
EnableArithmetic<T>
    right_of(const Rectangle<T> & rect) { return rect.left + rect.width; }

template <typename T>
EnableArithmetic<T>
    bottom_of(const Rectangle<T> & rect) { return rect.top + rect.height; }

template <typename T>
std::enable_if_t<std::is_arithmetic_v<T>, Size2<T>>
    size_of(const Rectangle<T> &);

template <typename T>
std::enable_if_t<std::is_arithmetic_v<T>, Vector2<T>>
    center_of(const Rectangle<T> &);

template <typename T>
EnableRectangle<T> find_rectangle_intersection
    (const Rectangle<T> &, const Rectangle<T> &);

template <typename T>
std::enable_if_t<std::is_arithmetic_v<T>, bool>
    overlaps(const Rectangle<T> &, const Rectangle<T> &);

template <typename T>
EnableArithmetic<T> area_of(const Rectangle<T> & a)
    { return a.width*a.height; }

template <typename T>
EnableRectangle<T> compose(const Vector2<T> & top_left, const Size2<T> &);

template <typename T>
std::enable_if_t<std::is_arithmetic_v<T>, Tuple<Vector2<T>, Size2<T>>>
    decompose(const Rectangle<T> & rect)
{ return std::make_tuple(top_left_of(rect), size_of(rect)); }

// ----------------------- Implementation Details -----------------------------
#ifndef DOXYGEN_SHOULD_SKIP_THIS

namespace detail {

template <typename T>
Vector2<T> find_intersection
    (const Vector2<T> & a_first, const Vector2<T> & a_second,
     const Vector2<T> & b_first, const Vector2<T> & b_second);

template <typename T>
std::enable_if_t<std::is_floating_point_v<T>, std::tuple<Vector2<T>,Vector2<T>>>
    find_velocities_to_target
    (const Vector2<T> & source, const Vector2<T> & target,
     const Vector2<T> & influencing_acceleration, T speed);

template <typename T>
Vector2<T> find_closest_point_to_line
    (const Vector2<T> & a, const Vector2<T> & b,
     const Vector2<T> & external_point);

template <typename T>
bool is_inside_triangle
    (const Vector2<T> & a, const Vector2<T> & b, const Vector2<T> & c,
     const Vector2<T> & test_point);

} // end of detail namespace -> into ::cul

template <typename Vec>
EnableVec2UtilRetScalar<Vec> magnitude(const Vec & r) {
    using T = typename Vector2Scalar<Vec>::Type;
    using namespace exceptions_abbr;
    if constexpr (std::is_same_v<Vec, Vector2<T>>) {
        if (is_real(r)) return std::sqrt(r.x*r.x + r.y*r.y);
        throw InvArg("magnitude: given vector must have real number components.");
    } else if constexpr (k_is_vector2_util_suitable<Vec>) {
        return magnitude(convert_to<Vector2<T>>(r));
    } else {
        throw RtError("Bad branch");
    }
}

template <typename Vec>
EnableVec2Util<Vec, Vec> normalize(const Vec & r) {
    using T = typename Vector2Scalar<Vec>::Type;
    using namespace exceptions_abbr;
    if constexpr (std::is_same_v<Vec, Vector2<T>>) {
        if (is_real(r) && r != Vector2<T>()) return r*(T(1) / magnitude(r));
        throw InvArg("normalize: attempting to normalize a non real or zero vector.");
    } else {
        static_assert(k_is_vector2_util_suitable<Vec>, "Type Vec not suitible for conversion.");
        return convert_to<Vec>(normalize(convert_to<Vector2<T>>(r)));
    }
}

template <typename Vec>
EnableVec2Util<Vec, bool> are_within
    (const Vec & a, const Vec & b, typename Vector2Scalar<Vec>::Type error)
{
    using T = typename Vector2Scalar<Vec>::Type;
    using namespace exceptions_abbr;
    if constexpr (std::is_same_v<Vec, Vector2<T>>) {
        auto diff = a - b;
        if (is_real(diff)) return magnitude(diff) <= error;
        throw InvArg("normalize: attempting to normalize a non real or zero vector.");
    } else {
        static_assert(k_is_vector2_util_suitable<Vec>, "Type Vec not suitible for conversion.");
        using VecImp = Vector2<T>;
        return are_within(convert_to<VecImp>(a), convert_to<VecImp>(b), error);
    }
}

template <typename Vec>
EnableVec2Util<Vec, Vec> rotate_vector
    (const Vec & r, typename Vector2Scalar<Vec>::Type rot)
{
    if (!is_real(r) || !is_real(rot)) {
        throw std::invalid_argument("rotate_vector: r must be a real vector "
                                    "and rot a real number.");
    }
    using Scalar = typename Vector2Scalar<Vec>::Type;
    using Tr     = Vector2Traits<Scalar, Vec>;
    typename Tr::GetX get_x;
    typename Tr::GetY get_y;
    // [r.x] * [ cos(rot) sin(rot)]
    // [r.y]   [-sin(rot) cos(rot)]
    return typename Tr::Make{}(
        get_x(r)*std::cos(rot) - get_y(r)*std::sin(rot),
        get_x(r)*std::sin(rot) + get_y(r)*std::cos(rot));
}

template <typename Vec>
EnableVec2UtilRetScalar<Vec> dot(const Vec & v, const Vec & u) noexcept {
    using Scalar  = typename Vector2Scalar<Vec>::Type;
    using Tr      = Vector2Traits<Scalar, Vec>;
    typename Tr::GetX get_x;
    typename Tr::GetY get_y;
    return get_x(v)*get_x(u) + get_y(v)*get_y(u);
}

template <typename Vec>
EnableVec2UtilRetScalar<Vec> cross(const Vec & v, const Vec & u) noexcept {
    using Scalar  = typename Vector2Scalar<Vec>::Type;
    using Tr      = Vector2Traits<Scalar, Vec>;
    typename Tr::GetX get_x;
    typename Tr::GetY get_y;
    return get_x(v)*get_y(u) - get_x(u)*get_y(v);
}

template <typename Vec>
EnableVec2UtilRetScalar<Vec> angle_between(const Vec & v, const Vec & u) {
    // problematic with integer vectors...
    using namespace exceptions_abbr;
    if (!is_real(v) || !is_real(u)) {
        throw InvArg("angle_between: both vectors must be real vectors.");
    }
    using T = typename Vector2Scalar<Vec>::Type;
    static const constexpr T k_error = 0.00005;
    auto mag_v = magnitude(v);
    auto mag_u = magnitude(u);
    if (   are_within(mag_v / mag_u, T(0), k_error)
        || are_within(mag_u / mag_v, T(0), k_error))
    {
        // this will have to be tested with my game and other stuff
        throw InvArg("angle_between: both vectors must not be too far in magnitude.");
    }
    auto frac = dot(v, u) / (mag_v*mag_u);
    if      (frac > T( 1)) { frac = T( 1); }
    else if (frac < T(-1)) { frac = T(-1); }
    return std::acos(frac);
}

template <typename Vec>
EnableVec2UtilRetScalar<Vec> directed_angle_between(const Vec & from, const Vec & to) {
    using namespace exceptions_abbr;
    if (!is_real(from) || !is_real(to)) {
        throw InvArg("directed_angle_between: both vectors must be real vectors.");
    }

    using Scalar = typename Vector2Scalar<Vec>::Type;
    using Tr     = Vector2Traits<Scalar, Vec>;
    typename Tr::GetX get_x;
    typename Tr::GetY get_y;
    return std::atan2(get_y(from), get_x(from)) - std::atan2(get_y(to), get_x(to));
}

template <typename Vec>
EnableVec2Util<Vec, Vec> project_onto(const Vec & a, const Vec & b) {
    using T = typename Vector2Scalar<Vec>::Type;
    using namespace exceptions_abbr;
    if constexpr (std::is_same_v<Vec, Vector2<T>>) {
        static const constexpr T k_error = 0.00005;
        if ((a.x*a.x + a.y*a.y) < k_error*k_error)
            throw InvArg("project_onto: cannot project onto the zero vector.");
        if (!is_real(a) || !is_real(b))
            throw InvArg("project_onto: both vectors must be real.");
        return (dot(b, a)/(b.x*b.x + b.y*b.y))*b;
    } else {
        static_assert(k_is_vector2_util_suitable<Vec>,
                      "Type Vec not suitible for conversion.");
        using VecImp = Vector2<T>;
        return convert_to<Vec>(project_onto(convert_to<VecImp>(a),
                                            convert_to<VecImp>(b)));
    }
}

template <typename Vec>
EnableVec2Util<Vec, Vec> find_intersection
    (const Vec & a_first, const Vec & a_second,
     const Vec & b_first, const Vec & b_second)
{
    using namespace exceptions_abbr;
    using VecImp = Vector2<typename Vector2Scalar<Vec>::Type>;
    if constexpr (std::is_same_v<Vec, VecImp>) {
        if (   !is_real(a_first) || !is_real(a_second)
            || !is_real(b_first) || !is_real(b_second))
        {
            throw InvArg("find_intersection: each vector parameter must be a "
                         "real vector.");
        }
        return detail::find_intersection(a_first, a_second, b_first, b_second);
    } else {
        static_assert(k_is_vector2_util_suitable<Vec>,
                      "Type Vec not suitible for conversion.");
        return convert_to<Vec>(find_intersection(
            convert_to<VecImp>(a_first), convert_to<VecImp>(a_second),
            convert_to<VecImp>(b_first), convert_to<VecImp>(b_second)));
    }
}

template <typename Vec>
EnableVec2Util<Vec, std::tuple<Vec, Vec>>
    find_velocities_to_target
    (const Vec & source, const Vec & target,
     const Vec & influencing_acceleration,
     typename Vector2Scalar<Vec>::Type speed)
{
    using T = typename Vector2Scalar<Vec>::Type;
    if constexpr (std::is_same_v<Vec, Vector2<T>>) {
        return detail::find_velocities_to_target(
            source, target, influencing_acceleration, speed);
    } else {
        static_assert(k_is_vector2_util_suitable<Vec>,
                      "Type Vec not suitible for conversion.");
        using VecImp = Vector2<T>;
        auto [s0, s1] = find_velocities_to_target(
            convert_to<VecImp>(source), convert_to<VecImp>(target),
            convert_to<VecImp>(influencing_acceleration), speed);
        return std::make_tuple(convert_to<Vec>(s0), convert_to<Vec>(s1));
    }
}

template <typename Vec>
EnableVec2Util<Vec, Vec> find_closest_point_to_line
    (const Vec & a, const Vec & b, const Vec & external_point)
{
    using T = typename Vector2Scalar<Vec>::Type;
    if constexpr (std::is_same_v<Vec, Vector2<T>>) {
        return detail::find_closest_point_to_line(a, b, external_point);
    } else {
        static_assert(k_is_vector2_util_suitable<Vec>,
                      "Type Vec not suitible for conversion.");
        using VecImp = Vector2<T>;
        return convert_to<Vec>(find_closest_point_to_line(
            convert_to<VecImp>(a), convert_to<VecImp>(b),
            convert_to<VecImp>(external_point)));
    }
}

template <typename Vec>
EnableVec2Util<Vec, bool> is_real(const Vec & r) {
    using Tr = Vector2Traits<typename Vector2Scalar<Vec>::Type, Vec>;
    typename Tr::GetX get_x;
    typename Tr::GetY get_y;
    return is_real(get_x(r)) && is_real(get_y(r));
}

template <typename Vec>
EnableVec2Util<Vec, Vec> get_no_solution_sentinel() {
    static const Vec k_rv = []() {
        using Scalar  = typename Vector2Scalar<Vec>::Type;
        using Tr      = Vector2Traits<Scalar, Vec>;

        Scalar comp = get_no_solution_scalar<Scalar>();
        return typename Tr::Make{}(comp, comp);
    } ();
    return k_rv;
}

template <typename T>
constexpr std::enable_if_t<std::is_arithmetic_v<T>, T> get_no_solution_scalar() {
    using NumLims = std::numeric_limits<T>;
    /**/ if constexpr (NumLims::has_infinity) { return NumLims::infinity(); }
    else if constexpr (std::is_signed_v<T>  ) { return NumLims::min     (); }
    else                                      { return NumLims::max     (); }
}

template <typename Vec>
EnableVec2Util<Vec, bool> is_inside_triangle
    (const Vec & a, const Vec & b, const Vec & c, const Vec & test_point)
{
    using T = typename Vector2Scalar<Vec>::Type;
    if constexpr (std::is_same_v<Vec, Vector2<T>>) {
        using VecImp = Vector2<T>;
        return detail::is_inside_triangle(
            convert_to<VecImp>(a), convert_to<VecImp>(b), convert_to<VecImp>(c),
            convert_to<VecImp>(test_point));
    } else {
        return detail::is_inside_triangle(a, b, c, test_point);
    }
}

// ----------------------- Implementation Details -----------------------------
// ------------------ everything pretaining to rectangles ---------------------

template <typename T>
std::enable_if_t<std::is_arithmetic_v<T>, Size2<T>>
    make_size(T width_, T height_)
{
    Size2<T> rv;
    rv.width  = width_ ;
    rv.height = height_;
    return rv;
}

template <typename T>
void set_top_left_of(EnableRectangle<T> & rect, T left, T top) {
    rect.left = left;
    rect.top  = top ;
}

template <typename T>
void set_size_of(EnableRectangle<T> & rect, T width, T height) {
    rect.width  = width ;
    rect.height = height;
}

template <typename T>
std::enable_if_t<std::is_arithmetic_v<T>, Size2<T>>
    size_of(const Rectangle<T> & rect)
{
    Size2<T> size;
    size.width  = rect.width ;
    size.height = rect.height;
    return size;
}

template <typename T>
std::enable_if_t<std::is_arithmetic_v<T>, Vector2<T>>
    center_of(const Rectangle<T> & rect)
{
    return Vector2<T>(rect.left + rect.width  / T(2),
                      rect.top  + rect.height / T(2));
}

template <typename T>
EnableRectangle<T> find_rectangle_intersection
    (const Rectangle<T> & a, const Rectangle<T> & b)
{
    using TVec = Vector2<T>;
    auto high_a = TVec(right_of(a), bottom_of(a));
    auto high_b = TVec(right_of(b), bottom_of(b));

    auto low_rv  = TVec(std::max(a.left, b.left), std::max(a.top, b.top));
    auto high_rv = TVec(std::min(high_a.x, high_b.x), std::min(high_a.y, high_b.y));
    if (low_rv.x >= high_rv.x || low_rv.y >= high_rv.y) {
        return Rectangle<T>();
    } else {
        return Rectangle<T>(low_rv.x, low_rv.y,
                            high_rv.x - low_rv.x, high_rv.y - low_rv.y);
    }
}

template <typename T>
std::enable_if_t<std::is_arithmetic_v<T>, bool>
    overlaps(const Rectangle<T> & a, const Rectangle<T> & b)
{
    return    right_of (a) > b.left && right_of (b) > a.left
           && bottom_of(a) > b.top  && bottom_of(b) > a.top ;
}

template <typename T>
EnableRectangle<T> compose(const Vector2<T> & top_left, const Size2<T> & size) {
    return Rectangle<T>(top_left.x, top_left.y, size.width, size.height);
}

// ------------------- Vector Utility Helper Implementations ------------------

namespace detail {

template <typename T>
Vector2<T> find_intersection
    (const Vector2<T> & a_first, const Vector2<T> & a_second,
     const Vector2<T> & b_first, const Vector2<T> & b_second)
{
    using namespace exceptions_abbr;
    if constexpr (!std::is_floating_point_v<T>) {
        throw RtError("find_intersection: finding intersections on integer "
                      "vectors is not defined (perhaps a future feature?).");
    }
    static const Vector2<T> k_no_intersection = get_no_solution_sentinel<Vector2<T>>();

    auto p = a_first;
    auto r = a_second - p;

    auto q = b_first ;
    auto s = b_second - q;

    // http://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
    // two points of early failure
    auto r_cross_s = cross(r, s);
    if (r_cross_s == T(0)) return k_no_intersection;

    auto q_sub_p = q - p;
    auto t = cross(q_sub_p, s) / r_cross_s;

    if (t < T(0) || t > T(1)) return k_no_intersection;

    auto u = cross(q_sub_p, r) / r_cross_s;
    if (u < T(0) || u > T(1)) return k_no_intersection;

    return p + t*r;
}

// still in cul::detail namespace

template <typename T>
std::enable_if_t<std::is_floating_point_v<T>, std::tuple<Vector2<T>,Vector2<T>>>
    find_velocities_to_target
    (const Vector2<T> & source, const Vector2<T> & target,
     const Vector2<T> & influencing_acceleration, T speed)
{
    // Made possible by this wonderful resource:
    // https://www.forrestthewoods.com/blog/solving_ballistic_trajectories/
    // Thank you for releasing your work (demo source code specifically) under
    // the public domain Forrest Smith

    // note: I plan on relocating many of my utils to the commons library
    //       in order to release them under a more permissive license
    using Vec = Vector2<T>;
    static constexpr const T k_error = 0.00025;
    static auto are_very_close_v = [](const Vec & a, const Vec & b)
        { return are_within(a, b, k_error); };
    static auto are_very_close_s = [](const T & a, const T & b)
        { return are_within(a, b, k_error); };
    static const auto k_no_solution = std::make_tuple(
        get_no_solution_sentinel<Vector2<T>>(), get_no_solution_sentinel<Vector2<T>>());
    using std::make_tuple;
    if (   !is_real(source) || !is_real(speed)
        || !is_real(target) || !is_real(influencing_acceleration))
    {
        throw std::invalid_argument("compute_velocities_to_target: All "
                                    "arguments must be real numbers.");
    }
    if (are_very_close_v(source, target)) {
        if (are_very_close_v(influencing_acceleration, Vec())) {
            return make_tuple(Vec(), Vec());
        }
        // return straight up trajectory
        auto s = -normalize(influencing_acceleration)*speed;
        return make_tuple(s, s);
    }
    if (are_very_close_v(influencing_acceleration, Vec())) {
        // return straight to target
        auto s = normalize(target - source)*speed;
        return make_tuple(s, s);
    }

    // assumptions at this point source != target && acc != 0
    static auto are_parallel = [](Vec a, Vec b)
        { return are_very_close_v(normalize(a), normalize(b)); };
    static auto comp_from_basis = [](Vec a, Vec basis) {
        auto proj = project_onto(a, basis);
        return magnitude(proj)*( are_parallel(proj, basis) ? T(1) : T(-1) );
    };

    auto j = -normalize(influencing_acceleration);
    auto i = rotate_vector(j, k_pi_for_type<T>*0.5);

    T t0, t1;
    {
        auto diff_i = magnitude(project_onto(target - source, i));
        auto diff_j = comp_from_basis(target - source, j);

        auto spd_sq = speed*speed;
        auto g = magnitude(influencing_acceleration);
        auto do_atan_with_sqpart = [spd_sq, g, diff_i] (T sqpart)
            { return std::atan( (spd_sq + sqpart) / (g*diff_i) ); };

        auto randicand = spd_sq*spd_sq - g*(g*diff_i*diff_i + T(2)*spd_sq*diff_j);
        if (randicand < T(0)) return k_no_solution;
        auto sqpart = std::sqrt(randicand);
        t0 = do_atan_with_sqpart( sqpart);
        t1 = do_atan_with_sqpart(-sqpart);

    }
    auto ground_dir = normalize(project_onto(target - source, i));
    auto up         = -normalize(influencing_acceleration);

    auto s0 = ground_dir*std::cos(t0)*speed + up*std::sin(t0)*speed;
    if (are_very_close_s(t0, t1)) { return make_tuple(s0, s0); }
    return make_tuple(s0, ground_dir*std::cos(t1)*speed + up*std::sin(t1)*speed);
}

template <typename T>
Vector2<T> find_closest_point_to_line
    (const Vector2<T> & a, const Vector2<T> & b,
     const Vector2<T> & external_point)
{
    using Vec = Vector2<T>;
    const auto & c = external_point;
    if (a == b) return a;
    if (a - c == Vec()) return a;
    if (b - c == Vec()) return b;
    // obtuse angles -> snap to extreme points
    auto angle_at_a = angle_between(a - b, a - c);
    if (angle_at_a > k_pi_for_type<T>*0.5) return a;

    auto angle_at_b = angle_between(b - a, b - c);
    if (angle_at_b > k_pi_for_type<T>*0.5) return b;

    // https://www.eecs.umich.edu/courses/eecs380/HANDOUTS/PROJ2/LinePoint.html
    T mag = [&a, &b, &c]() {
        auto num = (c.x - a.x)*(b.x - a.x) + (c.y - a.y)*(b.y - a.y);
        auto denom = magnitude(b - a);
        return num / (denom*denom);
    } ();
    return Vec(a.x, a.y) + mag*Vec(b.x - a.x, b.y - a.y);
}

template <typename T>
bool is_inside_triangle
    (const Vector2<T> & a, const Vector2<T> & b, const Vector2<T> & c,
     const Vector2<T> & test_point)
{
    // derived from mathematics presented here:
    // https://blackpawn.com/texts/pointinpoly/default.html
    const auto & p = test_point;
    // convert to Barycentric cordinates
    auto ca = c - a;
    auto ba = b - a;
    auto pa = p - a;

    auto dot_caca = dot(ca, ca);
    auto dot_caba = dot(ca, ba);
    auto dot_capa = dot(ca, pa);
    auto dot_baba = dot(ba, ba);
    auto dot_bapa = dot(ba, pa);

    auto denom = dot_caca*dot_baba - dot_caba*dot_caba;
    auto u = dot_baba*dot_capa - dot_caba*dot_bapa;
    auto v = dot_caca*dot_bapa - dot_caba*dot_capa;

    return u >= 0 && v >= 0 && (u + v < denom);
}

} // end of detail namespace -> into ::cul
#endif
} // end of cul namespace
