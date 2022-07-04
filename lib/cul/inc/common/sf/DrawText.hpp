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

#include <common/Vector2.hpp>
#include <common/BitmapFont.hpp>
#include <common/SfmlVectorTraits.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>

#include <vector>
#include <string>

namespace cul {

/** A bitmap font which works with SFML. To render to a SFML render target,
 *  fonts need to specify a texture.
 */
class SfBitmapFont : public BitmapFont {
public:
    /** @returns texture used by the font */
    virtual const sf::Texture & texture() const = 0;

    /** Loads one builtin fonts. If the font is already loaded this function
     *  does nothing.
     *
     *  This function will also preload builtin GridBitmapFonts.
     *
     *  @returns reference to the loaded builtin font
     */
    static const SfBitmapFont & load_builtin_font(BuiltinFont);
};

/** Offers simple bitmap font based string rendering.
 *
 *  This should only be used for single line rendering. The intent of this
 *  class is to provide a simple helpful too for further rendering of bitmap
 *  text.
 *
 *  Alone, this can be used for flash pop-up text, or perhaps debug lines, or
 *  something else.
 *
 *  A lot of my projects use bitmap fonts for it's own style/my own shear
 *  ineptitude.
 *
 *  @note Should be useable by ASGL
 *  @note It is always possible to design this for wider characters, for
 *        anything other than LtR scripts are considered out of scope of this
 *        simple utility
 */
class DrawText final : public sf::Drawable {
public:
    template <typename T>
    using BasicString = std::basic_string<T>;

    template <typename T>
    using BsIter = typename std::basic_string<T>::const_iterator;

    using BuiltinFont = BitmapFont::BuiltinFont;
    using Pixel       = GridBitmapFont::Pixel;

    /** Primitive type which should be used to render verticies gotten from
     *  vertex iterators.
     */
    static constexpr const auto k_primitive_type = sf::PrimitiveType::Triangles;

    /** Character used in place of another if there's no texture rectangle for
     *  it.
     */
    static constexpr const char k_substitution = BitmapFont::k_substitution;

    /** @returns the color used for the given pixel classification. */
    static sf::Color color_for_pixel(Pixel);

    /** Loads the given builtin bitmap font. */
    void load_builtin_font(BuiltinFont);

    /** Assigns the font of the text.
     *  @note this has an effect of clearing the text, make sure to assign
     *        the font before doing much else with this object
     *  @warning the given font must survive for the life of the draw text
     *           (and the font's texture must survive the product verticies)
     */
    void assign_font(const SfBitmapFont &);

    /** Assigns the font of the text from another text object.
     *  @note this has an effect of clearing the text, make sure to assign
     *        the font before doing much else with this object
     *  @warning the given font must survive for the life of the draw text
     *           (and the font's texture must survive the product verticies)
     */
    void assign_font_from(const DrawText &);

    /** Sets the text's string and center position. */
    template <typename T>
    void set_text_center(sf::Vector2f, const BasicString<T> &);

    /** Sets the text's string and top left position. */
    template <typename T>
    void set_text_top_left(sf::Vector2f, const BasicString<T> &);

    /** Sets the text's string and top right position. */
    template <typename T>
    void set_text_top_right(sf::Vector2f, const BasicString<T> &);

    /** Sets the text's string and center position. */
    template <typename T>
    void set_text_center(sf::Vector2f, BsIter<T> beg, BsIter<T> end);

    /** Sets the text's string and top left position. */
    template <typename T>
    void set_text_top_left(sf::Vector2f, BsIter<T> beg, BsIter<T> end);

    /** Sets the text's string and top right position. */
    template <typename T>
    void set_text_top_right(sf::Vector2f, BsIter<T> beg, BsIter<T> end);

    /** Sets the text's string and center position. */
    template <typename T>
    void set_text_center(sf::Vector2f, const T * beg, const T * end);

