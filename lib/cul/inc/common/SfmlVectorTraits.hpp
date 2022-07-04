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

#include <SFML/System/Vector2.hpp>
#include <common/Vector2Traits.hpp>

namespace cul {

template <typename T>
struct Vector2Scalar<sf::Vector2<T>> {
    using Type = T;
};

template <typename T>
struct Vector2Traits<T, sf::Vector2<T>> {
    static constexpr const bool k_is_vector_type          = true;
    static constexpr const bool k_should_define_operators = false;

    struct GetX {
        T operator () (const sf::Vector2<T> & r) const { return r.x; }
    };
    struct GetY {
        T operator () (const sf::Vector2<T> & r) const { return r.y; }
    };
    struct Make {
        sf::Vector2<T> operator () (const T & x_, const T & y_) const
            { return sf::Vector2<T>{x_, y_}; }
    };
};

// future home for sf::Vector3 perhaps?

} // end of cul namespace
