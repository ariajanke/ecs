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

#include <type_traits>

namespace cul {

/** Your specialization should look something like this for template vector 
 *  types:
 *  @code

namespace cul {

template <typename T>
struct Vector2Scalar<MySize<T>> {
    using Type = T;
};

template <typename T>
struct Vector2Traits<T, MySize<T>> {
    struct GetX {
        constexpr T operator () (const MySize<T> & r) const { return r.width; }
    };
    struct GetY {
        constexpr T operator () (const MySize<T> & r) const { return r.height; }
    };
    struct Make {
        constexpr MySize<T> operator () (T w, T h) const {
            // note: allow for any needed rearrangement for construction of the
            //       object
            return MySize<T>(h, w);
        }
    };
    static constexpr const bool k_is_vector_type          = true;
    static constexpr const bool k_should_define_operators = true;
};

} // end of cul namespace
 * @endcode
 */

template <typename VectorType>
struct Vector2Scalar {
    using Type = void;
};

/**
 *  @tparam VectorType Must be default and copy constructible.
 */
template <typename ScalarType, typename VectorType>
struct Vector2Traits {
    /** When this constant is true, this tells the convert_to function that
     *  the vector type in question maybe converted to any other vector type
     */
    static constexpr const bool k_is_vector_type = false;
    
    /** When this constant is true, this will enable a number of vector 
     *  operator definitions including unary -, addition, subtraction, scalar
     *  multipication, scalar division, equality and inequality operators.
     */
    static constexpr const bool k_should_define_operators = false;
    
