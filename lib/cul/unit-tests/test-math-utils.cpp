#include <common/Util.hpp>
#include <common/Vector2Util.hpp>

#include <vector>
#include <set>
#include <numeric>
#include <iostream>

#include <cassert>

// don't whipe somethings in release mode!
inline void verify(bool b) { assert(b); }
// ... but... how is this good at tests then?

static void test_v2();

int main() {
    // purpose: just make sure it compiles!
    // maybe test a few outputs
    // Util header
    {
    static constexpr const int k_samp_size = 100;
    std::vector<int> samp;
    samp.resize(k_samp_size);
    std::iota(samp.begin(), samp.end(), 0);
    struct OrderTuple {
        bool operator () (const cul::Tuple<int, int> & lhs, const cul::Tuple<int, int> & rhs) const {
            static auto get_idx = [](const cul::Tuple<int, int> & t)
                { return std::get<0>(t) + std::get<1>(t)*k_samp_size; };
            return get_idx(lhs) < get_idx(rhs);
        }
    };

    std::set<cul::Tuple<int, int>, OrderTuple> check;
    cul::quad_range(samp.begin(), samp.end(), [&check](int i, int j) {
        auto as_tuple = std::tuple(std::min(i, j), std::max(i, j));
        verify(check.insert(as_tuple).second);
    });
    int i = 0;
    cul::quad_range(samp.begin(), samp.end(),[&i](int, int) {
        ++i;
        return cul::fc_signal::k_break;
    });
    assert(i == 1);
    cul::quad_range(samp, [](int, int) {});
    cul::quad_range({ 1, 2, 3, 4, 5, 6, 7, 8 }, [](int, int) {});
    }
    cul::magnitude(-10  );
    cul::magnitude(-10.0);
    cul::magnitude(-10.f);
    assert(cul::normalize(-10) == -1);
    assert(cul::normalize( 89) ==  1);
    cul::normalize(-10.0);
    cul::normalize(10.f);
    cul::normalize([]() { return 10; }());
    assert(cul::is_nan(0. / 0.)); // I believe this forces a nan?
    assert(!cul::is_nan(0u));
    assert(!cul::is_real(std::numeric_limits<double>::infinity()));
    cul::is_real(100);
    assert(cul::round_to<int>(100.5) == 101);
    static constexpr const double k_chosen_const = 0.1491596009;
    auto [high_false, low_true] = cul::find_smallest_diff<double>([](double x) {
        return x > k_chosen_const;
    });
    assert(high_false < k_chosen_const);
    assert(low_true   > k_chosen_const);
    cul::find_highest_false<double>([](double x) { return x > k_chosen_const; });
    cul::find_lowest_true<double>([](double x) { return x > k_chosen_const; });
    // Vector2 header
    test_v2();
}

static void test_v2() {
    using namespace cul;
    using VectorI = cul::Vector2<int>;
    using VectorD = cul::Vector2<double>;

    static constexpr const auto k_pi_d = k_pi_for_type<double>;
    [[maybe_unused]] static constexpr const auto k_pi_i = k_pi_for_type<int>;
    // therefore pi is exactly three QED

    [[maybe_unused]] int i1 = magnitude(VectorI(100, 100));
    [[maybe_unused]] double d1 = magnitude(VectorD(100, 100));

    [[maybe_unused]] VectorD vd1 = normalize(VectorD(100, 200));

    vd1 = rotate_vector(VectorD(1.2, 9.8), k_pi_d*0.5);
    i1  = dot(VectorI(20, 12), VectorI(5, 4));
    d1  = angle_between(VectorD(5.4, 3.4), VectorD(10., -9.01));
    vd1 = project_onto(VectorD(12., -100.), VectorD(1., 1.1));
    vd1 = find_intersection(VectorD(0, 0), VectorD(1, 1), VectorD(100, 1), VectorD(101, 0));
    assert(!is_real(vd1));
    assert(!is_real(vd1 = get_no_solution_sentinel<VectorD>()));

    vd1 = find_closest_point_to_line(VectorD(0, 0), VectorD(1, 1), VectorD(0.2, 0.4));

    using SizeI = cul::Size2<int>;
    using SizeD = cul::Size2<double>;
    using RectI = cul::Rectangle<int>;
    using RectD = cul::Rectangle<double>;

    RectD rectd = RectD(0, 40.5, 200.5, 100);
    RectI recti = RectI(0, 0, 240, 125);
    SizeD szd;
    SizeI szi;
    assert(szi.width == 0);
    set_top_left_of(rectd, 100., 50.5);
    set_size_of(recti, 100, 120);
    set_top_left_of(rectd, VectorD(10., 4.5));
    set_size_of(rectd, szd = make_size(100., 40.));
    d1 = right_of(rectd);
    i1 = bottom_of(recti);
    szd = size_of(rectd);
    vd1 = center_of(rectd);
    recti = find_rectangle_intersection(recti, RectI(10, 5, 1000, 50));
    d1 = area_of(rectd);
    recti = compose(VectorI(15, 45), SizeI(size_of(rectd)));
    std::tie(vd1, szd) = decompose(rectd);
    [](SizeI) {}(szi);
    [](SizeD) {}(SizeD(szi));
}
