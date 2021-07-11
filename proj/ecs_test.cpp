#include <ecs/ecs.hpp>

#include <common/Util.hpp>
#include <common/Vector2Util.hpp>
#include <common/SfmlVectorTraits.hpp>

#include <iostream>
#include <memory>
#include <random>
#include <algorithm>
#include <thread>
#include <chrono>
#include <unordered_map>

//#define MACRO_BUILD_WITH_SFML
#ifdef MACRO_BUILD_WITH_SFML
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Color.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#endif

#include <cassert>

namespace def_tests {

struct A {};
struct B {};
struct C {};

// here are some tests
static_assert(ecs::detail::ForTypes<A, B, C>::Has<C>::value, "");
static_assert(ecs::detail::ForTypes<A, B, C>::IsSuperSetOf<B, C>::value, "");
static_assert(ecs::detail::ForTypes<C, A, B>::IsSuperSetOf<C, A>::value, "");
static_assert(ecs::detail::ForTypes<C, A, B>::IsSuperSetOf<const C, A>::with_const, "");

}

using ecs::EntityRef;

namespace t1 {

struct CompWithRef {
    EntityRef entref;
};

struct NotifyDel {
    ~NotifyDel() {
        std::cout << "nd destructed" << std::endl;
    }
};

struct SomethingElse {
    std::size_t num = 0;
    ~SomethingElse() { num = 0xDEADBEEF; }
};

static_assert(
    ecs::detail::WouldInlineComponent<SomethingElse>::k_value ==
    (sizeof(SomethingElse) <= ecs::detail::k_auto_inline_size), "");

static_assert(ecs::detail::k_auto_inline_size == 3*sizeof(void*), "following tests assume inline size of 3 pointers");

static_assert(ecs::detail::WouldInlineComponent<SomethingElse>::k_value, "");
static_assert(ecs::detail::WouldInlineComponent<NotifyDel>::k_value, "");
static_assert(ecs::detail::WouldInlineComponent<CompWithRef>::k_value, "");

using TestCount = ecs::detail::CountInlinedComponents<SomethingElse, NotifyDel, CompWithRef>;
#if 1
static_assert(TestCount::GetInlineIndex<SomethingElse>::k_index == 2, "");
static_assert(TestCount::GetInlineIndex<NotifyDel>::k_index == 1, "");
static_assert(TestCount::GetInlineIndex<CompWithRef>::k_index == 0, "");
#endif

} // end of t1 namespace

void test_f1();

void test_f2();

void test_f3();

namespace {
#ifdef MACRO_BUILD_WITH_SFML
using Vector = sf::Vector2<double>;

struct Velocity     : public Vector {
    Velocity() = default;
    Velocity(Vector r): Vector(r) {}
};
struct Position     : public Vector {
    Position() = default;
    Position(Vector r): Vector(r) {}
};
struct Displacement : public Vector {
    Displacement() = default;
    Displacement(Vector r): Vector(r) {}
};

struct NumberLike {
    double val;
    operator double () { return val; }
    double & operator = (double x) { val = x; return val; }
};

struct Mass   : public NumberLike {};
struct Radius { double val; operator double () const { return val; } };

struct Name {
    std::string s;
};

struct TestComp {
    std::array<void *, 4> m;
};

}
#if 0
namespace ecs {

template <>
struct DefineWouldInline<Name> : public TrueType {};

}
#endif
namespace {

struct FrameCounter { int fc = 0; };

using Entity = ecs::Entity<Velocity, Position, Displacement, Mass, Radius, Name, FrameCounter, TestComp>;
static_assert (ecs::detail::ForTypes<Velocity, Position, Displacement, Mass, Radius, Name>::Has<Velocity>::value, "" );
using EntityManager = Entity::ManagerType;
using System = EntityManager::SystemType;
#if 0
template <typename ... Types>
using FilteredSystem = EntityManager::FilteredSystem<Types...>;
#endif
class TimeAwareSystem {
public:
    void set_elapsed_time(double et) { m_et = et; }
protected:
    double elapsed_time() const { return m_et; }

