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
#include <utility>

#include <ariajanke/ecs3/FunctionTraits.hpp>

#include <common/TypeSet.hpp>

namespace ecs {

template <typename Type>
class Optional;

template <typename T>
struct StripOpt_ {
    using Type = T;
};

template <typename T>
struct StripOpt_<Optional<T>> {
    using Type = T;
};

template <typename T>
struct IsAnOptionalType : std::false_type {};

template <typename T>
struct IsAnOptionalType<Optional<T>> : std::true_type {};

template <typename T>
using StripOptional = typename StripOpt_<T>::Type;

class Uofa_ {
    template <typename ... Types>
    struct UofaImpl_ {
        using Set = cul::TypeSet<>;
    };

    template <typename Func, typename ... Types>
    struct UofaImpl_<Func, Types...> {
        using Set = typename FunctionTraitsOf<Func>::ArgumentTypeSet
            ::template Union<typename UofaImpl_<Types...>::Set>;
    };

public:
    template <typename ... Types>
    using UnionOfFunctorArguments = typename UofaImpl_<Types...>::Set;
};

template <typename EntityType>
class SingleSystemBase;

template <typename ... FullUnionTypes>
class SingleSystemsGenerator {
public:
    // It's not complete...
    // "Opt" has to be stripped to some degree...
    // They have to be transformed into pointers or something
    //
    // we should test retrieving components one at a time as functors are
    // called, remember what *I think* is faster could very well be slower!
    // Without testing, there is no knowing!

    template <typename ... Types>
    struct TypeSetAsTuple {
        using Type = Tuple<Types...>;
    };

    template <typename ... Types>
    struct TypeSetAsTuple<cul::TypeSet<Types...>> :
        public TypeSetAsTuple<Types...> {};

    using FullUnionTuple = typename TypeSetAsTuple<
        typename cul::TypeSet<FullUnionTypes...>
        ::template Transform<std::add_pointer_t>
    >::Type;

    template <typename EntityType, typename ... OtherFuncs>
    class SysLayer : public SingleSystemBase<EntityType> {
    protected:
        using ArgSet = cul::TypeSet<>;

        // bottom layer needs to know the union type
        SysLayer(OtherFuncs && ...) {}

        void do_mine(const FullUnionTuple &) const {}
    };

    template <typename EntityType, typename Func, typename ... OtherFuncs>
    class SysLayer<EntityType, Func, OtherFuncs...> :
        public SysLayer<EntityType, OtherFuncs...>
    {
    protected:
        using Super = SysLayer<EntityType, OtherFuncs...>;
        using Traits = FunctionTraitsOf<Func>;
        using ArgSet = typename Traits::ArgumentTypeSet;
        using RequiredArguments = typename ArgSet::template RemoveIf<IsAnOptionalType>;

        template <typename ... Types>
        struct HasRequiredTypes {
            bool operator () (const FullUnionTuple &) const noexcept { return true; };
        };

        template <typename Head, typename ... Types>
        struct HasRequiredTypes<Head, Types...> {
            using HeadPtr = std::add_pointer_t<Head>;
            bool operator () (const FullUnionTuple & tup) const noexcept {
                return !!std::get<HeadPtr>(tup)
                       && HasRequiredTypes<Types...>{}(tup);
            };
        };

        template <typename ... Types>
        struct HasRequiredTypes<cul::TypeSet<Types...>> :
            public HasRequiredTypes<Types...>
        {
            using HasRequiredTypes<Types...>::operator();
        };

        template <typename ... RemainingTypes>
        struct Adapter {
            template <typename ... ArgTypes>
            void operator () (const Func & f_, const FullUnionTuple &, ArgTypes && ... args) const
                { f_(std::forward<ArgTypes>(args)...); }
        };

        template <typename Head, typename ... RemainingTypes>
        struct Adapter<Head, RemainingTypes...>:
            public Adapter<RemainingTypes...>
        {
            using OptType = IsAnOptionalType<std::remove_reference_t<Head>>;
            using HeadPtr = std::add_pointer_t<StripOptional<std::remove_reference_t<Head>>>;

            template <typename ... ArgTypes>
            void operator () (const Func & f_, const FullUnionTuple & tup, ArgTypes && ... args) const {
                Adapter<RemainingTypes...>{}(f_, tup,
                    std::forward<ArgTypes>(args)...,
                    get_head(tup, OptType{}));
            }

            static Head & get_head
                (const FullUnionTuple & tup, std::false_type)
            { return *std::get<HeadPtr>(tup); }

            static Head get_head
                (const FullUnionTuple & tup, std::true_type)
            { return Head{std::get<HeadPtr>(tup)}; }
        };

        template <typename ... RemainingTypes>
        struct Adapter<cul::TypeSet<RemainingTypes...>>:
            public Adapter<RemainingTypes...>
        {
            template <typename ... ArgTypes>
            void operator () (const Func & f_, const FullUnionTuple & tup) const
                { Adapter<RemainingTypes...>{}(f_, tup); }
        };

        SysLayer(Func && f_, OtherFuncs && ... others):
            Super(std::forward<OtherFuncs>(others)...),
            f(std::move(f_)) {}

        // top level needs the intersection from each functor argument types

        void do_mine(const FullUnionTuple & tup) const {
            // what about wittling down here from deriveds?
            if (HasRequiredTypes<RequiredArguments>{}(tup)) {
                Adapter<ArgSet>{}(f, tup);
            }
            Super::do_mine(tup);
        }

    private:
        Func f;
    };

