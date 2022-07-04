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

#include <common/Vector2Traits.hpp>

namespace cul {

template <typename T>
struct Vector2 {
    constexpr Vector2() {}
    constexpr Vector2(T x_, T y_): x(x_), y(y_) {}

    template <typename U>
    constexpr explicit Vector2(U x_, U y_): x(T(x_)), y(T(y_)) {}

    template <typename U>
    constexpr explicit Vector2(const Vector2<U> & r): x(T(r.x)), y(T(r.y)) {}

    T x = 0, y = 0;
};

template <typename T>
struct Size2 {
    constexpr Size2() {}

    constexpr Size2(T width_, T height_):
        width(width_), height(height_)
    {}

    template <typename U>
    constexpr explicit Size2(U width_, U height_): width(T(width_)), height(T(height_)) {}

    template <typename U>
    constexpr explicit Size2(const Size2<U> & r):
        width(T(r.width)), height(T(r.height))
    {}

    T width = 0, height = 0;
};

/** Representation of a rectangle in four scalars or two vectors (with one
 *  being a "size" vector.
 */
template <typename T>
struct Rectangle {
    constexpr Rectangle() {}

    constexpr Rectangle(T left_, T top_, T width_, T height_):
        left(left_), top(top_), width(width_), height(height_)
    {}

    constexpr Rectangle(const Vector2<T> & r, const Size2<T> & sz):
        left (     r.x), top   (      r.y),
        width(sz.width), height(sz.height)
    {}

    template <typename U>
    constexpr explicit Rectangle(const Rectangle<U> & rect):
        left (T(rect.left )), top   (T(rect.top   )),
        width(T(rect.width)), height(T(rect.height))
    {}

    T left = 0, top = 0, width = 0, height = 0;
};

/** @returns true if the vector r is contained in the rectangle rect.
 *
 *  This is an "STL" like boundry check. The right and bottom end of the
 *  rectangle (equal to position + size) is considered out of bounds. The left
 *  and top are inside.
 *
 *  @param r position to be tested
 *  @param rect rectangle to check if r is contained in
 */
template <typename T>
bool is_contained_in(const Vector2<T> & r, const Rectangle<T> & rect) noexcept {
    return    r.x >=  rect.left               && r.y >=  rect.top
           && r.x <  (rect.left + rect.width) && r.y <  (rect.top + rect.height);
}

// ------- trait definitions for Vector2 and Size2, which maybe ignored -------

template <typename T>
struct Vector2Scalar<Vector2<T>> {
    using Type = T;
};

template <typename T>
struct Vector2Traits<T, Vector2<T>> {
    static constexpr const bool k_is_vector_type          = true;
    static constexpr const bool k_should_define_operators = true;

    struct GetX {
        constexpr T operator () (const Vector2<T> & r) const { return r.x; }
    };
    struct GetY {
        constexpr T operator () (const Vector2<T> & r) const { return r.y; }
    };
    struct Make {
        constexpr Vector2<T> operator () (const T & x_, const T & y_) const
            { return Vector2<T>{x_, y_}; }
    };
};

template <typename T>
struct Vector2Scalar<Size2<T>> {
    using Type = T;
};

template <typename T>
struct Vector2Traits<T, Size2<T>> {
    static constexpr const bool k_is_vector_type          = true;
    static constexpr const bool k_should_define_operators = true;

    struct GetX {
        constexpr T operator () (const Size2<T> & r) const { return r.width; }
    };
    struct GetY {
        constexpr T operator () (const Size2<T> & r) const { return r.height; }
    };
    struct Make {
        constexpr Size2<T> operator () (const T & w_, const T & h_) const
            { return Size2<T>{w_, h_}; }
    };
};

template <typename T>
constexpr bool operator ==
    (const cul::Rectangle<T> & lhs, const cul::Rectangle<T> & rhs) noexcept
{
    return    lhs.left  == rhs.left  && rhs.top    == lhs.top
           && lhs.width == rhs.width && rhs.height == lhs.height;
}

template <typename T>
constexpr bool operator !=
    (const cul::Rectangle<T> & lhs, const cul::Rectangle<T> & rhs) noexcept
{ return !(lhs == rhs); }

} // end of cul namespace
