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

#include <ariajanke/ecs3/ecs.hpp>

#include <ariajanke/cul/TestSuite.hpp>

#include <iostream>

template <typename ... Types>
using Tuple = std::tuple<Types...>;

template <typename T>
class Counted {
public:
    Counted() { ++s_count; }
    Counted(const Counted &) { ++s_count; }
    Counted(Counted &&) { ++s_count; }
    Counted & operator = (const Counted &) = default;
    Counted & operator = (Counted &&) = default;
    ~Counted() { --s_count; }

    static const int & count() { return s_count; }
    static void hard_reset() { s_count = 0; }
private:
    static int s_count;
};

template <typename T>
/* private static */ int Counted<T>::s_count = 0;

using cul::TypeList;

class AllTogether { AllTogether() {} };

using AllInst = Counted<AllTogether>;

struct A final : public Counted<A>, public AllInst {};
struct B final : public Counted<B>, public AllInst {};
struct C final : public Counted<C>, public AllInst {
    static constexpr bool k_quiet = true;
    static constexpr auto k_message = "hello";
    C() {
        if constexpr (!k_quiet)
            std::cout << "C constructed" << std::endl;
    }
    C(const C &) = delete;
    C(C && lhs): mem(std::move(lhs.mem)) {
        if constexpr (!k_quiet)
            std::cout << "C moved" << std::endl;
        lhs.mem.clear();
    }
    ~C() {
        if constexpr (!k_quiet)
            std::cout << "C destructed" << std::endl;
        i = 0xDEADBEEF;
    }

    C & operator = (const C &) = delete;
    C & operator = (C && lhs) {
        if constexpr (!k_quiet)
            std::cout << "C move assigned" << std::endl;
        mem.swap(lhs.mem);
        return *this;
    }

    std::string mem = k_message;
    int i = 0;
};
struct D final : public Counted<D>, public AllInst { std::array<int, 100> m; };
struct E final : public Counted<E>, public AllInst {
    E(float, bool, const char *) {}
};
struct F final : public Counted<F>, public AllInst {};

template <typename ... Types>
void reset_counts_on_(TypeList<Types...>)
    { AllInst::hard_reset(); }

template <typename Head, typename ... Types>
void reset_counts_on_(TypeList<Head, Types...>) {
    Counted<Head>::hard_reset();
    return reset_counts_on_(TypeList<Types...>{});
}

inline void reset_all_counts()
    { reset_counts_on_(TypeList<A, B, C, D, E, F>{}); }

namespace ecs {
#if 1
template <>
struct MetaFunctionPreferredId<A> final {
    static constexpr const int k_preferred_id = 1;
};

template <>
struct MetaFunctionPreferredId<B> final {
    static constexpr const int k_preferred_id = 2;
};

template <>
struct MetaFunctionPreferredId<C> final {
    static constexpr const int k_preferred_id = 3;
};

template <>
struct MetaFunctionPreferredId<D> final {
    static constexpr const int k_preferred_id = 4;
};

template <>
struct MetaFunctionPreferredId<E> final {
    static constexpr const int k_preferred_id = 5;
};

template <>
struct MetaFunctionPreferredId<F> final {
    static constexpr const int k_preferred_id = 6;
};
#endif
template <>
struct MetaFunctionPreferredName<A> final {
    static constexpr const auto k_name = "Component A";
};

template <>
struct MetaFunctionPreferredName<B> final {
    static constexpr const auto k_name = "Component B";
};

template <>
struct MetaFunctionPreferredName<C> final {
    static constexpr const auto k_name = "Component C";
};

template <>
struct MetaFunctionPreferredName<D> final {
    static constexpr const auto k_name = "Component D";
};

template <>
struct MetaFunctionPreferredName<E> final {
    static constexpr const auto k_name = "Component E";
};

template <>
struct MetaFunctionPreferredName<F> final {
    static constexpr const auto k_name = "Component F";
};

} // end of ecs_rev3 namespace

constexpr auto k_a_key = ecs::MetaFunctionPreferredId<A>::k_preferred_id;
constexpr auto k_b_key = ecs::MetaFunctionPreferredId<B>::k_preferred_id;
constexpr auto k_c_key = ecs::MetaFunctionPreferredId<C>::k_preferred_id;
constexpr auto k_d_key = ecs::MetaFunctionPreferredId<D>::k_preferred_id;
constexpr auto k_e_key = ecs::MetaFunctionPreferredId<E>::k_preferred_id;
constexpr auto k_f_key = ecs::MetaFunctionPreferredId<F>::k_preferred_id;

using namespace cul::ts;
using namespace cul::exceptions_abbr;

template <typename EntityType>
const char * k_name_for_entity_tests = "<NO NAME :c!!>";

template <typename EntityType>
bool run_tests_for_entity_type();

template <typename EntityType>
bool test_interface();