    double m_et = 0;
};

// first  low  bounds where f(first ) = false
// second high bounds where f(second) = true
template <typename Func>
std::pair<double, double> find_smallest_diff(Func && f, double hint = 0);

template <typename Func>
double find_highest_false(Func && f, double hint = 0.5) {
    return find_smallest_diff(std::move(f), hint).first;
}

template <typename Func>
double find_lowest_true(Func && f, double hint = 0.5) {
    return find_smallest_diff(std::move(f), hint).second;
}

template <typename Func>
std::pair<double, double> find_smallest_diff(Func && f, double hint) {
    if (f(0)) {
        throw std::runtime_error("find_smallest_diff: f(0) is true");
    }
    if (!f(1)) {
        throw std::runtime_error("find_smallest_diff: f(1) is false");
    }

    static constexpr const double k_good_enough = 0.00005;
    bool   fg   = f(hint);
    double low  = fg ? 0    : hint;
    double high = fg ? hint : 1   ;

    while ((high - low) > k_good_enough) {
        double t = low + (high - low)*0.5;
        if (f(t)) {
            high = t;
        } else {
            low = t;
        }
    }
    return std::make_pair(low, high);
}

struct CollisionRecord {
    std::vector<EntityRef> others;
};

class ColSys final :
#   if 0
    public FilteredSystem<Displacement, Position, Radius>,
#   endif
    public System
#   if 0
    ,public ecs::UniqueToSystemComponent<CollisionRecord>
#   endif
{
    // displacement, position, radius
    // non-chunkable
    void update(const ContainerView & cont) override {
        // clipping and applying displacements has to be done syncronously
        for (auto e : cont) {
            if (!has_ucomponent(e)) add_ucomponent(e);
            ucomponent_for(e).others.clear();
        }
        cul::quad_range/*<Entity>*/(cont, [this](Entity a, Entity b) {
            ucomponent_for(a).others.push_back(b);
            ucomponent_for(b).others.push_back(a);
        });

        for (auto e : cont) {
            auto & dis = e.get<Displacement>();
            for (auto & other : ucomponent_for(e).others) {
                auto ndis = trim_displacement(dis, e, Entity(other));
                if (ndis != dis) {
                    e.get<Velocity>() *= -1.;
                }
                dis = ndis;
            }
            e.get<Position>() += dis;
            dis = Vector();
        }
    }

    Vector trim_displacement
        (Vector a_dis, const Entity & a, const Entity & b)
    {
        Vector a_pos = a.get<Position>();
        Vector b_pos = b.get<Position>();
        double pass_thershold = a.get<Radius>() + b.get<Radius>();
        auto overlaps_at = [=](double t) {
            return cul::magnitude((a_pos + a_dis*t) - b_pos) < pass_thershold;
        };
        if (overlaps_at(0)) return a_dis;
        if (overlaps_at(1)) {
            auto t = find_highest_false(overlaps_at);
            a_dis *= find_highest_false(overlaps_at);
            assert(!overlaps_at(t));
        }
        return a_dis;
    }

    bool has_ucomponent(const Entity & e) const
        { return m_map.find(e) != m_map.end(); }

    void add_ucomponent(const Entity & e) {
        assert(!has_ucomponent(e));
        m_map[e];
    }

    CollisionRecord & ucomponent_for(const Entity & e) {
        assert(has_ucomponent(e));
        return m_map[e];
    }

    std::unordered_map<Entity, CollisionRecord, ecs::EntityHasher> m_map;
};

class VelSys final :
#   if 0
    public FilteredSystem<Velocity, Displacement>,
#   endif
    public System,
    public TimeAwareSystem
{
    // displacement, const velocity
    // chunkable
    void update(const ContainerView & cont) override {
        for (auto e : cont) {
            e.get<Displacement>() = e.get<Velocity>()*elapsed_time();
        }
    }
};

class AccSys final : public System, public TimeAwareSystem {
    // const mass, const position, velocity
    // non-chunkable
    void update(const ContainerView & cont) override {
        cul::quad_range/*<Entity>*/(cont, [this](Entity a, Entity b) {
            do_pair(a, b);
            do_pair(b, a);
        });
    }
    void do_pair(Entity & a, Entity & b) {
        using namespace cul;
        static constexpr const double k_grav = 0.0025*4.;
        auto diff = b.get<Position>() - a.get<Position>();
        auto dist = magnitude(diff);
        auto prod_mass = a.get<Mass>() * b.get<Mass>();
        diff = normalize(diff);
        auto vel_del = k_grav*diff*(prod_mass / (dist*dist))*elapsed_time();
        Vector & a_vel = a.get<Velocity>();
        a_vel += vel_del;
        if (magnitude(a_vel) > 200) a_vel = normalize(a_vel)*200.;
    }
};

class FrameCountSys final : public System {
    void update(const ContainerView & cont) override {
        for (auto e : cont) {
            if (!e.has<FrameCounter>()) continue;
            auto & fc = e.get<FrameCounter>();
            fc.fc++;
        }
    }
};

struct Disc {
    std::vector<sf::Vertex> verticies;
    Vector translation;
};

Disc make_disc(std::default_random_engine &, double radius);

class DrawSys final :
    public System, public sf::Drawable
#   if 0
    public ecs::UniqueToSystemComponent<Disc>
#   endif
{
public:
    DrawSys() {
        m_rng = std::default_random_engine { std::random_device()() };
    }
private:
    // const color, const position, const radius
    void update(const ContainerView & cont) override {
        for (Entity & e : cont) {
            if (!has_ucomponent(e)) {
                add_ucomponent(e) = make_disc(m_rng, e.get<Radius>());
            }
            ucomponent_for(e).translation = e.get<Position>();
        }
    }

    void draw(sf::RenderTarget & target, sf::RenderStates states) const override {
        for (const auto & pair : m_map) {
            const auto & disc = pair.second;
            auto tstates = states;
            tstates.transform.translate(sf::Vector2f(disc.translation));
            target.draw(disc.verticies.data(), disc.verticies.size(), sf::PrimitiveType::Triangles, tstates);
        }
    }

    bool has_ucomponent(const Entity & e) const
        { return m_map.find(e) != m_map.end(); }

    Disc & add_ucomponent(const Entity & e) {
        assert(!has_ucomponent(e));
        return m_map[e];
    }

    Disc & ucomponent_for(const Entity & e) {
        assert(has_ucomponent(e));
        return m_map[e];
    }

    std::unordered_map<Entity, Disc, ecs::EntityHasher> m_map;
    std::default_random_engine m_rng;
};

sf::Color random_color(std::default_random_engine & rng) {
    using Distri = std::uniform_int_distribution<uint8_t>;
    sf::Color rv;
    std::array<uint8_t *, 3> comps { &rv.r, &rv.g, &rv.b };
    auto randf = [&rng](std::size_t i) { return std::uniform_int_distribution<std::size_t>(0, i - 1)(rng); };
    std::random_shuffle(comps.begin(), comps.end(), randf);
    *comps[0] = Distri(200, 255)(rng);
    *comps[1] = Distri(100, 200)(rng);
    *comps[2] = Distri(100, 200)(rng);
    return rv;
}

Disc make_disc
    (std::default_random_engine & rng, double radius)
{
    static constexpr const double k_pi = cul::k_pi_for_type<double>;
    std::vector<sf::Vertex> rv;
    double steps = std::max(std::min(std::round(radius / 5.), 20.), 8.);
    rv.reserve(std::size_t(steps)*3);
    double step_size = k_pi*2. / steps;
    for (double t = 0; t < k_pi*2.; t += step_size) {
        double next_t = std::min(t + step_size, k_pi*2);
        auto mk_vrt = [](double x, double y) { return sf::Vertex(sf::Vector2f(float(x), float(y))); };
        double x  = std::cos(t     )*radius;
        double y  = std::sin(t     )*radius;
        double nx = std::cos(next_t)*radius;
        double ny = std::sin(next_t)*radius;
        rv.push_back(mk_vrt(x , y ));
        rv.push_back(mk_vrt(nx, ny));
        rv.push_back(mk_vrt(0 , 0 ));
    }
    auto color = random_color(rng);
    for (auto & vtx : rv) {
        vtx.color = color;
    }
    Disc rrv;
    rrv.verticies.swap(rv);
    return rrv;
}

template <typename ... Types>
class SystemsLayer {
public:
    void set_elapsed_time(double) {}
#   if 0
    void register_system(EntityManager &) {}
#   endif
    void render_to(sf::RenderTarget &) const {}
    void run_systems(EntityManager &) {}
};

template <typename Head, typename ... Types>
class SystemsLayer<Head, Types...> : public SystemsLayer<Types...> {
public:
    void set_elapsed_time(double et) {
        if constexpr (std::is_base_of<TimeAwareSystem, Head>::value) {
            sys.set_elapsed_time(et);
        }
        SystemsLayer<Types...>::set_elapsed_time(et);
    }
    void run_systems(EntityManager & em) {
        em.run_system(sys);
        SystemsLayer<Types...>::run_systems(em);
    }
#   if 0
    void register_system(EntityManager & em) {
        em.register_system(&sys);
        SystemsLayer<Types...>::register_system(em);
    }
#   endif
    void render_to(sf::RenderTarget & target) const {
        if constexpr (std::is_base_of<sf::Drawable, Head>::value) {
            target.draw(sys);
        }
        SystemsLayer<Types...>::render_to(target);
    }
private:
    Head sys;
};

void add_typicals(Entity e, double rad, double mass, double x, double y) {
    e.add<Velocity>();
    e.add<Mass>().val = mass;
    e.add<Radius>().val = rad;
    e.add<Position>() = Vector(x, y);
    e.add<Displacement>();
}

#endif
} // end of <anonymous> namespace
// end of ifdef MACRO_BUILD_WITH_SFML

