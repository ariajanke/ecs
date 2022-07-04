#include <common/Vector2Util.hpp>

#include <unordered_map>

namespace tn {

using VectorI = cul::Vector2<int>;

struct Cust {

};

Cust operator + (const Cust &, const Cust &) {
    return Cust();
}

void do_vector_stuff() {
    [[maybe_unused]] Cust c;
    VectorI a, b;
    a + b;
}

struct VectorHasher {
    std::size_t operator () (const VectorI & r) const
        { return r.x + r.y*1000; }
};

}


void do_other_vector_stuff() {
    using VectorI = cul::Vector2<int>;
    std::unordered_map<VectorI, tn::Cust, tn::VectorHasher> map;
    map[VectorI(1, 0)] = tn::Cust();
}

#include <SFML/System/Vector2.hpp>

#include <glm/vec2.hpp>

#include <common/Vector2.hpp>
#include <common/Vector2Util.hpp>
#include <common/SfmlVectorTraits.hpp>

#include <iostream>

namespace cul {

template <>
struct Vector2Scalar<glm::vec2> {
    using Type = glm::vec2::value_type;
};

template <>
struct Vector2Traits<glm::vec2::value_type, glm::vec2> {
    using Scal = glm::vec2::value_type;

    static constexpr const bool k_is_vector_type          = true ;
    static constexpr const bool k_should_define_operators = false;

    struct GetX {
        Scal operator () (const glm::vec2 & r) const { return r.x; }
    };
    struct GetY {
        Scal operator () (const glm::vec2 & r) const { return r.y; }
    };
    struct Make {
        glm::vec2 operator () (const Scal & x_, const Scal & y_) const
            { return glm::vec2{x_, y_}; }
    };
};

} // end of cul namespace

struct A {};

