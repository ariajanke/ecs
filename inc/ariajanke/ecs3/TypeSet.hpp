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

namespace ecs {

template <typename ... Types>
class TypeSet {
    static_assert(sizeof...(Types) == 0, "Should be zero at primary.");

    template <typename T>
    struct Contains_ {
        static constexpr bool k_value = false;
    };

    template <typename ... OtherTypes>
    struct Union_ {
        using Set = TypeSet<OtherTypes...>;
    };

    template <typename ... OtherTypes>
    struct Union_<TypeSet<OtherTypes...>>:
        public Union_<OtherTypes...> {};

    template <typename ... OtherTypes>
    struct Intersection_ {
        using Set = TypeSet<>;
    };

    template <typename ... OtherTypes>
    struct Difference_ {
        using Set = TypeSet<>;
    };

    template <typename T>
    struct WithoutType_ {
        using Set = TypeSet<>;
    };

    template <typename T>
    struct SetWithHead_ {
        using Set = TypeSet<T>;
    };

    template <typename ... OtherTypes>
    struct ContainsAllOthers_ {
        static constexpr bool k_value = true;
    };

    template <typename OtherHead, typename ... OtherTypes>
    struct ContainsAllOthers_<OtherHead, OtherTypes...> {
        static constexpr bool k_value = false;
    };

    template <typename ... OtherTypes>
    struct ContainsAllOthers_<TypeSet<OtherTypes...>> :
        public ContainsAllOthers_<OtherTypes...> {};

public:
    // interface part

    template <typename T>
    static constexpr bool kt_contains = Contains_<T>::k_value;

    template <typename ... OtherTypes>
    using Difference = TypeSet<>;

    template <typename ... OtherTypes>
    static constexpr bool kt_equal_to_set = ContainsAllOthers_<OtherTypes...>::k_value;

    template <typename ... OtherTypes>
    using Intersection = typename Intersection_<OtherTypes...>::Set;

    template <typename T>
    static constexpr int kt_occurance_count = 0;

    template <template <typename> class PredicateTemplate>
    using RemoveIf = TypeSet<>;

    template <typename T>
    using SetWithHead = typename SetWithHead_<T>::Set;

    using TailSet = TypeSet<>;

    template <template <typename> class Transformer>
    using Transform = TypeSet<>;

    template <typename ... OtherTypes>
    using Union = typename Union_<OtherTypes...>::Set;

    template <typename T>
    using WithoutType = typename WithoutType_<T>::Set;
};

// no ordering of types can be done so many operations become O(n^2)
template <typename Head, typename ... Types>
class TypeSet<Head, Types...> {
    using TailSet_ = TypeSet<Types...>;

    template <typename T>
    static constexpr bool kt_contains_ =
        std::is_same_v<Head, T> ||
        TailSet_::template kt_contains<T>;

    template <typename T>
    struct SetWithHead_ {
        static_assert(!kt_contains_<T>);
        using Set = TypeSet<T, Head, Types...>;
    };

    template <typename T>
    struct WithoutType_ {
        using Set = std::conditional_t<
            std::is_same_v<T, Head>,
            TailSet_,
            typename TailSet_::template WithoutType<T>::template SetWithHead<Head> >;
    };

    template <typename ... OtherTypes>
    struct Union_ {
        using Set = TypeSet<Head, Types...>;
    };

    template <typename ... OtherTypes>
    struct Intersection_ {
        using Set = TypeSet<>;
    };

    template <typename ... OtherTypes>
    struct Difference_ {
        using Set = TypeSet<Head, Types...>;
    };

    template <typename OtherHead, typename ... OtherTypes>
    struct Union_<OtherHead, OtherTypes...> {
        // the set must have types are all unique
        using Set = typename WithoutType_<OtherHead>::Set
            ::template Union<OtherTypes...>
            ::template SetWithHead<OtherHead>;
    };

    template <typename ... OtherTypes>
    struct Union_<TypeSet<OtherTypes...>> :
        public Union_<OtherTypes...> {};

    template <typename OtherHead, typename ... OtherTypes>
    struct Intersection_<OtherHead, OtherTypes...> {
        using SubIntersection = typename Intersection_<OtherTypes...>::Set;
        using Set = std::conditional_t<kt_contains_<OtherHead>,
            typename SubIntersection::template SetWithHead<OtherHead>,
            SubIntersection>;
    };

    template <typename OtherHead, typename ... OtherTypes>
    struct Difference_<OtherHead, OtherTypes...> {
        // we want to remove the Other* types
        using Set = typename WithoutType_<OtherHead>::Set
            ::template Difference<OtherTypes...>;
    };

    template <typename ... OtherTypes>
    struct Difference_<TypeSet<OtherTypes...>> :
        public Difference_<OtherTypes...> {};

    template <typename ... OtherTypes>
    struct Intersection_< TypeSet<OtherTypes...> > :
        public Intersection_<OtherTypes...> {};

    template <typename ... OtherTypes>
    struct ContainsAllOthers_ {
        static constexpr bool k_value = true;
    };

    template <typename OtherHead, typename ... OtherTypes>
    struct ContainsAllOthers_<OtherHead, OtherTypes...> {
        static constexpr bool k_value = kt_contains_<OtherHead>
            && ContainsAllOthers_<OtherTypes...>::k_value;
    };

    template <typename ... OtherTypes>
    struct ContainsAllOthers_<TypeSet<OtherTypes...>> :
        public ContainsAllOthers_<OtherTypes...> {};