#ifdef MACRO_BUILD_WITH_SFML
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#endif

void test2();
void test3();

int main() {

    test_f1();
    test_f2();
    test_f3();
    test2();
    test3();
#   ifdef MACRO_BUILD_WITH_SFML

    using TestCount = ecs::detail::CountInlinedComponents<Velocity, Position, Displacement, Mass, Radius, Name, FrameCounter>;

    std::cout << std::boolalpha <<
        (ecs::detail::WouldInlineComponent<Velocity>::k_value) << " " <<
        (ecs::detail::WouldInlineComponent<Position>::k_value)<< " " <<
        (ecs::detail::WouldInlineComponent<Displacement>::k_value)<< " " <<
        (ecs::detail::WouldInlineComponent<Mass>::k_value)<< " " <<
        (ecs::detail::WouldInlineComponent<Radius>::k_value) << " " <<
        (ecs::detail::WouldInlineComponent<Name>::k_value) << " " <<
        (ecs::detail::WouldInlineComponent<FrameCounter>::k_value) << " " <<
        (ecs::detail::WouldInlineComponent<TestComp>::k_value) << std::endl;

    std::cout <<
        TestCount::GetInlineIndex<Velocity>::k_index << " " <<
        TestCount::GetInlineIndex<Position>::k_index << " " <<
        TestCount::GetInlineIndex<Displacement>::k_index << " " <<
        TestCount::GetInlineIndex<Mass>::k_index << " " <<
        TestCount::GetInlineIndex<Radius>::k_index << " " <<
        TestCount::GetInlineIndex<Name>::k_index << " " <<
        TestCount::GetInlineIndex<FrameCounter>::k_index << " " <<
        TestCount::GetInlineIndex<TestComp>::k_index << std::endl;

    std::cout << "Entity component table size "
              << Entity::k_component_table_size << " bytes.\n"
              << "Number of inlined components "
              << Entity::k_number_of_components_inlined << " / "
              << Entity::k_component_count << "." << std::endl;

    EntityManager emana;
    SystemsLayer<DrawSys, AccSys, VelSys, ColSys, FrameCountSys> systems;
#   if 0
    systems.register_system(emana);
#   endif
    systems.set_elapsed_time(1. / 60.);

    // ecs::Entity<Velocity, Position, Displacement, Mass, Radius>;
    {
    auto a = emana.make_entity();
    add_typicals(a, 30, 1600, 320, 240);

    [[maybe_unused]] auto & vel = a.get<Velocity>();

    a.add<FrameCounter>();
    a.add<Name>().s = "big";

    auto b = emana.make_entity();
    add_typicals(b, 20, 10, 320, 80);
    b.add<Name>().s = "small";
    b.get<Velocity>() = Vector(6., 3.);
#   if 0
    EntityManager::FilteredEntity<Position, Velocity> efv(b);
    efv.get<Velocity>().x = 0.25;
#   endif
    auto c = emana.make_entity();
    add_typicals(c, 5, 2000, 500, 0);
    c.get<Velocity>().y = 10;

    bool all_have_fc = true;
    for (auto e : { a, b, c }) {
        if (!e.has<FrameCounter>()) all_have_fc = false;
    }
    std::cout << all_have_fc << std::endl;

    }    

    sf::RenderWindow win;
    win.setFramerateLimit(60);
    win.create(sf::VideoMode(640, 480), " ");
    while (win.isOpen()) {
        {
        sf::Event event;
        while (win.pollEvent(event)) {
            switch (event.type) {
            case sf::Event::KeyReleased:
                if (event.key.code == sf::Keyboard::Escape)
                    win.close();
                break;
            default:
                break;

            }
        }
        }
#       if 0
        emana.update_systems();
#       endif
        systems.run_systems(emana);
        emana.process_deletion_requests();
        win.clear();
        systems.render_to(win);
        win.display();
        std::this_thread::sleep_for(std::chrono::microseconds(16667));
    }
#   endif
    return 0;
}

