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

#include <common/sf/DrawText.hpp>

#include <common/Util.hpp>
#include <common/SfmlVectorTraits.hpp>
#include <common/Vector2Util.hpp>
#include <common/sf/Util.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <memory>

#include <cassert>

namespace {

using namespace cul::exceptions_abbr;
using BuiltinFont = cul::BitmapFont::BuiltinFont;

constexpr const int k_top_left     = 0;
constexpr const int k_top_right    = 1;
constexpr const int k_bottom_left  = 2;
constexpr const int k_bottom_right = 3;
constexpr const int k_rect_count   = 4;

const cul::SfBitmapFont & get_builtin_font(BuiltinFont);

std::array<sf::Vector2f, k_rect_count> to_texture_positions
    (const cul::Rectangle<int> &);

} // end of <anonymous> namespace

namespace cul {

/* static */ const SfBitmapFont & SfBitmapFont::load_builtin_font
    (BuiltinFont bfont)
    { return get_builtin_font(bfont); }

/* static */ sf::Color DrawText::color_for_pixel(Pixel px) {
    using Gbf = GridBitmapFont;
    switch (px) {
    case Gbf::k_set      : return sf::Color::White;
    case Gbf::k_unset    : return sf::Color(0, 0, 0, 0);
    case Gbf::k_highlight: return sf::Color::Black;
    }
    throw RtError("DrawText::color_for_pixel: bad branch, bad value for pixel.");
}

void DrawText::load_builtin_font(BuiltinFont bfont) {
    m_font = &get_builtin_font(bfont);
    m_verticies.clear();
}

void DrawText::assign_font(const SfBitmapFont & font) {
    m_font = &font;
    m_verticies.clear();
}

void DrawText::assign_font_from(const DrawText & dtext) {
    m_font = dtext.font();
    m_verticies.clear();
}

void DrawText::move(sf::Vector2f r) {
    for (auto & vtx : m_verticies)
        vtx.position += r;
}

Size2<float> DrawText::measure_text(int character_count) const {
    if (!m_font) {
        throw RtError("DrawText::measure_text: a font is needed in order to "
                      "measure the size of some number of characters.");
    }
    auto size_ = m_font->character_size();
    size_.width *= character_count;
    return Size2<float>(size_);
}

const SfBitmapFont * DrawText::font() const { return m_font; }

std::vector<sf::Vertex> DrawText::give_verticies() {
    auto rv = std::move(m_verticies);
    m_verticies.clear();
    return rv;
}

const std::vector<sf::Vertex> & DrawText::verticies() const
    { return m_verticies; }

/* private */ void DrawText::draw
    (sf::RenderTarget & target, sf::RenderStates states) const
{
    states.texture = &m_font->texture();
    target.draw(m_verticies.data(), m_verticies.size(), k_primitive_type, states);
}

/* private */ sf::Vector2f DrawText::push_character(sf::Vector2f r, char code) {
    if (!is_real(r)) {
        throw InvArg("DrawText::push_character: position must be a real vector.");
    }
    if (!m_font) {
        throw RtError("DrawText::push_character: cannot push a character without a font.");
    }

    using sf::Vertex;
    Rectangle<int> texture_rect((*m_font)(code), m_font->character_size());
    auto texture_pos = to_texture_positions(texture_rect);
    auto mk_vertex = [&texture_pos, r](int idx) {
        auto diff_for = [&texture_pos](int idx) {
            assert(idx >= 0 && idx < k_rect_count);
            return texture_pos[idx] - texture_pos[k_top_left];
        };
        const auto & k_white = sf::Color::White;
        return Vertex(r + diff_for(idx), k_white, texture_pos[idx]);
    };

    std::array<Vertex, k_rect_count> verticies;
    verticies[k_top_left    ] = mk_vertex(k_top_left    );
    verticies[k_top_right   ] = mk_vertex(k_top_right   );
    verticies[k_bottom_left ] = mk_vertex(k_bottom_left );
    verticies[k_bottom_right] = mk_vertex(k_bottom_right);

    m_verticies.insert(m_verticies.end(), verticies.begin() + 0, verticies.begin() + 3);
    m_verticies.insert(m_verticies.end(), verticies.begin() + 1, verticies.begin() + 4);
    return r + sf::Vector2f(float(texture_rect.width), 0.f);
}

} // end of cul namespace

namespace {

using cul::GridBitmapFont;
using cul::BitmapFont;
using FontPtr = std::unique_ptr<const cul::SfBitmapFont>;

class SfBitmapFontComplete final : public cul::SfBitmapFont {
public:
    const sf::Texture & texture() const final { return m_texture; }

    cul::Vector2<int> operator () (char code) const final
        { return (*m_font)(code); }

    cul::Size2<int> character_size() const final
        { return m_font->character_size(); }

    void setup(const GridBitmapFont &);

    bool has_font() const { return m_font; }

private:
    sf::Texture m_texture;
    const GridBitmapFont * m_font = nullptr;
};

const cul::SfBitmapFont & get_builtin_font(BuiltinFont bfont) {
    static std::array<FontPtr, BitmapFont::k_builtin_font_count> s_fonts
        = cul::make_filled_array<BitmapFont::k_builtin_font_count, FontPtr>(nullptr);
    if (s_fonts[bfont]) return *s_fonts[bfont];
    auto ptr = std::make_unique<SfBitmapFontComplete>();
    ptr->setup(GridBitmapFont::load_builtin_font(bfont));
    assert(ptr->has_font());
    s_fonts[bfont] = std::move(ptr);
    return *s_fonts[bfont];
}

std::array<sf::Vector2f, k_rect_count> to_texture_positions
    (const cul::Rectangle<int> & rect)
{
    using namespace cul;
    using VecF = sf::Vector2f;
    auto right  = float(right_of(rect));
    auto bottom = float(bottom_of(rect));
    auto tl = convert_to<VecF>(top_left_of(rect));
    std::array<VecF, k_rect_count> rv;
    rv[k_top_left    ] = tl;
    rv[k_top_right   ] = VecF(right, tl.y  );
    rv[k_bottom_left ] = VecF(tl.x , bottom);
    rv[k_bottom_right] = VecF(right, bottom);
    return rv;
}

// ----------------------------------------------------------------------------

void SfBitmapFontComplete::setup(const GridBitmapFont & font) {
    // terse, but computationally intense
    using namespace cul;
    m_font = &font;
    Grid<sf::Color> grid_texture;
    const auto & pixels = m_font->pixels();
    grid_texture.set_size(pixels.width(), pixels.height(), sf::Color::Red);
    for (Vector2<int> r; r != grid_texture.end_position(); r = grid_texture.next(r)) {
        grid_texture(r) = DrawText::color_for_pixel(pixels(r));
    }
    m_texture.loadFromImage(to_image(grid_texture));
}

} // end of <anonymous> namespace
