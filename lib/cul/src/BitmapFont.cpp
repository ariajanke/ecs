/****************************************************************************

    MIT License

    Copyright (c) 2021 Aria Janke

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

#include <common/BitmapFont.hpp>
#include <common/Util.hpp>
#include <common/StringUtil.hpp>

#include "sf-8x8Font.hpp"
#include "sf-8x16Font.hpp"

#include <memory>

#include <cassert>
#include <cstring>
#include <cmath>

namespace {

using namespace cul::exceptions_abbr;
using GridBitmapFontPtr = std::unique_ptr<const cul::GridBitmapFont>;
using BuiltinFont       = cul::BitmapFont::BuiltinFont;

GridBitmapFontPtr make_builin_font(BuiltinFont);

} // end of <anonymous> namespace

namespace cul {

/* static */ const GridBitmapFont & GridBitmapFont::load_builtin_font
    (BuiltinFont bf)
{
    // if initialized, they'll never be written to
    static auto s_builtin_fonts = make_filled_array
        <BitmapFont::k_builtin_font_count, GridBitmapFontPtr, std::nullptr_t>(nullptr);
    if (s_builtin_fonts[bf]) return *s_builtin_fonts[bf];
    s_builtin_fonts[bf] = make_builin_font(bf);
    return *s_builtin_fonts[bf];
}

} // end of cul namespace

namespace {

using cul::Grid;
using GetCharFunc = const char * (*)(char);

inline cul::Vector2<int> get_nowhere() { return cul::Vector2<int>(-1, -1); }

class GridBitmapFontComplete final : public cul::GridBitmapFont {
public:
    using Vector = cul::Vector2<int>;
    using Size   = cul::Size2  <int>;

    const Grid<Pixel> & pixels() const final { return m_pixels; }

    Vector operator () (char code) const final;

    Size character_size() const final { return m_character_size; }

    void setup(GetCharFunc get_char, Size char_size, bool has_highlight);

private:
    static constexpr const auto k_char_map_size = std::numeric_limits<char>::max();
    void add_highlights();

    Grid<Pixel> m_pixels;
    cul::Size2<int> m_character_size;
    std::array<cul::Vector2<int>, k_char_map_size> m_char_map
        = cul::make_filled_array<k_char_map_size>(get_nowhere());
};

GridBitmapFontPtr make_builin_font(BuiltinFont bf) {
    using Size = cul::Size2<int>;
    using BFont = cul::BitmapFont;
    bool has_highlight = false;
    Size size;
    GetCharFunc source_f = nullptr;
    switch (bf) {
    case BFont::k_8x8_font:
        size = Size(8, 8);
        source_f = cul::detail::get_8x8_char;
        break;
    case BFont::k_8x16_font:
        size = Size(8, 16);
        source_f = cul::detail::get_8x16_char;
        break;
    case BFont::k_8x8_highlighted_font:
        has_highlight = true;
        source_f = cul::detail::get_8x8_char;
        size = Size(8 + 2, 8 + 2);
        break;
    case BFont::k_8x16_highlighted_font:
        has_highlight = true;
        size = Size(8 + 2, 16 + 2);
        source_f = cul::detail::get_8x16_char;
        break;
    default: throw InvArg("<cul anonymous>::make_builin_font: specified "
                          "builtin font is not a valid value.");
    }
    auto rv = std::make_unique<GridBitmapFontComplete>();
    rv->setup(source_f, size, has_highlight);
    return rv;
}

// ----------------------------------------------------------------------------

static constexpr const auto k_printable_characters
    = "`1234567890-=qwertyuiop[]\\asdfghjkl;'zxcvbnm,./~!@#$%^&*()_+QWERTY"
      "UIOP{}|ASDFGHJKL:\"ZXCVBNM<>? ";

static constexpr const auto k_printable_len
    = cul::find_str_len(k_printable_characters);

static constexpr const auto k_printable_end
    = cul::find_str_end(k_printable_characters);

cul::Size2<int> get_size_in_chars(cul::Size2<int> char_size);

cul::Vector2<int> GridBitmapFontComplete::operator () (char code) const {
    if (code < 0) code = BitmapFont::k_substitution;
    auto rv = m_char_map[code];
    assert(rv != get_nowhere());
    return rv;
}

void GridBitmapFontComplete::setup
    (GetCharFunc get_char, Size char_size, bool has_highlight)
{
    using namespace cul;
    assert(get_char);
    assert(char_size.width != 0 && char_size.height != 0);

    auto size_in_chars = get_size_in_chars(char_size);
    m_character_size = char_size;
    m_pixels.set_size(size_in_chars.width *char_size.width ,
                      size_in_chars.height*char_size.height,
                      k_unset);

    auto get_px = [/*has_highlight, char_size*/](Vector r, int char_width, const char * px_string) {
#       if 0
        if (has_highlight) {
            if (   r.x == 0 || r.y == 0 || r.x == char_size.width + 1
                || r.y == char_size.height + 1)
            { return k_unset; }
            r -= Vector(1, 1);
        }
#       endif
        if (detail::is_on_pixel(px_string[r.x + r.y*char_width]))
            { return k_set; }
        return k_unset;
    };

    Vector r;
    for (auto itr = k_printable_characters; itr != k_printable_end; ++itr) {
        auto dest = make_sub_grid(m_pixels,
            r         + (has_highlight ? Vector(1, 1) : Vector()),
            char_size - (has_highlight ? Size  (2, 2) : Size  ()));
        auto char_str = get_char(*itr);
        m_char_map[*itr] = r;
        for (Vector r; r != dest.end_position(); r = dest.next(r)) {
            dest(r) = get_px(r, dest.width(), char_str);
        }
        r.x += char_size.width;
        if (r.x >= char_size.width*size_in_chars.width) {
            r.x = 0;
            r.y += char_size.height;
        }
    }
    for (auto & r : m_char_map) {
        if (r == get_nowhere())
            r = m_char_map[BitmapFont::k_substitution];
    }
    if (has_highlight) add_highlights();
}

/* private */ void GridBitmapFontComplete::add_highlights() {
    static auto k_neighbors = {
        Vector(1, 0), Vector(-1, 0), Vector(0,  1), Vector( 0, -1),
        Vector(1, 1), Vector(-1, 1), Vector(1, -1), Vector(-1, -1),
    };
    for (Vector r; r != m_pixels.end_position(); r = m_pixels.next(r)) {
        if (m_pixels(r) != k_set) continue;
        for (auto v : k_neighbors) {
            assert(m_pixels.has_position(r + v));
            if (m_pixels(r + v) == k_set) continue;
            m_pixels(r + v) = k_highlight;
        }
    }
}

// ----------------------------------------------------------------------------

cul::Size2<int> get_size_in_chars(cul::Size2<int> char_size) {
    using namespace cul;
    int width_in_chars, height_in_chars;
    auto total_pixels = char_size.width*char_size.height*k_printable_len;
    width_in_chars  = round_to<int>(std::sqrt(total_pixels)) / char_size.width + 1;
    height_in_chars =    k_printable_len / width_in_chars
                      + ((k_printable_len % width_in_chars) ? 1 : 0);
    assert(total_pixels <=  width_in_chars*char_size.width
                           *height_in_chars*char_size.height);
    return Size2<int>(width_in_chars, height_in_chars);
}

} // end of <anonymous> namespace