void test_f1() {

    using namespace t1;

    std::cout <<
    t1::TestCount::GetInlineIndex<SomethingElse>::k_index << " " <<
    t1::TestCount::GetInlineIndex<NotifyDel>::k_index << " " <<
    t1::TestCount::GetInlineIndex<CompWithRef>::k_index << std::endl;

    using Entity = ecs::Entity<CompWithRef, SomethingElse, NotifyDel>;
    using EntMan  = Entity::ManagerType;

    static_assert(EntMan::HasType<NotifyDel>::k_value, "");

    {

    EntMan mana;
    Entity a = mana.make_entity();
    Entity b = mana.make_entity();

    a.add<CompWithRef>().entref = b;
    a.add<SomethingElse>();
    b.add<NotifyDel>();

    a.get<SomethingElse>();

    b.request_deletion();

    mana.process_deletion_requests();

    if (a.get<CompWithRef>().entref.has_expired()) {
        std::cout << "good" << std::endl;
    } else {
        std::cout << "bad" << std::endl;
    }

    auto entref = a.get<CompWithRef>().entref;
    try {
        Entity e(entref);
    } catch (...) {
        std::cout << "exception thrown with expired reference" << std::endl;
    }

    }
    {
    EntMan mana;
    Entity a = mana.make_entity();
    auto & se = a.add<SomethingElse>();

    auto s = std::make_unique<SomethingElse>();

    for (int i = 0; i != 1024; ++i) {
        Entity t = mana.make_entity();
        t.add<SomethingElse>();
    }
    if (se.num == 0xDEADBEEF) {
        std::cout << "caveat's definitely at play!" << std::endl;
    }

    }
}

