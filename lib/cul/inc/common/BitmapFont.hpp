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

#pragma once

#include <common/Vector2.hpp>
#include <common/SubGrid.hpp>

#include <utility>

namespace cul {

/** A simple bitmap font, maps characters to texture rectangles. */
class BitmapFont {
public:
    enum BuiltinFont {
        k_8x8_font, k_8x8_highlighted_font, k_8x16_font, k_8x16_highlighted_font,
        k_builtin_font_count
    };

    /** The character that's used if the given characters cannot fit in a char
     *  type.
     */
    static constexpr const char k_substitution = '?';

    virtual ~BitmapFont() {}

    /** @returns the texture and location of the character rectangle
     *  @note    this function will never be called for the tab, new line, or
     *           return carriage characters
     */
    virtual Vector2<int> operator () (char) const = 0;

    /** @returns the size of each character in pixels */
    virtual Size2<int> character_size() const = 0;
};

/** @brief Represents a bitmap font as abstract "pixels".
 *
 *  You can take any of the builtin fonts from this format and convert to
 *  whatever type you'd like.
 */
class GridBitmapFont : public BitmapFont {
public:
    enum Pixel : uint8_t {
        /** for any unset pixel, a background color or transparency */
        k_unset,
        /** for any pixel that is a highlight boundry color, to help the text
         *  pop out in multi-colored environments
         */
        k_highlight,
        /** for any set pixel, black and white are common, possible choices */
        k_set
    };

    /** @returns a grid representing pixels */
    virtual const Grid<Pixel> & pixels() const = 0;

    /** Loads one builtin fonts without any instance associated with it. If the
     *  font is already loaded this function does nothing.
     *  @note This will require you to link to the library if called.
     */
    static const GridBitmapFont & load_builtin_font(BuiltinFont);
};

} // end of cul namespace
