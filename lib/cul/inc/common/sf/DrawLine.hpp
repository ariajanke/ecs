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

#pragma once

#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>

#include <array>

namespace cul {

/** Simple drawable line.
 *
 *  It has four write-only attributes. As it's only meant to represent a line
 *  on the screen itself. While these are knowable, they are hidden.
 *
 *  Two, each are points of the line, a beginning and end. There is no way to
 *  distinguish between the two (by design).
 *  Thickness, which must be 1 or greater.
 *  Color, which maybe any color, though the entire line is a single color.
 *
 *  @note Hopefully this design turns out to be "tight" enough for a library.
 */
class DrawLine final : public sf::Drawable {
public:
    static constexpr const int k_vertex_count = 4;
    using Iterator = std::array<sf::Vertex, k_vertex_count>::const_iterator;
    static constexpr const auto k_primitive_type = sf::PrimitiveType::TriangleStrip;

    /** By default, lines are "zero" sized with no thickness. */
    DrawLine() {}

    /** Creates a draw line, setting all its attributes all at once.
     *  The two points are the vector parameters, with a thickness and color
     *  paramter too.
     */
    DrawLine(sf::Vector2f, sf::Vector2f, float thickness, sf::Color);

    /** Sets the two points of the draw line, points are given as parameters in
     *  any order.
     */
    void set_points(sf::Vector2f, sf::Vector2f);

    /** Sets the color of the line. */
    void set_color(sf::Color) noexcept;

    /** Sets the thickness of the line. */
    void set_thickness(float);

    /** Moves the line by some given displacement. */
    void move(sf::Vector2f);

    /** @returns begin iterator to the verticies
     *  @note it maybe desirable to grab the verticies and push them somewhere
     *        else (for future rendering perhaps?)
     */
    Iterator begin() const;

    /** @returns end iterator to the verticies */
    Iterator end() const;

private:
    void draw(sf::RenderTarget &, sf::RenderStates) const final;

    float thickness() const;

    sf::Color color() const;

    std::tuple<sf::Vector2f, sf::Vector2f> points() const;

    void set_attributes(sf::Vector2f, sf::Vector2f, float thickness, sf::Color) noexcept;

    std::array<sf::Vertex, k_vertex_count> m_verticies;
};

} // end of cul namespace