    /** For both this type and GetY the following two operators should be 
     *  defined:
     *  @code
T   operator () (const MyVector<T> & r) const { return r.x; }
T & operator () (      MyVector<T> & r) const { return r.x; }
     *  @endcode
     */
    struct GetX {};
    struct GetY {};
    struct Make {};
};

template <typename ScalarType, typename VectorType>
constexpr const bool k_is_vector2 
    = Vector2Traits<ScalarType, VectorType>::k_is_vector_type;

template <typename ScalarType, typename VectorType>
constexpr const bool k_should_define_vector2_operators
    = Vector2Traits<ScalarType, VectorType>::k_should_define_operators;

template <typename ScalarType, typename VectorType>
using EnableVector2Op = std::enable_if_t<
    k_should_define_vector2_operators<ScalarType, VectorType>,
VectorType>;

template <typename ScalarType, typename VectorType>
using EnableVector2BoolOp = std::enable_if_t<
    k_should_define_vector2_operators<ScalarType, VectorType>,
bool>;

// ---------------------------- Unary Vector Operator --------------------------

template <typename VectorType, 
          typename ScalarType = typename cul::Vector2Scalar<VectorType>::Type>
constexpr cul::EnableVector2Op<ScalarType, VectorType>
    operator - (const VectorType & r)
{
    using Tr = cul::Vector2Traits<ScalarType, VectorType>;
    typename Tr::GetX get_x{};
    typename Tr::GetY get_y{};
    return typename Tr::Make{}(-get_x(r), -get_y(r));
}

// -------------------------- Vector on Vector operators -----------------------


template <typename VectorType,
          typename ScalarType = typename cul::Vector2Scalar<VectorType>::Type>
cul::EnableVector2Op<ScalarType, VectorType> &
    operator += (VectorType & r, const VectorType & v)
{
    using Tr = cul::Vector2Traits<ScalarType, VectorType>;
    typename Tr::GetX get_x{};
    typename Tr::GetY get_y{};
    return (r = typename Tr::Make{}(get_x(r) + get_x(v), get_y(r) + get_y(v)));
}

template <typename VectorType,
          typename ScalarType = typename cul::Vector2Scalar<VectorType>::Type>
cul::EnableVector2Op<ScalarType, VectorType> &
    operator -= (VectorType & r, const VectorType & v)
{
    using Tr = cul::Vector2Traits<ScalarType, VectorType>;
    typename Tr::GetX get_x{};
    typename Tr::GetY get_y{};
    return (r = typename Tr::Make{}(get_x(r) - get_x(v), get_y(r) - get_y(v)));
}

template <typename VectorType,
          typename ScalarType = typename cul::Vector2Scalar<VectorType>::Type>
constexpr cul::EnableVector2Op<ScalarType, VectorType>
    operator + (const VectorType & r, const VectorType & v)
{
    using Tr = cul::Vector2Traits<ScalarType, VectorType>;
    typename Tr::GetX get_x{};
    typename Tr::GetY get_y{};
    return typename Tr::Make{}(get_x(r) + get_x(v), get_y(r) + get_y(v));
}

template <typename VectorType,
          typename ScalarType = typename cul::Vector2Scalar<VectorType>::Type>
constexpr cul::EnableVector2Op<ScalarType, VectorType>
    operator - (const VectorType & r, const VectorType & v)
{
    using Tr = cul::Vector2Traits<ScalarType, VectorType>;
    typename Tr::GetX get_x{};
    typename Tr::GetY get_y{};
    return typename Tr::Make{}(get_x(r) - get_x(v), get_y(r) - get_y(v));
}

// -------------------------- Vector on Scalar operators -----------------------

template <typename VectorType,
          typename ScalarType = typename cul::Vector2Scalar<VectorType>::Type>
cul::EnableVector2Op<ScalarType, VectorType> &
    operator *= (VectorType & r, const ScalarType & a)
{
    using Tr = cul::Vector2Traits<ScalarType, VectorType>;
    typename Tr::GetX get_x{};
    typename Tr::GetY get_y{};
    return (r = typename Tr::Make{}(get_x(r)*a, get_y(r)*a));
}

template <typename VectorType,
          typename ScalarType = typename cul::Vector2Scalar<VectorType>::Type>
cul::EnableVector2Op<ScalarType, VectorType> &
    operator /= (VectorType & r, const ScalarType & a)
{
    using Tr = cul::Vector2Traits<ScalarType, VectorType>;
    typename Tr::GetX get_x{};
    typename Tr::GetY get_y{};
    return (r = typename Tr::Make{}(get_x(r) / a, get_y(r) / a));
}

template <typename VectorType,
          typename ScalarType = typename cul::Vector2Scalar<VectorType>::Type>
constexpr cul::EnableVector2Op<ScalarType, VectorType>
    operator * (const VectorType & r, const ScalarType & a)
{
    using Tr = cul::Vector2Traits<ScalarType, VectorType>;
    typename Tr::GetX get_x{};
    typename Tr::GetY get_y{};
    return typename Tr::Make{}(get_x(r)*a, get_y(r)*a);
}

template <typename VectorType,
          typename ScalarType = typename cul::Vector2Scalar<VectorType>::Type>
constexpr cul::EnableVector2Op<ScalarType, VectorType>
    operator / (const VectorType & r, const ScalarType & a)
{
    using Tr = cul::Vector2Traits<ScalarType, VectorType>;
    typename Tr::GetX get_x{};
    typename Tr::GetY get_y{};
    return typename Tr::Make{}(get_x(r) / a, get_y(r) / a);
}

template <typename VectorType,
          typename ScalarType = typename cul::Vector2Scalar<VectorType>::Type>
constexpr cul::EnableVector2Op<ScalarType, VectorType>
    operator * (const ScalarType & a, const VectorType & r)
{
    using Tr = cul::Vector2Traits<ScalarType, VectorType>;
    typename Tr::GetX get_x{};
    typename Tr::GetY get_y{};
    return typename Tr::Make{}(a*get_x(r), a*get_y(r));
}

// ------------------------------- Vector equality -----------------------------

template <typename VectorType,
          typename ScalarType = typename cul::Vector2Scalar<VectorType>::Type>
constexpr cul::EnableVector2BoolOp<ScalarType, VectorType>
    operator ==
    (const VectorType & r, const VectorType & u)
{
    using Tr = cul::Vector2Traits<ScalarType, VectorType>;
    typename Tr::GetX get_x{};
    typename Tr::GetY get_y{};
    return get_x(r) == get_x(u) && get_y(r) == get_y(u);
}

template <typename VectorType,
          typename ScalarType = typename cul::Vector2Scalar<VectorType>::Type>
constexpr cul::EnableVector2BoolOp<ScalarType, VectorType>
    operator !=
    (const VectorType & r, const VectorType & u)
{ return !(r == u); }

// ---------------------------------- conversion -------------------------------

template <typename DestType, typename SourceType>
constexpr const bool k_both_types_conversion_suitible =
        k_is_vector2<typename cul::Vector2Scalar<SourceType>::Type, SourceType>
     && k_is_vector2<typename cul::Vector2Scalar<DestType  >::Type, DestType  >;

template <typename DestType, typename SourceType>
constexpr std::enable_if_t<
    k_both_types_conversion_suitible<DestType, SourceType>,
DestType> convert_to(const SourceType & r)
{
    using SourceScalar = typename cul::Vector2Scalar<SourceType>::Type;
    using DestScalar   = typename cul::Vector2Scalar<DestType  >::Type;
    using SourceTr     = cul::Vector2Traits<SourceScalar, SourceType>;
    using DestTr       = cul::Vector2Traits<DestScalar  , DestType  >;

    typename SourceTr::GetX get_sx{};
    typename SourceTr::GetY get_sy{};

    return typename DestTr::Make{}(get_sx(r), get_sy(r));
}

} // end of cul namespace
