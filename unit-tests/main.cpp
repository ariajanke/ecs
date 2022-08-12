/***************************************************************************

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

#include "shared.hpp"
#include <memory>

template <>
const char * k_name_for_entity_tests<ecs::HashTableEntity> = "HashTableEntity";

template <>
const char * k_name_for_entity_tests<ecs::AvlTreeEntity> = "AvlTreeEntity";

static constexpr const int k_dog_noises = 1;
static constexpr const int k_cat_noises = 2;

class Animal : AllInst {
public:
    virtual ~Animal() {}
    virtual int speak() const = 0;
};

class Dog final : public Animal, Counted<Dog> {
    int speak() const final { return k_dog_noises; }
};

class Cat final : public Animal, Counted<Cat> {
    int speak() const final { return k_cat_noises; }
};

bool test_sharedptr();

class TestTypeSetAndOptional {
    template <typename T>
    using Optional = ecs::Optional<T>;

    template <typename ... Types>
    using TypeSet = cul::TypeSet<Types...>;

    template <typename T>
    using StripOptional = ecs::StripOptional<T>;

    template <typename T>
    using IsAnOptionalType = ecs::IsAnOptionalType<T>;

    // play types
    using StrippedSet = TypeSet<A, B, Optional<C>>::Transform<StripOptional>::Transform<std::add_pointer_t>;
    static_assert(StrippedSet::kt_contains<A *>);
    static_assert(StrippedSet::kt_contains<C *>);
    static_assert(!StrippedSet::kt_contains<Optional<C>>);
    using St = TypeSet<A &, Optional<B>, C &, B &>;
    static_assert(St::RemoveIf<IsAnOptionalType>::kt_contains<A &>);
    static_assert(St::RemoveIf<IsAnOptionalType>::kt_contains<C &>);
    static_assert(!St::RemoveIf<IsAnOptionalType>::kt_contains<Optional<B>>);
    static_assert(!St::RemoveIf<IsAnOptionalType>::kt_contains<TypeSet<>>);
    using Rq = St::RemoveIf<IsAnOptionalType>;
    static_assert(St::Difference<Rq>::kt_equal_to_set<Optional<B>>);

    static_assert(St::Difference<Rq>::kt_contains<Optional<B>>);
    static_assert(!St::Difference<Rq>::kt_contains<TypeSet<>>);
    static_assert(St::Difference<Rq>::Transform<StripOptional>::kt_equal_to_set<B>);
    static_assert(!St::Difference<Rq>::Transform<StripOptional>::kt_contains<TypeSet<>>);
    static_assert(Rq::Transform<std::remove_reference_t>::kt_equal_to_set<A, B, C>);
    using Un = St::Difference<Rq>::Transform<StripOptional>::Union<Rq::Transform<std::remove_reference_t>>;//::Transform<std::remove_reference_t>;
    static_assert(Un::kt_equal_to_set<A, B, C>);
    static_assert(Un::kt_occurance_count<B> == 1);
    static_assert(Un::kt_occurance_count<C> == 1);
    static_assert(Un::kt_occurance_count<A> == 1);
};

#define MACRO_SHOULD_FAIL_COMPILIATION 0
class N { N() {
    using namespace ecs;
#   if MACRO_SHOULD_FAIL_COMPILIATION == 1
    TypeSet<C, A, B, C>{};
#   elif MACRO_SHOULD_FAIL_COMPILIATION == 2
    TypeSet<A, B, C>::SetWithHead<A>{};
#   endif
    }
};




template <typename Func>
void get_info_on_func(Func && f) {
    // now we have a typelist associated with the functor
    auto count = ecs::FunctionTraitsOf<Func>::k_argument_count;
    std::cout << "Func has " << count << " parameter(s)." << std::endl;
}

void system_from_function(B &) {
    std::cout << "This system is built from a function." << std::endl;
}

int main() {
    using namespace ecs;

    get_info_on_func([]( int, char ){});

    using cul::TypeSet;
    static_assert(TypeSet<A, B>::Union<B>::kt_occurance_count<B> == 1);
    MetaFunctions::set_component_addition_tracker([](const char * name, void *) {
        std::cout << "New component: \"" << name << "\" was just created." << std::endl;
    }, nullptr);
#   if 0
    EntityRef a;
    EntityRef b{a};
    EntityRef c{std::move(b)};
    EntityRef d, e;
    d = a;
    e = std::move(c);
    assert(e.hash() == 0);
#   endif

    {
    using Entity = AvlTreeEntity;
    using Scene  = ecs::SceneOf<Entity>;
    Scene scene;
    auto e = scene.make_entity();
    e.add<A, B, C>();
    scene.make_entity().add<C, std::string>();
    make_singles_system<AvlTreeEntity>()(scene);
    {
    using Ts = TypeSet<Optional<B>, C &>::Union<B &>;
    using Rq = Ts::RemoveIf<IsAnOptionalType>;
    static_assert(Rq::kt_equal_to_set<C &, B &>);
    using Opt = Ts::Difference<Rq>;
    static_assert(Opt::kt_equal_to_set<Optional<B>>);
    using FullUnion = Opt::Transform<StripOptional>
        ::Union<Rq::Transform<std::remove_reference_t>>;
    [[maybe_unused]] FullUnion m{};
    }
    (*make_singles_system_uptr<AvlTreeEntity>([](A &) {
        std::cout << "Sys requiring an A says hello." << std::endl;
    }, [](Optional<B> optb, C &) {
        std::cout << "Sys requiring an C, optionally a B says hello." << std::endl;
        if (optb) {
            std::cout << "This entity also has a b." << std::endl;
        }
    }, [](std::string & str, C & c) {
        str = c.mem;
        std::cout << str << std::endl;
    }, system_from_function))(scene);
    // these need to be seperated into their own unit tests
    }
    {
    using std::cout, std::endl;
    // default and named
    SharedPtr<int> sint1;
    SharedPtr<int> sint2{SharedPtr<int>::make(10)};
    // copy constructors
    SharedPtr<int> sint3{sint2};
    SharedPtr<const int> sint4{sint2};
    // SharedPtr<int>{sint4}; // error
    // move constructors
#   ifdef MACRO_COMPILER_GCC
#   pragma GCC diagnostic ignored "-Wpessimizing-move"
#   endif
    SharedPtr<int>{ std::move( SharedPtr<int>::make(10) ) };
    SharedPtr<const int>{ std::move( SharedPtr<int>::make(10) ) };
#   ifdef MACRO_COMPILER_GCC
#   pragma GCC diagnostic pop
#   endif
    // assignments
    sint4 = sint3;
    sint3 = sint2;
    // sint3 = sint4; // error

    // move assignments
    sint4 = std::move(sint3);
    sint3 = std::move(sint2);
    // sint3 = std::move(sint4); // error

    }
    // systems next...
    // for now two target types of systems:
    // - single full entity
    // - single tuples, composable into onto each other
    // in the future, we absolutely want schedulable systems, to show off
    // multithreading capabilities
    //
    // also code clean up! and documentation
    // current code needs to be refactored, HashTable entity needs to be
    // expanded, tests need to be arranged to cover entire intended interface
    //
    // I want to get to that point where this code is fairly "self-contained"
    // even if I'm not completely satisified with the feature set.

    return andf(
        run_tests_for_entity_type<HashTableEntity>(),
        run_tests_for_entity_type<AvlTreeEntity>(),
        test_sharedptr(),
        test_hashtableentity(),
        test_avltreeentity()

                ) ? 0 : ~0;
}

#define mark MACRO_MARK_POSITION_OF_CUL_TEST_SUITE

bool test_sharedptr() {
    using namespace cul::ts;
    using ecs::SharedPtr, ecs::WeakPtr;
    class Reseter {
    public:
        Reseter() {}
        ~Reseter() {
            AllInst::hard_reset();
            Counted<Dog>::hard_reset();
            Counted<Cat>::hard_reset();
        }
    };

    TestSuite suite;
    suite.start_series("shared pointer utilities");
    // gonna go... one by one
    mark(suite).test([] {
        return test(!SharedPtr<int>{});
    });
    mark(suite).test([] {
        auto ptr = SharedPtr<int>::make(10);
        return test(*ptr == 10);
    });
    mark(suite).test([] {
        Reseter r;
        int old_count = [] {
            auto ptr = SharedPtr<Dog>::make();
            return Counted<Dog>::count();
        } ();
        return test(old_count == 1 && Counted<Dog>::count() == 0);
    });
    // upcast
    mark(suite).test([] {
        Reseter r;
        SharedPtr<const Animal> animal{SharedPtr<Cat>::make()};
        return test(animal->speak() == k_cat_noises);
    });
    // downcast
    mark(suite).test([] {
        SharedPtr<const Animal> animal{SharedPtr<Cat>::make()};
        return test(   !animal.dynamically_cast_to<const Dog>()
                    &&  animal.dynamically_cast_to<const Cat>());
    });
    // w to w copy construct
    mark(suite).test([] {
        auto ptr = SharedPtr<int>::make(10);
        return test(*SharedPtr<int>{ptr} == 10);
    });
    // w to c copy construct
    mark(suite).test([] {
        auto ptr = SharedPtr<int>::make(10);
        return test(*SharedPtr<const int>{ptr} == 10);
    });
    // c to c copy construct
    mark(suite).test([] {
        SharedPtr<const int> ptr = SharedPtr<const int>::make(10);
        return test(*SharedPtr<const int>{ptr} == 10);
    });
    // should "compile" test c to w
    // SharedPtr<int>{SharedPtr<const int>::make(10)};
    // w to w move construct
    mark(suite).test([] {
        auto ptr = SharedPtr<int>::make(10);
        return test(*SharedPtr<int>{std::move(ptr)} == 10);
    });
    // w to c move construct
    mark(suite).test([] {
        auto ptr = SharedPtr<int>::make(10);
        return test(*SharedPtr<const int>{std::move(ptr)} == 10);
    });
    // c to c move construct
    mark(suite).test([] {
        SharedPtr<const int> ptr = SharedPtr<const int>::make(10);
        return test(*SharedPtr<const int>{std::move(ptr)} == 10);
    });
    // reserve for c to w move
    // w to w on move assignment
    mark(suite).test([] {
        SharedPtr<int> b;
        {
        SharedPtr<int> a = SharedPtr<int>::make(100);
        // no increment should occur
        b = std::move(a);
        }
        return test(*b == 100);
    });
    // w to c on move assignment
    mark(suite).test([] {
        SharedPtr<const int> b;
        {
        SharedPtr<int> a = SharedPtr<int>::make(100);
        b = std::move(a);
        }
        return test(*b == 100);
    });
    // c to c on move assignment
    mark(suite).test([] {
        SharedPtr<const int> b;
        {
        SharedPtr<const int> a = SharedPtr<int>::make(100);
        b = std::move(a);
        }
        return test(*b == 100);
    });
    // <INV> c to w on move assignment
    // w to w on copy assignment
    mark(suite).test([] {
        SharedPtr<int> b;
        {
        SharedPtr<int> a = SharedPtr<int>::make(100);
        // no increment should occur
        b = a;
        }
        return test(*b == 100);
    });
    // w to c on copy assignment
    mark(suite).test([] {
        SharedPtr<const int> b;
        {
        SharedPtr<int> a = SharedPtr<int>::make(100);
        b = a;
        }
        return test(*b == 100);
    });
    // c to c on copy assignment
    mark(suite).test([] {
        SharedPtr<const int> b;
        {
        SharedPtr<const int> a = SharedPtr<int>::make(100);
        b = a;
        }
        return test(*b == 100);
    });
    // <INV> c to w on copy assignment

    // sw to ww copy constructor
    mark(suite).test([] {
        SharedPtr<int> a = SharedPtr<int>::make(100);
        WeakPtr<int> b{a};
        return test(*b.lock() == 100);
    });
    // sw to wc copy constructor
    mark(suite).test([] {
        SharedPtr<int> a = SharedPtr<int>::make(100);
        WeakPtr<const int> b{a};
        static_assert(std::is_same_v<decltype(b.lock()), SharedPtr<const int>>);
        return test(*b.lock() == 100);
    });
    // sc to wc copy constructor
    mark(suite).test([] {
        SharedPtr<const int> a = SharedPtr<int>::make(100);
        WeakPtr<const int> b{a};
        static_assert(std::is_same_v<decltype(b.lock()), SharedPtr<const int>>);
        return test(*b.lock() == 100);
    });
    // <INV> sc to ww copy constructor

    // ww to ww copy constructor
    mark(suite).test([] {
        SharedPtr<int> a = SharedPtr<int>::make(100);
        WeakPtr<int> b{a};
        WeakPtr<int> c{b};
        return test(*c.lock() == 100);
    });
    // ww to wc copy constructor
    mark(suite).test([] {
        SharedPtr<int> a = SharedPtr<int>::make(100);
        WeakPtr<int> b{a};
        WeakPtr<const int> c{b};
        return test(*c.lock() == 100);
    });
    // wc to wc copy constructor
    mark(suite).test([] {
        SharedPtr<int> a = SharedPtr<int>::make(100);
        WeakPtr<const int> b{a};
        WeakPtr<const int> c{b};
        return test(*c.lock() == 100);
    });
    // <INV> wc to ww copy constructor

    // sw to ww move constructor
    mark(suite).test([] {
        WeakPtr<A> b{[] {
            SharedPtr<A> a = SharedPtr<A>::make();
#           pragma GCC diagnostic ignored "-Wpessimizing-move"
            return std::move(a);
#           pragma GCC diagnostic pop
        } ()};
        return test(b.has_expired() && Counted<A>::count() == 0);
    });
    Counted<A>::hard_reset();
    // sw to wc move constructor
    mark(suite).test([] {
        SharedPtr<int> a = SharedPtr<int>::make(100);
        SharedPtr<int> c{a};
        WeakPtr<const int> b{std::move(a)};
        return test(*b.lock() == 100);
    });
    // sc to wc move constructor
    mark(suite).test([] {
        SharedPtr<int> a = SharedPtr<int>::make(100);
        SharedPtr<const int> c{a};
        WeakPtr<const int> b{std::move(c)};
        return test(*b.lock() == 100);
    });
    // <INV> sc to ww move constructor

    // ww to ww move constructor
    mark(suite).test([] {
        SharedPtr<int> a = SharedPtr<int>::make(100);
        WeakPtr<int> b{a};
        WeakPtr<int> c{std::move(b)};
        return test(*c.lock() == 100 && c.observers() == 1);
    });
    // ww to wc move constructor
    mark(suite).test([] {
        SharedPtr<int> a = SharedPtr<int>::make(100);
        WeakPtr<int> b{a};
        WeakPtr<const int> c{std::move(b)};
        return test(*c.lock() == 100);
    });
    // wc to wc move constructor
    mark(suite).test([] {
        SharedPtr<int> a = SharedPtr<int>::make(100);
        WeakPtr<const int> b{a};
        WeakPtr<const int> c{std::move(b)};
        return test(*c.lock() == 100);
    });
    // <INV> wc to ww move constructor

    // ww to ww copy assignment
    mark(suite).test([] {
        SharedPtr<int> a = SharedPtr<int>::make(100);
        WeakPtr<int> b{a};
        WeakPtr<int> c;
        c = b;
        return test(*c.lock() == 100);
    });
    // ww to wc copy assignment
    mark(suite).test([] {
        SharedPtr<int> a = SharedPtr<int>::make(100);
        WeakPtr<int> b{a};
        WeakPtr<const int> c;
        c = b;
        return test(*c.lock() == 100);
    });
    // wc to wc copy assignment
    mark(suite).test([] {
        SharedPtr<int> a = SharedPtr<int>::make(100);
        WeakPtr<const int> b{a};
        WeakPtr<const int> c;
        c = b;
        return test(*c.lock() == 100 && c.observers() == 2);
    });
    // <INV> wc to ww copy assignment

    // ww to ww move assignment
    mark(suite).test([] {
        SharedPtr<int> a = SharedPtr<int>::make(100);
        WeakPtr<int> b{a};
        WeakPtr<int> c;
        c = std::move(b);
        return test(*c.lock() == 100);
    });
    // ww to wc move assignment
    mark(suite).test([] {
        SharedPtr<int> a = SharedPtr<int>::make(100);
        WeakPtr<int> b{a};
        WeakPtr<const int> c;
        c = std::move(b);
        return test(*c.lock() == 100 && c.observers() == 1);
    });
    // wc to wc move assignment
    mark(suite).test([] {
        SharedPtr<int> a = SharedPtr<int>::make(100);
        WeakPtr<const int> b{a};
        WeakPtr<const int> c;
        c = std::move(b);
        return test(*c.lock() == 100 && c.observers() == 1);
    });
    // <INV> wc to ww move assignment

    // should have some instance counting tests...
    // then... weak pointers

    mark(suite).test([] {
        WeakPtr<int> a;
        {
        auto b = SharedPtr<int>::make(10);
        a = WeakPtr<int>{b};
        }
        return test(a.has_expired());
    });

    return suite.has_successes_only();
}