void test_f2() {
    struct SpawnerComp {};
    struct CommonComp {};

    using EntityMan = ecs::EntityManager<SpawnerComp, CommonComp>;
    using System    = EntityMan::SystemType;

    class SimpleAddingSys final : public System {
        void update(const ContainerView & view) {
            for (auto & e : view) {
                // caught on test machine
                // on other machines, hopefully the program crashes
                assert(e);
                if (e.has<SpawnerComp>()) {
                    // really try and force a reallocation
                    for (int i = 0; i != 100; ++i) {
                        e.make_entity();
                    }
                    e.request_deletion();
                }
            }
        }
    };
    EntityMan eman;
    SimpleAddingSys ssys;
#   if 0
    eman.register_system(&ssys);
#   endif
    auto e1 = eman.make_entity();
    auto e2 = eman.make_entity();

    e1.add<CommonComp>();
    e2.add<CommonComp>();
    e2.add<SpawnerComp>();
#   if 0
    eman.update_systems();
#   endif
    eman.run_system(ssys);
    eman.process_deletion_requests();
}

void test_f3() {
    EntityManager eman;
    Entity a = eman.make_entity(), b = eman.make_entity(), c = eman.make_entity();
#   if 0 // nts: don't delete these, they're for demonstration purposes
    a.add<Velocity, Velocity>(); // error!
#   endif
#   if 0
    a.add<Displacement, Velocity, Position, Position>(); // error!
#   endif
    a.add<Velocity, Displacement, Position>();
    b.add<Velocity, Position, Displacement>() = a.get<Velocity, Position, Displacement>();
    a.remove<Position, Displacement, Velocity>();
    c.add<Velocity>();
    c.ensure<Displacement, Velocity, Position>() = b.get<Displacement, Velocity, Position>();

    {
    // IDE not co-operating
    Mass * mptr;
    Displacement * disptr;
    std::tie(mptr, disptr) = b.ptr<Mass, Displacement>();
    assert(!mptr && disptr);
    }

    const auto & ba = b;
    ba.get<Velocity, Position, Displacement>();
    ba.ptr<Velocity, Mass, Displacement>();
    assert((b.has_all<Displacement, Position, Velocity>()));
    assert(( !a.has_any<Displacement, Velocity, Position>() ));

    auto d = eman.make_entity();
    d.add<Velocity, Displacement, Position>();
    try {
        d.remove<Velocity, Displacement, Mass>();
    } catch (...) {
        ;
    }
    assert(d.has<Velocity>());
    try {
        d.add<Mass, Radius, Displacement>();
    } catch (...) {
        ;
    }
    assert(!d.has<Mass>());
}

