/****************************************************************************

    MIT License

    Copyright 2021 Aria Janke

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.

*****************************************************************************/

#include <common/sf/DrawLine.hpp>

#include <common/Util.hpp>
#include <common/Vector2Util.hpp>
#include <common/SfmlVectorTraits.hpp>
#include <common/sf/DrawRectangle.hpp>

#include <SFML/Graphics/RenderTarget.hpp>

#include <cassert>

namespace {

using namespace cul::exceptions_abbr;
using std::get;
using Iterator = cul::DrawLine::Iterator;
using cul::DrawRectangle;

constexpr const int  k_a_up   = DrawRectangle::k_top_left;
constexpr const int  k_a_down = DrawRectangle::k_bottom_left;
constexpr const int  k_b_down = DrawRectangle::k_bottom_right;
constexpr const int  k_b_up   = DrawRectangle::k_top_right;
constexpr const auto k_pi     = cul::k_pi_for_type<float>;

float verify_thickness(const char * caller, float);

sf::Vector2f verify_point(const char * caller, sf::Vector2f);

sf::Vector2f make_unit_start();

sf::Vector2f to_polar(float angle, float distance);

} // end of <anonymous> namespace

namespace cul {

DrawLine::DrawLine
    (sf::Vector2f a, sf::Vector2f b, float thickness, sf::Color color)
{
    static constexpr const auto k_call_name = "DrawLine::DrawLine";
    set_attributes(verify_point(k_call_name, a), verify_point(k_call_name, b),
                   verify_thickness(k_call_name, thickness), color);
}

void DrawLine::set_points(sf::Vector2f a, sf::Vector2f b) {
    static constexpr const auto k_call_name = "DrawLine::set_points";
    set_attributes(verify_point(k_call_name, a), verify_point(k_call_name, b),
                   thickness(), color());
}

void DrawLine::set_color(sf::Color color) noexcept {
    set_attributes(get<0>(points()), get<1>(points()), thickness(), color);
}

void DrawLine::set_thickness(float t) {
    set_attributes(get<0>(points()), get<1>(points()),
                   verify_thickness("DrawLine::set_thickness", t), color());
}

void DrawLine::move(sf::Vector2f r) {
    verify_point("DrawLine::move", r);
    for (auto & vtx : m_verticies) vtx.position += r;
}

Iterator DrawLine::begin() const { return m_verticies.begin(); }

Iterator DrawLine::end() const { return m_verticies.end(); }

/* private */ void DrawLine::draw
    (sf::RenderTarget & target, sf::RenderStates states) const
{ target.draw(m_verticies.data(), m_verticies.size(), k_primitive_type, states); }

/* private */ float DrawLine::thickness() const {
    return cul::magnitude(  m_verticies[k_a_up  ].position
                          - m_verticies[k_a_down].position);
}

/* private */ sf::Color DrawLine::color() const
    { return m_verticies[k_a_up].color; }

/* private */ std::tuple<sf::Vector2f, sf::Vector2f> DrawLine::points() const {
    using std::make_tuple;
    const auto & v = m_verticies;
    return make_tuple((v[k_a_up].position + v[k_a_down].position)*0.5f,
                      (v[k_b_up].position + v[k_b_down].position)*0.5f);
}

/* private */ void DrawLine::set_attributes
    (sf::Vector2f a, sf::Vector2f b, float thickness, sf::Color color) noexcept
{
    assert(is_real(a));
    assert(is_real(b));
    assert(thickness >= 1.f);

    auto init_angle = directed_angle_between(a - b, make_unit_start());
    auto mk_vertex = [thickness, init_angle, color](sf::Vector2f pt, float ang_dir) {
        auto theta = init_angle + ang_dir*(k_pi * 0.5f);
        return sf::Vertex(pt + to_polar(theta, thickness * 0.5f), color);
    };

    m_verticies[k_a_up  ] = mk_vertex(a,  1.f);
    m_verticies[k_a_down] = mk_vertex(a, -1.f);
    m_verticies[k_b_down] = mk_vertex(b, -1.f);
    m_verticies[k_b_up  ] = mk_vertex(b,  1.f);
}

} // end of cul namespace

namespace {

float verify_thickness(const char * caller, float t) {
    static constexpr const auto k_error = 0.0005f;
    if (t > 1.f) return t;
    if (t > 1.f - k_error) return 1.f;
    throw InvArg(std::string(caller) + ": thickness must be greater than one, "
                 "or \"close enough\" to one to round up.");
}

sf::Vector2f verify_point(const char * caller, sf::Vector2f r) {
    if (cul::is_real(r)) return r;
    throw InvArg(std::string(caller) + ": point must be a real vector.");
}

sf::Vector2f make_unit_start() { return sf::Vector2f(1.f, 0.f); }

sf::Vector2f to_polar(float angle, float distance)
    { return cul::rotate_vector(make_unit_start(), angle)*distance; }

} // end of <anonymous> namespace