template <typename EntityType>
bool test_scene();

template <typename EntityType>
bool test_reftypes();

// testing internals

bool test_hashtableentity();

bool test_avltreeentity();

template <typename ExcpType, typename F>
bool should_throw(F && f) {
    try {
        f();
    } catch (ExcpType &) {
        return true;
    }
    return false;
}

template <typename ... Types>
bool andf(Types && ...) { return true; }

template <typename ... Types>
bool andf(bool val, Types && ... args) {
    bool gv = andf(std::forward<Types>(args)...);
    return val && gv;
}

// ----------------------------------------------------------------------------

#define mark MACRO_MARK_POSITION_OF_CUL_TEST_SUITE

template <typename EntityType>
bool run_tests_for_entity_type() {
    return andf(test_interface<EntityType>(),
                test_scene<EntityType>(),
                test_reftypes<EntityType>());
}

template <typename EntityType>
bool test_interface() {
    using std::get;
    TestSuite suite;
    const auto suite_name = std::string{"Interface of "} + k_name_for_entity_tests<EntityType>;
    suite.start_series(suite_name.c_str());
#   if 0
    auto & a = Counted<A>::count();
    auto & b = Counted<B>::count();
    auto & c = Counted<C>::count();
    auto & d = Counted<D>::count();
    auto & e = Counted<E>::count();
    auto & all = AllInst::count();
#   endif

    // --------------------------------- add ----------------------------------

    // C references a space which is then thrown away before returning here:
    // add verb adds components
    mark(suite).test([] {
        EntityType e = EntityType::make_sceneless_entity();
        Tuple<A &, B &, C &> gv = e.template add<A, B, C>();
        e.template add<E>(9.f, true, "hello");
        return test(Counted<A>::count() == 1 && Counted<B>::count() == 1 &&
                    Counted<C>::count() == 1 && Counted<E>::count() == 1 &&
                    AllInst::count() == 4);
    });
    reset_all_counts();

    // add must properly call constructors
    mark(suite).test([] {
        EntityType e = EntityType::make_sceneless_entity();
        auto gv = e.template add<B, C>();
        return test( get<C &>(gv).mem == C::k_message );
    });
    reset_all_counts();

    // add with arguments
    mark(suite).test([] {
        auto e = EntityType::make_sceneless_entity();
        e.template add<E>(0.f, true, "");
        return test(AllInst::count() == 1 && Counted<E>::count() == 1);
    });
    reset_all_counts();

    // -------------------------------- ensure -------------------------------

    mark(suite).test([] {
        auto e = EntityType::make_sceneless_entity();
        e.template add<C>();
        e.template ensure<C>();
        e.template ensure<D, B>();
        return test(    Counted<D>::count() == 1 && Counted<C>::count() == 1
                     && AllInst::count() == 3 );
    });

    // --------------------------------- get ----------------------------------

    mark(suite).test([] {
        auto e = EntityType::make_sceneless_entity();
        e.template add<A, B, C>();
        C & c = e.template get<C>();
        const auto & [a, b] = e.template get<A, B>(); {}
        static_assert(std::is_same_v<decltype(a), A &>, "Get returns incorrect type");
        static_assert(std::is_same_v<decltype(b), B &>, "Get returns incorrect type");
        return test(c.mem == C::k_message);
    });

    mark(suite).test([] {
        auto e = EntityType::make_sceneless_entity();
        return test(should_throw<RtError>([&e] {
            e.template get<A>();
        }));
    });

    mark(suite).test([] {
        auto e = EntityType::make_sceneless_entity();
        e.template add<B>();
        return test(should_throw<RtError>([&e] {
            e.template get<A>();
        }));
    });

    // const entity reference
    mark(suite).test([] {
        auto e = EntityType::make_sceneless_entity();
        const auto & cref = e;
        e.template add<A, B, C>();
        const C & c = cref.template get<C>();
        const auto & [a, b] = cref.template get<A, B>(); {}
        static_assert(std::is_same_v<decltype(a), const A &>, "Get returns incorrect type");
        static_assert(std::is_same_v<decltype(b), const B &>, "Get returns incorrect type");
        return test(c.mem == C::k_message);
    });

    // const entity object (and so also: as_constant())
    mark(suite).test([] {
        auto e = EntityType::make_sceneless_entity();
        e.template add<A, B, C>();

        auto cobj = e.as_constant();
        const C & c = cobj.template get<C>();
        const auto & [a, b] = cobj.template get<A, B>(); {}
        static_assert(std::is_same_v<decltype(a), const A &>, "Get returns incorrect type");
        static_assert(std::is_same_v<decltype(b), const B &>, "Get returns incorrect type");
        return test(c.mem == C::k_message);
    });

    mark(suite).test([] {
        auto e = EntityType::make_sceneless_entity();
        auto cobj = e.as_constant();
        return test(should_throw<RtError>([&cobj] {
            cobj.template get<F>();
        }));
    });

    // --------------------------------- has ----------------------------------

    mark(suite).test([] {
        auto e = EntityType::make_sceneless_entity();
        e.template add<A>();
        return test(e.template has<A>());
    });

    mark(suite).test([] {
        auto e = EntityType::make_sceneless_entity();
        e.template add<A, B, C>();
        return test(e.template has_all<A, B, C>());
    });

    mark(suite).test([] {
        auto e = EntityType::make_sceneless_entity();
        e.template add<A, B, C>();
        return test(!e.template has_all<A, F, B>());
    });

    mark(suite).test([] {
        auto e = EntityType::make_sceneless_entity();
        e.template add<A, B, C>();
        return test(e.template has_any<A, D>());
    });

    mark(suite).test([] {
        auto e = EntityType::make_sceneless_entity();
        e.template add<A, B, C>();
        const auto & ce = e;
        return test( ce.template has<A>() );
    });

    mark(suite).test([] {
        auto e = EntityType::make_sceneless_entity();
        e.template add<A, B, C>();
        const auto & ce = e;
        return test( ce.template has_all<A, B, C>());
    });

    mark(suite).test([] {
        auto e = EntityType::make_sceneless_entity();
        e.template add<A, B, C>();
        const auto & ce = e;
        return test( ce.template has_any<A, B>());
    });

    mark(suite).test([] {
        auto e = EntityType::make_sceneless_entity();
        e.template add<A, B, C>();
        auto cobj = e.as_constant();
        return test( cobj.template has<B>() );
    });

    mark(suite).test([] {
        auto e = EntityType::make_sceneless_entity();
        e.template add<A, B, C>();
        auto cobj = e.as_constant();
        return test( cobj.template has_any<A, B>() );
    });

    mark(suite).test([] {
        auto e = EntityType::make_sceneless_entity();
        e.template add<A, B, C>();
        auto cobj = e.as_constant();
        return test( cobj.template has_all<A, C>() );
    });

    // --- ptr ---

    mark(suite).test([] {
        auto e = EntityType::make_sceneless_entity();
        e.template add<A, B, C>();
        C * c = e.template ptr<C>();
        auto [a, b] = e.template ptr<A, B>(); {}
        static_assert(std::is_same_v<decltype(a), A *>, "Get returns incorrect type");
        static_assert(std::is_same_v<decltype(b), B *>, "Get returns incorrect type");
        return test(c->mem == C::k_message);
    });

    mark(suite).test([] {
        auto e = EntityType::make_sceneless_entity();
        return test(!e.template ptr<A>());
    });

    mark(suite).test([] {
        auto e = EntityType::make_sceneless_entity();
        e.template add<B>();
        return test(!e.template ptr<A>());
    });

    // const entity reference
    mark(suite).test([] {
        auto e = EntityType::make_sceneless_entity();
        const auto & cref = e;
        e.template add<A, B, C>();
        const C * c = cref.template ptr<C>();
        auto [a, b, f] = cref.template ptr<A, B, F>(); {}
        static_assert(std::is_same_v<decltype(a), const A *>, "Get returns incorrect type");
        static_assert(std::is_same_v<decltype(b), const B *>, "Get returns incorrect type");
        return test(c->mem == C::k_message && !f);
    });

    // const entity object (and so also: as_constant())
    mark(suite).test([] {
        auto e = EntityType::make_sceneless_entity();
        e.template add<A, B, C>();

        auto cobj = e.as_constant();
        const C * c = cobj.template ptr<C>();
        auto [a, b, f] = cobj.template ptr<A, B, F>(); {}
        static_assert(std::is_same_v<decltype(a), const A *>, "Get returns incorrect type");
        static_assert(std::is_same_v<decltype(b), const B *>, "Get returns incorrect type");
        return test(c->mem == C::k_message && !f);
    });

    // --- remove ---

    mark(suite).test([] {
        auto e = EntityType::make_sceneless_entity();
        e.template add<A, B, C>();
        bool had_b = e.template has<B>();
        int old_b_count = Counted<B>::count();
        e.template remove<B>();
        return test(   had_b && old_b_count == 1
                    && !e.template has<B>() && Counted<B>::count() == 0);
    });

    mark(suite).test([] {
        // correct lifetime control?
        int b_count, c_count, old_a_count;
        {
        auto e = EntityType::make_sceneless_entity();
        e.template add<A, B, C>();
        e.template remove<B, C>();
        b_count = Counted<B>::count();
        c_count = Counted<C>::count();
        old_a_count = Counted<A>::count();
        }
        return test(   AllInst::count() == 0 && c_count == 0 && b_count == 0
                    && old_a_count == 1);
    });

    // --- utilities ---
    mark(suite).test([] {
        // inequality
        auto ea = EntityType::make_sceneless_entity();
        EntityType eb;
        return test(ea != eb);
    });

    // (bool) operator
    mark(suite).test([] {
        auto ea = EntityType::make_sceneless_entity();
        return test(!!ea);
    });

    mark(suite).test([] {
        EntityType eb;
        return test(!eb);
    });

    mark(suite).test([] {
        // copy constructor, and equality
        auto ea = EntityType::make_sceneless_entity();
        EntityType eb{ea};
        return test(ea == eb);
    });

    mark(suite).test([] {
        // move constructor
        auto ea = EntityType::make_sceneless_entity();
        EntityType eb{std::move(ea)};
        return test(ea != eb);
    });
    mark(suite).test([] {
        // assignments
        auto ea = EntityType::make_sceneless_entity();
        EntityType eb, ec;
        eb = std::move(ea);
        ec = eb;
        return test(eb == ec && ea != eb);
    });

    mark(suite).test([] {
        auto ea = EntityType::make_sceneless_entity();
        EntityType eb;
        std::cout << ea.hash() << std::endl;
        std::cout << eb.hash() << std::endl;

        // hash code for "null" entity must be 0
        return test(eb.hash() == 0 && eb.hash() != ea.hash());
    });

    mark(suite).test([] {
        auto ea = EntityType::make_sceneless_entity();
        auto eb = ea;
        EntityType ec;
        ec.swap(ea);
        return test(ec == eb && !ea);
    });

    // --- is_null ---

    mark(suite).test([] {
        auto ea = EntityType::make_sceneless_entity();
        EntityType eb;
        const auto & ceb = eb;
        auto acobj = ea.as_constant();
        return test(   !ea.is_null() && eb.is_null() && ceb.is_null()
                    && !acobj.is_null());
    });
    return suite.has_successes_only();
}