namespace t2 {

struct Hierarchy {
    ~Hierarchy();

    EntityRef parent, left_sibling, right_sibling;
};

struct Id {
    ~Id() {
        std::cout << "entity with id #" << id << " removed." << std::endl;
    }
    int id;
};

struct Safety {
    std::size_t u = 0;
    ~Safety() { u = 0xDEADBEEF; }
};

using EntityManager = ecs::EntityManager<Hierarchy, Id, Safety>;
using Entity = EntityManager::EntityType;
using System = EntityManager::SystemType;

Hierarchy::~Hierarchy() {
    if (!left_sibling.has_expired()) {
        Entity left(left_sibling);
        left.get<Hierarchy>().right_sibling = right_sibling;
    }
    if (!right_sibling.has_expired()) {
        t2::Entity right = Entity(right_sibling);
        right.get<Hierarchy>().left_sibling = left_sibling;
    }
}

}

namespace t2 {

Entity left_most(Entity e) {
    if (!e) return e;
    if (!e.has<Hierarchy>()) return e;
    Entity rv = e;
    while (e.get<Hierarchy>().left_sibling) {
        e = Entity(e.get<Hierarchy>().left_sibling);
    }
    return rv;
}

void print_siblings(Entity e) {
    e = left_most(e);
    while (e) {
        std::cout << e.get<Id>().id << " ";
        auto ref = e.get<Hierarchy>().right_sibling;
        if (ref.has_expired()) break;
        e = Entity(ref);
    }
    std::cout << std::endl;
}

}

void test2() {
    using namespace t2;
    using EntityManager = t2::Entity::ManagerType;
#   if 1
    {
    EntityManager emana;
    t2::Entity a = emana.make_entity();
    t2::Entity b = emana.make_entity();
    t2::Entity c = emana.make_entity();
    t2::Entity d = emana.make_entity();
    d.add<Hierarchy>().parent = b;
    a.add<Id>().id = 1;
    b.add<Id>().id = 2;
    c.add<Id>().id = 3;
    a.add<Hierarchy>().right_sibling = b;
    b.add<Hierarchy>().left_sibling  = a;
    b.get<Hierarchy>().right_sibling = c;
    c.add<Hierarchy>().left_sibling  = b;
    print_siblings(a);
    b.request_deletion();
    emana.process_deletion_requests();
    print_siblings(a);

    assert(d.get<Hierarchy>().parent.has_expired());
    d.remove<Hierarchy>();
    d.add<Id>().id = 5;
    d.remove<Id>();
    EntityRef era = EntityRef::create_new_entity(ecs::detail::ReferenceManager::null_instance());
    auto erb = era;
    {
    auto erc = erb;
    }
    {
    EntityRef * testptr = nullptr;
    (void)testptr;
    {
    EntityRef tester;
    testptr = &tester;
    {
    EntityManager emana;
    tester = emana.make_entity();
    }
    }
    int k = 0;
    ++k;
    }
    }
#   endif
#   if 1
    const Safety * sptr = nullptr;
    (void)sptr;
    t2::Entity e;
    {
    EntityManager emana;
    e = emana.make_entity();
    sptr = &e.add<Safety>();
    }
    assert(e.has_expired());
    auto * ptr = e.ptr<Safety>();
    assert(!ptr);
#   endif
    {
    EntityManager emana;

    t2::Entity e = emana.make_entity();
    e.add<Safety>();

    t2::Entity a = emana.make_entity();
    a.add<Hierarchy>();
    t2::Entity b = emana.make_entity();
    b.add<Id>();

    e.request_deletion();
    a.request_deletion();
    emana.process_deletion_requests();

    }
    // entity ref instantiation by rvalue
    {
    EntityManager emana;
    t2::Entity e = emana.make_entity();
    auto e2 = e;
    EntityRef ref(std::move(e2));
    }
    {
    class EntityCounter final : public t2::System {
    public:
        int get_count() const { return m_count; }
    private:
        void update(const ContainerView & cont) override {
            m_count = 0;
            for (auto e : cont) {
                (void)e;
                ++m_count;
            }
        }
        int m_count = 0;
    };
    EntityManager emana;
    EntityCounter counter;
#   if 0
    emana.register_system(&counter);
#   endif
    auto spawner = emana.make_entity();
    emana.make_entity();
#   if 0
    emana.update_systems(); // append new entities
    emana.update_systems(); // count them
#   endif
    emana.process_deletion_requests();
    emana.run_system(counter); // append new entities
    emana.run_system(counter); // count them
    assert(counter.get_count() == 2);
    spawner.make_entity();
    spawner.make_entity();
#   if 0
    emana.update_systems();
    emana.update_systems();
#   endif
    emana.process_deletion_requests();
    emana.run_system(counter);
    emana.run_system(counter);
    assert(counter.get_count() == 4);
    }
    {
    t2::Entity e;
    t2::Entity d;
    e = d;
    }
    {
    using t1Entity = ecs::Entity<t1::CompWithRef, t1::SomethingElse, t1::NotifyDel>;
    using t1Manager = t1Entity::ManagerType;
        {
        t2::EntityManager t2mana;
        t1Manager t1mana;
        t2::Entity e2 = t2mana.make_entity();
        t1Entity e1 = t1mana.make_entity();

        EntityRef eref;
        bool caught_bad_ref = false;
        try {
            eref = e1;
            assert(eref);
            t2::Entity t2t(eref);
        }  catch (std::exception &) {
            caught_bad_ref = true;
        }
        assert(caught_bad_ref);
        }
        {
        t2::EntityManager t2mana;
        t1Manager t1mana;
        t2::Entity e2 = t2mana.make_entity();
        t1Entity e1 = t1mana.make_entity();

        EntityRef eref;
        bool caught_bad_ref = false;
        try {
            eref = e1;
            assert(eref);
            t2::Entity t2t;
            t2t = t2::Entity(std::move(eref));
        }  catch (std::exception &) {
            caught_bad_ref = true;
        }
        assert(caught_bad_ref);
        }
    }
}