    template <typename ... OtherTypes>
    struct EqualInSize_ {
        static constexpr bool k_value =
            sizeof...(OtherTypes) == sizeof...(Types) + 1;
    };

    template <typename ... OtherTypes>
    struct EqualInSize_<TypeSet<OtherTypes...>> :
        public EqualInSize_<OtherTypes...> {};

    template <template <typename> class PredicateTemplate>
    struct RemoveIf_ {
        static constexpr const bool k_should_remove_head = std::is_base_of_v<std::true_type, PredicateTemplate<Head>>;
        using RiTailSet = typename TailSet_::template RemoveIf<PredicateTemplate>;
        using Set = std::conditional_t<k_should_remove_head,
            RiTailSet,
            typename RiTailSet::template SetWithHead<Head>
        >;
    };

public:
    template <typename T>
    static constexpr bool kt_contains = kt_contains_<T>;

    template <typename ... OtherTypes>
    using Difference = typename Difference_<OtherTypes...>::Set;

    template <typename ... OtherTypes>
    static constexpr bool kt_equal_to_set =
           ContainsAllOthers_<OtherTypes...>::k_value
        && EqualInSize_<OtherTypes...>::k_value;

    template <typename ... OtherTypes>
    using Intersection = typename Intersection_<OtherTypes...>::Set;

    template <template <typename> class PredicateTemplate>
    using RemoveIf = typename RemoveIf_<PredicateTemplate>::Set;

    template <typename T>
    using SetWithHead = typename SetWithHead_<T>::Set;

    using TailSet = TailSet_;

    template <template <typename> class Transformer>
    using Transform = typename TailSet_::template Transform<Transformer>
        ::template SetWithHead<Transformer<Head>>;

    template <typename ... OtherTypes>
    using Union = typename Union_<OtherTypes...>::Set;

    template <typename T>
    using WithoutType = typename WithoutType_<T>::Set;

    template <typename T>
    static constexpr int kt_occurance_count =
        kt_contains<T> ? 1 : 0 +
        TailSet::template kt_occurance_count<T>;

    static_assert(!TailSet::template kt_contains<Head>,
                  "TypeSet must contain a unique set of types.");
};

class TypeSetTestsPriv {
    struct A final {};
    struct B final {};
    struct C final {};
    struct D final {};
    struct E final {};
    struct F final {};

    TypeSetTestsPriv() {}
#   ifdef MACRO_ARIAJANKE_ECS3_ENABLE_TYPESET_TESTS

    // must work on empties, lists of types, and TypeSets
    // make sure product sets only have one of each type
    static_assert(!TypeSet<>::kt_contains<A>);
    static_assert(!TypeSet<>::kt_contains<F>);

    static_assert(TypeSet<A, B, C>::kt_occurance_count<C> == 1);
    static_assert(TypeSet<A, B, C>::Transform<std::add_pointer_t>::kt_equal_to_set<A *, B *, C *>);
    static_assert( TypeSet<A, B, C>::kt_contains<A>);
    static_assert(!TypeSet<A, B, C>::kt_contains<F>);

    static_assert( TypeSet<>::kt_equal_to_set<>);
    static_assert( TypeSet<A, B, C>::kt_equal_to_set<B, C, A>);
    static_assert(!TypeSet<A, B, C>::kt_equal_to_set<B, D, A>);
    static_assert( TypeSet<A, B, C>::kt_equal_to_set<C, B, A>);

    static_assert(TypeSet<A, B, C>::kt_equal_to_set<TypeSet<A, B, C>>);
    static_assert(!TypeSet<A, B, C>::kt_equal_to_set<A, B, C, D>);
    static_assert(TypeSet<A, D, B, C>::kt_equal_to_set<A, B, C, D>);
    static_assert(TypeSet<>::kt_equal_to_set<>);

    static_assert(!TypeSet<A, B, C, D>::Difference<A, B>::kt_contains<A>);
    static_assert( TypeSet<A, B, C, D>::Difference<A, B>::kt_contains<C>);

    static_assert( TypeSet<C, B, A, D>::Difference<A, B>::kt_contains<C>);
    static_assert(!TypeSet<C, B, A, D>::Difference<A, C>::kt_contains<C>);
    static_assert(!TypeSet<C, B, A, D>::Difference<A, C>::kt_contains<A>);
    static_assert( TypeSet<C, B, A, D>::Difference<A, C>::kt_contains<D>);
    static_assert( TypeSet<C, B, A, D>::Difference<A, C>::kt_contains<B>);
    static_assert(!TypeSet<C, B, A, D>::Difference<A, C>::kt_contains<F>);

    static_assert(TypeSet<A, B, C, D>::Difference<A, B>::kt_equal_to_set<C, D>);
    static_assert(TypeSet<A, B, C, D>::Difference<TypeSet<A, B>>::kt_equal_to_set<C, D>);

    static_assert( TypeSet<A, B, C>::Intersection<B>::kt_contains<B>);
    static_assert( TypeSet<A, B, C>::Intersection<B, A>::kt_contains<A>);

    static_assert(TypeSet<A, B>::kt_contains<B>);
    static_assert(TypeSet<A, B>::Union<B>::Union<A>::kt_occurance_count<A> == 1);
    static_assert(TypeSet<A, B>::Union<B>::Union<A>::kt_occurance_count<B> == 1);

    static_assert(TypeSet<A, C>::Union<TypeSet<A, B>>::kt_equal_to_set<A, B, C>);
#   endif // MACRO_ARIAJANKE_ECS3_ENABLE_TYPESET_TESTS
};

} // end of ecs namespace