    /** Sets the text's string and top left position. */
    template <typename T>
    void set_text_top_left(sf::Vector2f, const T * beg, const T * end);

    /** Sets the text's string and top right position. */
    template <typename T>
    void set_text_top_right(sf::Vector2f, const T * beg, const T * end);

    /** Moves the text's position by the given displacement vector. */
    void move(sf::Vector2f);

    /** @returns the size needed to render some given number of characters
     *  @note this text renderer, renders all text on a single line
     */
    Size2<float> measure_text(int character_count) const;

    /** @returns current font pointer, which maybe null.
     *  This function maybe used to retreive your texture, needed to render
     *  verticies.
     */
    const SfBitmapFont * font() const;

    /** @returns all verticies produced by the set_text function family.
     *  @note This function also clears all verticies contained in this object.
     */
    std::vector<sf::Vertex> give_verticies();

    /** @returns a read only reference to the produced verticies */
    const std::vector<sf::Vertex> & verticies() const;

private:
    static constexpr const int k_verticies_per_character = 2*3;
    void draw(sf::RenderTarget &, sf::RenderStates) const final;

    sf::Vector2f push_character(sf::Vector2f, char);

    const SfBitmapFont * m_font = nullptr;
    std::vector<sf::Vertex> m_verticies;
};

// ----------------------------------------------------------------------------

template <typename T>
void DrawText::set_text_center(sf::Vector2f r, const BasicString<T> & bstr)
    { set_text_center<T>(r, bstr.begin(), bstr.end()); }

template <typename T>
void DrawText::set_text_top_left(sf::Vector2f r, const BasicString<T> & bstr)
    { set_text_top_left<T>(r, bstr.begin(), bstr.end()); }

template <typename T>
void DrawText::set_text_top_right(sf::Vector2f r, const BasicString<T> & bstr)
    { set_text_top_right<T>(r, bstr.begin(), bstr.end()); }

template <typename T>
void DrawText::set_text_center(sf::Vector2f r, BsIter<T> beg, BsIter<T> end) {
    r -= convert_to<sf::Vector2f>(measure_text(end - beg))*0.5f;
    set_text_top_left<T>(r, beg, end);
}

template <typename T>
void DrawText::set_text_top_left(sf::Vector2f r, BsIter<T> beg, BsIter<T> end) {
    // make sure we can dereference beg first
    if (beg == end) return;
    set_text_top_left<T>(r, &*beg, &*beg + (end - beg));
}

template <typename T>
void DrawText::set_text_top_right(sf::Vector2f r, BsIter<T> beg, BsIter<T> end) {
    r.x -= measure_text(end - beg).width;
    set_text_top_left<T>(r, beg, end);
}

template <typename T>
void DrawText::set_text_center(sf::Vector2f r, const T * beg, const T * end) {
    r -= convert_to<sf::Vector2f>(measure_text(end - beg))*0.5f;
    set_text_top_left<T>(r, beg, end);
}

template <typename T>
void DrawText::set_text_top_left(sf::Vector2f r, const T * beg, const T * end) {
    if (end < beg) {
        using InvArg = std::invalid_argument;
        // also note: it is not defined behavior to compare two iterators from
        //            different containers
        throw InvArg("DrawText::set_text_top_left: given iterators/pointers "
                     "must describe a valid sequence where begin does not go "
                     "beyond end.");
    }
    m_verticies.reserve((end - beg)*k_verticies_per_character);
    m_verticies.clear();
    for (auto itr = beg; itr != end; ++itr) {
        using CharLims = std::numeric_limits<char>;
        auto c = *itr;
        if (c < CharLims::min() || c > CharLims::max()) {
            c = T(k_substitution);
        }
        r = push_character(r, char(c));
    }
}

template <typename T>
void DrawText::set_text_top_right(sf::Vector2f r, const T * beg, const T * end) {
    r.x -= measure_text(end - beg).width;
    set_text_top_left<T>(r, beg, end);
}

} // end of cul namespace