int main() {
#   if 0
    cul::ConstString abc = "hello there";
    assert(abc != "hello nobody");
#   endif
    {
    cul::Vector2<float> a, b;
    (void)(a != b);
    (void)(a == b);
    const auto & ca = a;
    const auto & cb = b;
    (void)(ca != cb);
    (void)(ca == cb);
    sf::Vector2f c(40.f, 30.f);
    static constexpr const float k_scalar = 50.f;
    c = cul::normalize(c)*k_scalar;
    }
    {
    sf::Vector2i a { 50, 20 }, b;
    a + b;
    std::cout << std::boolalpha;
    std::cout << cul::k_is_convertible_vector2<A> << std::endl;
    std::cout << cul::k_is_convertible_vector2<sf::Vector2<int>> << std::endl;
    std::cout << cul::k_is_convertible_vector2<glm::vec2> << std::endl;
    std::cout << cul::k_is_convertible_vector2<glm::vec3> << std::endl;
    std::cout << cul::k_is_convertible_vector2<cul::Vector2<int>> << std::endl;
    std::cout << cul::magnitude(a) << std::endl;

    glm::vec2 c { 4.5, -1.2 };
    glm::vec2 d { 10, -1 };
    c = cul::normalize(c);
    c = cul::project_onto(c, d);
    std::cout << c.x << ", " << c.y << std::endl;
    std::cout << cul::magnitude(c) << std::endl;
    }
    {
    glm::vec2 a {  0,  0 };
    glm::vec2 b { 10, 10 };
    glm::vec2 c {  4,  0 };
    auto cpt = cul::find_closest_point_to_line(a, b, c);
    std::cout << cpt.x << ", " << cpt.y << std::endl;
    }
    {
    // check if operators are okay
    using VecD = cul::Vector2<double>;
    VecD a, b, c;
    (void)(a + b);
    (void)(a - c);
    a += b;
    a -= c;
    (void)-a;
    double s = 10.;
    a*s;
    a/s;
    a *= s;
    a /= s;
    (void)(a == b);
    (void)(a != c);
    }
    {
    // check if operators are okay
    using VecD = cul::Size2<double>;
    VecD a, b, c;
    (void)(a + b);
    (void)(a - c);
    a += b;
    a -= c;
    (void)-a;
    double s = 10.;
    a*s;
    a/s;
    a *= s;
    a /= s;
    (void)(a == b);
    (void)(a != c);
    }
    {
    using RectD = cul::Rectangle<double>;
    using VecD  = cul::Vector2<double>;
    using SizeD = cul::Size2<double>;
    RectD rect(VecD(10, 90), SizeD(78, 10));
    [[maybe_unused]] auto [r, sz] = cul::decompose(rect);
    [[maybe_unused]] auto b = rect;
    (void)r;
    (void)sz;
    }

    // run through each function, once with another library's vector type
    // and once with the internal vector type
    using VecD = cul::Vector2<double>;
    using VecF = cul::Vector2<float>;
    using VecI = cul::Vector2<int>;
    using cul::convert_to;
    {
        auto a = cul::magnitude(VecI(100, 190));
        auto b = cul::magnitude(sf::Vector2i(100, 190));
        assert(a == b);
    }
    {
        auto a = cul::normalize(VecF(-89.f, 10.f));
        auto b = cul::normalize(glm::vec2(-89.f, 10.f));
        cul::normalize(sf::Vector2f(10.f, -56.f));
        // their values should actually be exactly equal
        assert(a == convert_to<VecF>(b));
        auto abm_a = cul::are_within(a, convert_to<VecF>(b), 0.01f);
        auto abm_b = cul::are_within(convert_to<glm::vec2>(a), b, 0.1f);
        assert(abm_a && abm_b);
    }
    {
        static constexpr const auto k_pi =  cul::k_pi_for_type<float>;
        VecF a(5, 4);
        sf::Vector2f b(6, 7);
        a = cul::rotate_vector(a,  k_pi*0.5f );
        b = cul::rotate_vector(b, -k_pi*0.25f);
        std::cout << "vector (5, 4) rotated " << a.x << ", " << a.y
                  << "\nvector (6, 7) rotated " << b.x << ", " << b.y << std::endl;
    }
    {
        VecI a(5, 4), b(-2, 3);
        glm::vec2 c(6.5, 7.25), d(8.9, -1.4);
        [[maybe_unused]] int a_dot = dot(a, b);
        [[maybe_unused]] double c_dot = cul::dot(c, d);
        [[maybe_unused]] int a_cross = cross(a, b);
        [[maybe_unused]] double c_cross = cul::cross(c, d);
        VecD e(5, 4), f(-2, 3);
        auto ef_ang = cul::angle_between(e, f);
        auto ef_dang = cul::directed_angle_between(e, f);
        assert(cul::magnitude(ef_ang - cul::magnitude(ef_dang)) < 0.005);

        [[maybe_unused]] auto cd_ang  = cul::angle_between(c, d);
        [[maybe_unused]] auto cd_dang = cul::directed_angle_between(c, d);
    }
    {
        VecD a(100, 45), b(-1, 0.1);
        glm::vec2 c(6.5, 7.25), d(8.9, -1.4);
        auto abp = cul::project_onto(a, b);
        auto cdp = cul::project_onto(c, d);
        std::cout << "vector projection abp: " << abp.x << ", " << abp.y
                  << "\n projection cdp: " << cdp.x << ", " << cdp.y << std::endl;
    }
    {
        VecD ai(0, 0), bi(1, 1), ci(0, 1), di(1, 0);
        auto intxi = cul::find_intersection(ai, bi, ci, di);
        sf::Vector2f ao(0, 0), bo(1, 1), co(0, 1), do_(1, 0);
        auto intxo = cul::find_intersection(ao, bo, co, do_);
        std::cout << "vector intersection i: " << intxi.x << ", " << intxi.y
                  << "\n intersection o: " << intxo.x << ", " << intxo.y << std::endl;
    }
    // will need tests for round_to
    cul::round_to<int>(VecF(10.5, 11.3));
    {
        VecD s(34, 12), t(100, 20), g(0, -10);
        double speed = 100;
        auto [s0, s1] = cul::find_velocities_to_target(s, t, g, speed);
        auto so = convert_to<glm::vec2>(s);
        auto to = convert_to<glm::vec2>(t);
        auto go = convert_to<glm::vec2>(g);
        auto [s0o, s1o] = cul::find_velocities_to_target(so, to, go, float(speed));
        VecD{0., 9.} - VecD{8.9, 4.3};
        assert(cul::magnitude(s0 - convert_to<VecD>(s0o)) < 0.0005);
        assert(cul::magnitude(s1 - convert_to<VecD>(s1o)) < 0.0005);
    }
    {
        VecD a(5, 4), b(10, 12), c(7, 5);
        auto d = cul::find_closest_point_to_line(a, b, c);
        sf::Vector2f ao(-2, 3), bo(4, -3), co(0, 0);
        auto do_ = cul::find_closest_point_to_line(ao, bo, co);
        std::cout << "vector closest point i: " << d.x << ", " << d.y
                  << "\n closest point o: " << do_.x << ", " << do_.y << std::endl;
    }
    {
        std::cout << std::boolalpha
                  << cul::is_real(sf::Vector2f(900, 5.12e26f)) << " "
                  << cul::is_real(sf::Vector2i(1, 1)) << " "
                  << cul::is_real(cul::get_no_solution_sentinel<VecD>()) << std::endl;
    }
    assert(cul::are_within(
        cul::rotate_vector(VecF(1, 0), cul::k_pi_for_type<double>*0.5),
        VecF(0, 1),
        0.0005f));
    {
    using RectI = cul::Rectangle<int>;
    assert( cul::overlaps(RectI(0, 0, 10, 10), RectI(5, 5, 12, 2)));
    assert(!cul::overlaps(RectI(0, 0, 2 , 10), RectI(5, 5, 12, 2)));
    }

    // test out as best as possible constexpr
    {
    using SizeI = cul::Size2<int>;
    [[maybe_unused]] std::array<int, VecI{0, 5}.y> a;
    static_assert(VecI{-1, 4} == VecI{0, 4} + VecI{-1, 0}, "");
    static_assert(VecI{-1, 4} != VecI{0, 4} - VecI{-1, 0}, "");
    static_assert(VecI{-2, 2} == VecI{-1, 1}*2, "");
    static_assert(VecI{-2, 2} == 2*VecI{-1, 1}, "");
    static_assert(VecI{6, 3}/3 == VecI{2, 1}, "");
    static_assert(-VecI{2, 4} == VecI{-2, -4}, "");

    static_assert(VecI{}.x == 0, "");
    static constexpr const VecD k_grav{0, -9.81};
    static_assert(k_grav.y < 0., "");
    static_assert(VecI{k_grav}.y < 0, "");
    static_assert(SizeI{}.width == 0, "");
    static_assert(SizeI{2, 2}.width == 2, "");

    [[maybe_unused]] static constexpr const auto k_no_sol =
        cul::k_no_solution_for_v2<double>;
    static_assert(std::is_same_v<decltype(k_no_sol.x), double>, "");
    }

    return 0;
}