namespace t3 {

struct OtherComp {
    void print_something(int i) {
        *target_write = i;
        std::cout << "printing " << i << std::endl;
    }
    int * target_write = nullptr;
};

struct CompToParent {
    static constexpr const int k_signal = 10;
    CompToParent() {}
    CompToParent(const CompToParent &) = delete;
    CompToParent(CompToParent &&) = delete;
    ~CompToParent();

    CompToParent & operator = (const CompToParent &) = delete;
    CompToParent & operator = (CompToParent &&) = delete;

    ecs::EntityRef ref_to_parent;
};

using EntityManager = ecs::EntityManager<CompToParent, OtherComp>;
using Entity = EntityManager::EntityType;

CompToParent::~CompToParent() {
    if (Entity e { ref_to_parent }) {
        if (auto * other_comp = e.ptr<OtherComp>()) {
            other_comp->print_something(k_signal);
        }
    }
}

} // end of t3 namespace

void test3() {
    using namespace t3;
    t3::EntityManager eman;
    t3::Entity entity = eman.make_entity();
    int target = CompToParent::k_signal - 10;
    entity.add<OtherComp>().target_write = &target;
    entity.add<CompToParent>().ref_to_parent = entity;
    entity.request_deletion();

    eman.process_deletion_requests();

    assert(target == CompToParent::k_signal);
}

namespace ecs {
#if 0
template <>
struct DoBeforeComponentRemoval<t2::Id> {
    void operator () (t2::Id & id) {
        std::cout << "entity with id #" << id.id << " removed." << std::endl;
    }
};

template <>
struct DoBeforeComponentRemoval<t2::Hierarchy> {
    void operator () (t2::Hierarchy & h) {
        using namespace t2;
        if (!h.left_sibling.has_expired()) {
            t2::Entity left(h.left_sibling);
            left.component<Hierarchy>().right_sibling = h.right_sibling;
        }
        if (!h.right_sibling.has_expired()) {
            t2::Entity right = t2::Entity(h.right_sibling);
            right.component<Hierarchy>().left_sibling = h.left_sibling;
        }
    }
};
#endif
}
