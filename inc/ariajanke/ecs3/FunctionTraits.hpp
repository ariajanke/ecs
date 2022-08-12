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

#include <type_traits>

#include <ariajanke/ecs3/defs.hpp>

#include <common/TypeSet.hpp>

namespace ecs {

/** Work derived from this MIT licensed header, Copyright (c) 2017 aminroosta.
 *  https://github.com/aminroosta/sqlite_modern_cpp/blob/master/hdr/sqlite_modern_cpp/utility/function_traits.h
 *
 */
class FuncPriv final {
    FuncPriv() {}

    template <typename ... Types>
    friend class FunctionTraits;

    template <typename>
    struct FuncTraitsImpl;

    template <typename FunctorType>
    struct FuncTraitsImpl :
        public FuncTraitsImpl<decltype(&std::remove_reference_t<FunctorType>::operator())> {};

    template <typename FunctorType, typename ReturnType_, typename ... Types>
    struct FuncTraitsImpl<ReturnType_(FunctorType::*)(Types ...) const> :
        public FuncTraitsImpl<ReturnType_(Types...)>
    {};

    template <typename FunctorType, typename ReturnType_, typename ... Types>
    struct FuncTraitsImpl<ReturnType_(FunctorType::*)(Types ...)> :
        public FuncTraitsImpl<ReturnType_(Types...)>
    {};

    template <typename ReturnType_, typename ... Types>
    struct FuncTraitsImpl<ReturnType_(&)(Types ...)> :
        public FuncTraitsImpl<ReturnType_(Types...)> {};

    template <typename ReturnType_, typename ... Types>
    struct FuncTraitsImpl<ReturnType_(*)(Types ...)> :
        public FuncTraitsImpl<ReturnType_(Types...)> {};

    template <typename ReturnType_, typename ... Types>
    struct FuncTraitsImpl<ReturnType_(Types ...)> {
        using ArgumentTypes   = TypeList<Types...>;
        using ReturnType      = ReturnType_;
        using ArgumentTypeSet = cul::TypeSet<Types...>;

        static constexpr const int k_argument_count = sizeof...(Types);
    };

};

template <typename ... Types>
class FunctionTraits final {
    FunctionTraits() {}
public:
    using Traits = FuncPriv::FuncTraitsImpl<Types...>;
};

template <typename ... Types>
using FunctionTraitsOf = typename FunctionTraits<Types...>::Traits;

} // end of ecs namespace