    template <typename EntityType, typename ... Types>
    struct EntityAdapter {
        Tuple<> operator () (EntityType &) const { return Tuple<>{}; }
    };

    template <typename EntityType, typename OneType, typename ... Types>
    struct EntityAdapter<EntityType, OneType, Types...> {
        auto operator () (EntityType & ent) const
            { return std::make_tuple( ent.template ptr<OneType>() ); }
    };

    template <typename EntityType, typename OneType, typename TwoType, typename ... Types>
    struct EntityAdapter<EntityType, OneType, TwoType, Types...> {
        auto operator () (EntityType & ent) const
            { return ent.template ptr<OneType, TwoType, Types...>(); }
    };

    template <typename EntityType, typename ... Funcs>
    class SingleSystem final : public SysLayer<EntityType, Funcs...> {
        using Super = SysLayer<EntityType, Funcs...>;
    public:
        SingleSystem(Funcs && ... funcs):
            Super(std::forward<Funcs>(funcs)...) {}

        void operate(EntityType & ent) const final {
            // intersection reject here?
            // "TypeSet" should not be a type in the full type union! (uh oh)
            // also making a blank system should be possible
            // also should be made to work on a single type
            Super::do_mine(EntityAdapter<EntityType, FullUnionTypes...>{}(ent));
        }
    };
};

template <typename ... AllFunctorArgumentTypes>
class SingleSystemsGenerator<cul::TypeSet<AllFunctorArgumentTypes...>> :
    public SingleSystemsGenerator<AllFunctorArgumentTypes...> {};

template <typename ... Types>
using UnionOfFunctorArguments = Uofa_::UnionOfFunctorArguments<Types...>;

template <typename EntityType, typename ... Functors>
class SinglesSystemFromFunctors_ {
    using RequiredTypes = typename UnionOfFunctorArguments<Functors...>::template RemoveIf<IsAnOptionalType>;
    using OptionalTypes = typename UnionOfFunctorArguments<Functors...>::template Difference<RequiredTypes>;
    using FullUnion     = typename OptionalTypes::template Transform<StripOptional>
        ::template Union<
            typename RequiredTypes::template Transform<std::remove_reference_t>
        >;
public:
    using Type = typename SingleSystemsGenerator<FullUnion>
        ::template SingleSystem<EntityType, Functors...>;
};

} // end of ecs namespace