template <typename EntityType>
bool test_scene() {
    using Scene = ecs::SceneOf<EntityType>;
    TestSuite suite;
    const auto suite_name = std::string{"Scenes with "} + k_name_for_entity_tests<EntityType>;
    suite.start_series(suite_name.c_str());
    mark(suite).test([] {
        Scene scene;
        auto e = scene.make_entity();
        e.make_entity();
        scene.update_entities();
        return test(scene.count() == 2);
    });
    mark(suite).test([] {
        Scene scene;
        auto e = scene.make_entity();
        e.make_entity();
        scene.update_entities();

        e.request_deletion();
        scene.update_entities();
        return test(scene.count() == 1);
    });
    return suite.has_successes_only();
}

template <typename EntityType>
bool test_reftypes() {
    TestSuite suite;
    using ecs::EntityRef, ecs::ConstEntityRef;
    using ConstEntity = typename EntityType::ConstEntity;

    const auto suite_name = std::string{"Reference types with "} + k_name_for_entity_tests<EntityType>;
    suite.start_series(suite_name.c_str());
    // need: move and copy
    // to reference types
    mark(suite).test([] {
        auto ea = EntityType::make_sceneless_entity();
        auto eb = ea;
        auto ec = ea;
        EntityRef eref_by_copy{ea};
        EntityRef eref_by_move{std::move(eb)};

        ConstEntityRef ecref_by_eref_move{EntityRef{ea}};
        ConstEntityRef ecref_by_eref_copy{eref_by_copy};

        ConstEntityRef ecref_by_ent_copy{ec};
        ConstEntityRef ecref_by_ent_move{std::move(ec)};

        ConstEntityRef ecref_by_cent_move{ea.as_constant()};
        auto cobj = ea.as_constant();
        ConstEntityRef ecref_by_cent_copy{cobj};

        return test(   ConstEntityRef{ea} == ConstEntityRef{ea.as_constant()}
                    && ConstEntityRef{ea} != ConstEntityRef{}
                    && eref_by_move == eref_by_copy
                    && eref_by_move != EntityRef{});
    });
    // from reference types
    mark(suite).test([] {
        auto ea = EntityType::make_sceneless_entity();
        EntityRef erefa{ea};
        ConstEntityRef ecrefa{EntityRef{ea}};

        EntityRef erefa_ent_mv{ea};

        EntityType ent_by_copy{erefa};
        EntityType ent_by_move{std::move(erefa_ent_mv)};

        erefa_ent_mv = EntityRef{ea};

        ConstEntity cent_by_eref_copy{erefa};
        ConstEntity cent_by_eref_move{std::move(erefa_ent_mv)};

        ConstEntity cent_by_ecref_copy{ecrefa};
        ConstEntity cent_by_ecref_move{std::move(ecrefa)};
        return test(   ent_by_copy == ent_by_move
                    && cent_by_eref_copy == cent_by_ecref_move
                    && cent_by_ecref_move != ConstEntity{});
    });
    return suite.has_successes_only();
}

#undef mark
