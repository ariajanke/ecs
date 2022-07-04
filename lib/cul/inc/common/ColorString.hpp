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

#include <stdint.h>
#include <array>
#include <tuple>

#include <algorithm>
#include <stdexcept>
#include <optional>

#include <cmath>
#include <cassert>

namespace cul {

class ColorStringHelpers;
class ColorStringHelpersTests;

/** A ColorString represents RGBA colors as a '#' prefaced 3, 4, 6, or 8 digit
 *  hex string.
 *
 *  This class has operations which can be split among components, and whole
 *  string operations like changing size and converting to 32bit rgba data.
 *
 *  I found this class useful with WASM stuff.
 *
 *  An aside reflecting on this as a programming exercise: @n
 *  I wanted to give a proper go around with functional programming in C++. Now
 *  there are severe limitations of course. In C++, as we know functions are
 *  not first class citizens. This imposed awful limitations resulting in
 *  sometimes very ugly code. Further limitations imposed by the STL made
 *  imperative programming impossible to avoiding in some cases. It's not all
 *  bad, "constexpr" found use in enforcing a few things. @n
 *  @n
 *  Regardless I'm quite happy with what I accomplished with this immutable
 *  class. All methods should at least be pure.
 *  @n
 *  Also with so much of it being constexpr, it can be tested at compile time!
 *
 *  Using Godbolt's compiler explorer:
 *  Tested on: MSVC v19.20 (x86 and x64), GCC 7.3 x64, clang 5.0.0 x64
 *
 */
class ColorString final {
public:
    template <typename ... Types>
    using Tuple            = std::tuple<Types...>;
    template <typename ArgType>
    using EnableForNumeric = std::enable_if_t<std::is_arithmetic_v<ArgType>, ColorString>;
    using ComponentString  = std::array<char, 3>;
    using FullString       = std::array<char, 10>;
    using Helpers          = ColorStringHelpers;

    /** Strings which do not have an explicit alpha part will have this as its
     *  alpha value.
     */
    static constexpr const uint8_t k_default_alpha = 0xFF;

    enum ComponentName { k_red, k_green, k_blue, k_alpha };

    /** Represents a component in the color string, which may have one, two,
     *  or in the case of alpha component zero characters.
     */
    template <ComponentName kt_component>
    class Component final {
    public:

        /** Makes a new ColorString with this component being portioned
         *  according to the given fraction.
         *
         *  This is computed by: (num * as_u8()) / denom
         *  @warning There are no safe guards to handle overflow.
         */
        template <typename T>
        constexpr EnableForNumeric<T> portion(T num, T denom = T(1)) const;

        /** Makes new ColorString with this component being replaced by
         *  the given string.
         */
        constexpr ColorString replace_with(const char * part_string) const;

        /** Makes new ColorString with this component being replaced by
         *  the given string.
         */
        constexpr ColorString replace_with(const ComponentString & cmpstr) const
            { return replace_with(cmpstr.data()); }

        /** @returns a substring which holds the component
         *
         *  @note on design: a better abstraction is possible, but left out as
         *        a possible future extension
         */
        constexpr ComponentString as_string() const;

        /** @returns component as a raw numeric type */
        constexpr uint8_t as_u8() const;

        /** @returns one, two, or maybe zero if this is an alpha component */
        constexpr int length() const;

    private:
        friend class ColorString;
        constexpr Component(const ColorString * parent):
            m_str(parent) {}

        constexpr const char * begin() const;

        const ColorString * m_str;
    };

    /** The default color for color strings is white, simply "#FFF". */
    constexpr ColorString(): ColorString("#FFF") {}

    /** Constructs a ColorString from a c-string which represents a color.
     *  @throws if an invalid string value is provided
     *  @param cstr must be a '#' preface hexidecimal string, absent of any
     *              excessive characters, it maybe 3, 4, 6, or 8 digits long
     *              (or 4, 5, 7, or 9 characters)
     */
    constexpr explicit ColorString(const char * cstr);

    /** Constructs a ColorString from component strings. It'll favor the most
     *  terse representation which accomodates string lengths (not values!).
     *
     *  @note on favoring string length: an "FF" will be represented as "FF"
     *        regardless that it can be expressed as "F".
     *  @param red_part   must be a hexidecimal digit string (no extra characaters)
     *  @param green_part must be a hexidecimal digit string (no extra characaters)
     *  @param blue_part  must be a hexidecimal digit string (no extra characaters)
     *  @param alpha_part this is the only parameter whose length maybe 0,
     *                    must also be a hexidecimal digit string (no extra
     *                    characaters)
     */
    constexpr ColorString(const char * red_part , const char * green_part,
                          const char * blue_part, const char * alpha_part = "");

    /** Constructs a ColorString per color component, favoring the most terse
     *  representation.
     */
    constexpr ColorString(uint8_t red_part, uint8_t green_part, uint8_t blue_part,
                          uint8_t alpha_part = k_default_alpha);

    /** Assigns to a new c-string representation, much in the same as the
     *  constructor.
     *  @throws if an invalid string is provided
     */
    constexpr ColorString & operator = (const char *);

    /** @returns a 32bit RGBA representation of the string. */
    constexpr uint32_t to_rgba_u32() const;

    /** Creates a new ColorString with a different size.
     *  Acceptable sizes are: 4, 5, 7, 9
     *
     *  @note when the string is shortened, and the component cannot be
     *        expressed as one character, it will be rounded
     *
     *  @param alpha value to be used iff the alpha component was not already
     *               present in the string (for instance size 4 to 5, however
     *               size 5 to 7 or #RGBA to #RRGGBB will cause this argument
     *               to be ignored)
     */
    constexpr ColorString new_length(int, uint8_t alpha = 0xFF) const;

    /** @returns typical string length */
    constexpr int length() const;

    /** Accesses methods for the first, red component */
    constexpr auto red() const { return Component<k_red  >{this}; }

    /** Accesses methods for the second, green component */
    constexpr auto green() const { return Component<k_green>{this}; }

    /** Accesses methods for the third, blue component */
    constexpr auto blue() const { return Component<k_blue >{this}; }

    /** Accesses methods for the fourth, alpha component, which maybe used
     *  even if its length is zero
     */
    constexpr auto alpha() const { return Component<k_alpha>{this}; }

    /** Makes a new ColorString, which is interpolated between this and another
     *  ColorString being according to the given fraction.
     *
     *  While considering each component as "comp". This function follows the
     *  following formula:
     *  New comp =   (num          * (this  comp)) / denom
     *             + ((denom - num)* (other comp)) / denom
     *
     *  @warning There are no safe guards to handle overflow.
     */
    template <typename T>
    constexpr EnableForNumeric<T> interpolate
        (const ColorString &, T num, T denom = T(1)) const;

    /** @returns c-style string, in the same way std::string does */
    constexpr const char * c_str() const noexcept { return m_str.data(); }

    /** @returns pointer (which maybe used as an iterator) to the beginning of
     *  the string
     */
    constexpr const char * begin() const noexcept { return m_str.data(); }

    /** @returns pointer (which maybe used as an iterator) to one past the last
     *  character (much like all STL ranges)
     */
    constexpr const char * end() const noexcept { return begin() + length(); }

    /** compares two strings, defined like std::string's "<" operator */
    constexpr bool operator <  (const ColorString & rhs) const { return compare(rhs) <  0; }

    /** compares two strings, defined like std::string's ">" operator */
    constexpr bool operator >  (const ColorString & rhs) const { return compare(rhs) >  0; }

    /** compares two strings, defined like std::string's "<=" operator */
    constexpr bool operator <= (const ColorString & rhs) const { return compare(rhs) <= 0; }

    /** compares two strings, defined like std::string's ">=" operator */
    constexpr bool operator >= (const ColorString & rhs) const { return compare(rhs) >= 0; }

    /** compares two strings, defined like std::string's "==" operator */
    constexpr bool operator == (const ColorString & rhs) const { return compare(rhs) == 0; }

    /** compares two strings, defined like std::string's "!=" operator */
    constexpr bool operator != (const ColorString & rhs) const { return compare(rhs) != 0; }

    /** compares two strings, defined like std::string's "<" operator */
    constexpr bool operator <  (const char * rhs) const { return compare(rhs) <  0; }

    /** compares two strings, defined like std::string's ">" operator */
    constexpr bool operator >  (const char * rhs) const { return compare(rhs) >  0; }

    /** compares two strings, defined like std::string's "<=" operator */
    constexpr bool operator <= (const char * rhs) const { return compare(rhs) <= 0; }

    /** compares two strings, defined like std::string's ">=" operator */
    constexpr bool operator >= (const char * rhs) const { return compare(rhs) >= 0; }

    /** compares two strings, defined like std::string's "==" operator */
    constexpr bool operator == (const char * rhs) const { return compare(rhs) == 0; }

    /** compares two strings, defined like std::string's "!=" operator */
    constexpr bool operator != (const char * rhs) const { return compare(rhs) != 0; }

private:
    using SizeInt = std::conditional_t<(sizeof(void *) > 4), int, int_fast16_t>;

    static constexpr const char * verify_valid_string
        (const char * caller, const char * value);

    static constexpr const char * verify_valid_portion
        (const char * caller, const char * value);

    static constexpr bool is_valid_hex_char(char c);

    static constexpr uint8_t char_to_val(char c);

    static constexpr FullString init(const char * str);

    static constexpr std::array<int, 4>
        get_and_verify_part_lens
        (const char * rpart, const char * gpart,
         const char * bpart, const char * apart);

    constexpr int compare(const ColorString &) const;

    constexpr int compare(const char *) const;

    ColorString & copy(const ColorString &);

    ColorString & copy(const char *);

    FullString m_str = FullString{};
    SizeInt m_len = 0;
};

// ---------------------------- ColorStringHelpers ----------------------------

/** Defines utilities for ColorString. This interface is not designed for
 *  client use, and maybe subject to adurpt change.
 */
class ColorStringHelpers final {

    static constexpr const auto k_red   = ColorString::k_red  ;
    static constexpr const auto k_green = ColorString::k_green;
    static constexpr const auto k_blue  = ColorString::k_blue ;
    static constexpr const auto k_alpha = ColorString::k_alpha;

    using ComponentName = ColorString::ComponentName;
    using ComponentString = ColorString::ComponentString;
    using FullString = ColorString::FullString;
    using InvArg = std::invalid_argument;
    friend class ColorString;
    friend class ColorStringHelpersTests;

    template <typename ... Types>
    using Tuple = std::tuple<Types...>;

    template <ComponentName>
    friend class ColorString::Component;

    ColorStringHelpers() { /* please don't instantiate this */ }

    static InvArg make_bad_size(const char * caller) noexcept {
        return InvArg(std::string(caller) + ": size of color string must be 4, 5, 7, or 9");
    }

    static constexpr const char * end_of(const char * s)
        { return *s ? end_of(s + 1) : s; }

    template <ComponentName kt_component>
    static constexpr int component_string_offset(int strlen) {
        [[maybe_unused]] constexpr const auto k_no_alpha_msg =
            "component_string_offset: cannot get offset for string size which "
            "does not have an alpha portion.";

        switch (strlen) {
        case 4:
            if constexpr (kt_component == k_alpha) { throw InvArg(k_no_alpha_msg); }
            [[fallthrough]];
        case 5: return 1 + kt_component;
        case 7:
            if constexpr (kt_component == k_alpha) { throw InvArg(k_no_alpha_msg); }
            [[fallthrough]];
        case 9: return 1 + kt_component*2;
        default: throw make_bad_size("component_string_offset");
        }
    }

    template <ComponentName kt_component>
    static constexpr char funnel_small(const char * beg, const char * end) {
        const auto offset = component_string_offset<kt_component>(end - beg);
        switch (end - beg) {
        case 4: case 5: return beg[ offset ];
        case 7: case 9:
            return char(int(beg[ offset ]) + int(beg[ offset + 1 ]) / 2);
        default: throw make_bad_size("funnel_small");
        }
    }

    template <ComponentName kt_component>
    static constexpr ComponentString funnel_large(const char * beg, const char * end) {
        int len = [=] {
            switch (end - beg) {
            case 4: case 5: return 1;
            case 7: case 9: return 2;
            default: throw make_bad_size("funnel_large");
            }
        } ();
        const auto offset = component_string_offset<kt_component>(end - beg);
        return make_large(beg + offset, len);
    }

    static constexpr ComponentString make_large(const char * s, int len) {
        switch (len) {
        case 1 : return { s[0], s[0], '\0' };
        case 2 : return { s[0], s[1], '\0' };
        default: throw make_bad_size("make_large");
        }
    }

    template <ComponentName kt_component>
    static constexpr auto make_small_funnel
        (const char * source_beg, int len,
         const char * part_beg, int part_len)
    {
        return [=] (const ComponentName k_other_comp) {
            if (k_other_comp == kt_component) {
                // pull from part
                if (part_len == 1) return *part_beg;
                return char((int(part_beg[0]) + int(part_beg[1])) / 2);
            }
            switch (k_other_comp) {
            case k_red  : return funnel_small<k_red  >(source_beg, source_beg + len);
            case k_green: return funnel_small<k_green>(source_beg, source_beg + len);
            case k_blue : return funnel_small<k_blue >(source_beg, source_beg + len);
            case k_alpha: return funnel_small<k_alpha>(source_beg, source_beg + len);
            default     : throw "impossible branch";
            }
        };
    }

    template <ComponentName kt_component>
    static constexpr auto make_large_funnel
        (const char * source_beg, int len,
         const char * part_beg, int part_len)
    {
        // there must be a way to reduce this, perhaps to a single template
        // function?
        return [=] (const ComponentName k_other_comp) {
            if (k_other_comp == kt_component) {
                // pull from part
                if (part_len == 1)
                    return ComponentString{ part_beg[0], part_beg[0], '\0'};
                return ComponentString{ part_beg[0], part_beg[1], '\0'};
            }
            switch (k_other_comp) {
            case k_red  : return funnel_large<k_red  >(source_beg, source_beg + len);
            case k_green: return funnel_large<k_green>(source_beg, source_beg + len);
            case k_blue : return funnel_large<k_blue >(source_beg, source_beg + len);
            case k_alpha: return funnel_large<k_alpha>(source_beg, source_beg + len);
            default     : throw "impossible branch";
            }
        };
    }

    template <int idx_0, int idx_1>
    static constexpr auto make_to_u8() {
        const auto char_to_val = [](char c)
            { return std::get<uint8_t>(id_hex_char(c)); };
        return [char_to_val](const char * beg)
            { return (char_to_val(beg[idx_0]) << 4) | char_to_val(beg[idx_1]); };
    }

    template <int len>
    static constexpr auto make_to_part() {
        return [](const char * beg) {
            return std::array
                {len == 0 ? '\0' : beg[0], len == 1 ? '\0' : beg[1], '\0'};
        };
    }

    static constexpr Tuple<bool, uint8_t> id_hex_char(char c) {
        using std::make_tuple;
        if (c >= 'a' && c <= 'f') return make_tuple(true, (c - 'a') + 10);
        if (c >= 'A' && c <= 'F') return make_tuple(true, (c - 'A') + 10);
        if (c >= '0' && c <= '9') return make_tuple(true, c - '0');
        return make_tuple(false, '\0');
    }

    // std::all_of is not constexpr in C++17
    template <typename T, typename Func>
    static constexpr bool all_of(const T * beg, const T * end, Func && f) {
        if (beg == end) throw "";
        if (end - beg == 1) return f(*beg);
        return f(*beg) && all_of(beg + 1, end, std::move(f));
    }

    template <typename T, typename Func>
    static constexpr bool any_of(const T * beg, const T * end, Func && f) {
        if (beg == end) throw "";
        if (end - beg == 1) return f(*beg);
        return f(*beg) || any_of(beg + 1, end, std::move(f));
    }

    static constexpr int len_of(const char * s) { return end_of(s) - s; }

    static constexpr char as_char(int i)
        { return i > 9 ? ( (i - 10) + 'A' ) : (i + '0'); }

    static constexpr ComponentString u8_to_string(uint8_t u8) {
        auto to_a = [](int i) -> char {
            auto c = as_char(i);
            if (!std::get<bool>(id_hex_char(c))) throw "";
            return c;
        };
        return { to_a(u8 / 0x10), to_a(u8 % 0x10), '\0' };
    }

    template <typename T>
    static constexpr uint8_t portion_val
        (const T num, const T denom, const uint8_t comp_val)
    {
        const T val = (num*comp_val) / denom;
        if constexpr (std::is_floating_point_v<T>) {
            return std::round(val);
        }
        return val;
    }

    // done without checks
    static constexpr FullString
        make_with_byte_components
        (const char * rpart, const char * gpart, const char * bpart,
         const char * apart) noexcept
    {
        return { '#',
            rpart[0], rpart[1], gpart[0], gpart[1], bpart[0], bpart[1],
            apart ? apart[0] : '\0',
            apart ? apart[1] : '\0',
            '\0'
        };
    }

    template <typename Func>
    static constexpr auto make_with_byte_components
        (const ComponentString & rpart, const ComponentString & gpart,
         const ComponentString & bpart, bool has_apart, Func && make_apart) noexcept
    {
        ComponentString alval{};
        if (has_apart) alval = make_apart();
        return make_with_byte_components(rpart.data(), gpart.data(), bpart.data(),
                                         has_apart ? alval.data() : nullptr);
    }

    static constexpr FullString
        make_from_components
        (uint8_t red_part, uint8_t green_part, uint8_t blue_part, uint8_t alpha_part)
    {
        const bool alpha_omitable = alpha_part == ColorString::k_default_alpha;
        const auto all_can_be_one_char = [=] {
            auto can_be_one_char = [](uint8_t v) { return v % 0x11 == 0; };
            return    can_be_one_char(red_part ) && can_be_one_char(green_part)
                   && can_be_one_char(blue_part)
                   && (alpha_omitable || can_be_one_char(alpha_part));
        } ();

        if (all_can_be_one_char) {
            const auto as_one_char = [] (uint8_t v) { return as_char(v / 0x11); };
            return FullString {
                '#', as_one_char(red_part), as_one_char(green_part),
                as_one_char(blue_part),
                alpha_omitable ? '\0' : as_one_char(alpha_part),
                '\0', '\0', '\0', '\0'
            };
        } else {
            return make_with_byte_components(
                u8_to_string(red_part), u8_to_string(green_part),
                u8_to_string(blue_part),
                !alpha_omitable, [alpha_part] { return u8_to_string(alpha_part); });
        }
    }

    static constexpr FullString
        make_variable_part_strings
        (const char * rpart, const char * gpart, const char * bpart,
         const char * apart, const std::array<int, 4> & lens) noexcept
    {
        const bool any_long = any_of(lens.data(), lens.data() + lens.size(), [](int i) { return i > 1; });
        if (any_long) {
            return make_with_byte_components(
                make_large(rpart, lens[k_red  ]),
                make_large(gpart, lens[k_green]),
                make_large(bpart, lens[k_blue ]),
                lens[k_alpha],
                [apart, &lens] { return make_large(apart, lens[k_alpha]); });
        }
        return { '#', *rpart, *gpart, *bpart,
                 lens[k_alpha] ? *apart : '\0', '\0' };
    }
};

// --------------------- ColorString::ComponentFunctions ----------------------

template <ColorString::ComponentName kt_component>
constexpr const char * ColorString::Component<kt_component>::begin() const {
    using H = Helpers;
    return    m_str->begin()
           + H::component_string_offset<kt_component>(m_str->length());
}

template <ColorString::ComponentName kt_component>
template <typename T>
constexpr ColorString::EnableForNumeric<T>
    ColorString::Component<kt_component>::portion(T num, T denom) const
{
    const auto replace_with_u8 = [this](uint8_t val)
        { return replace_with(Helpers::u8_to_string(val)); };

    return replace_with_u8(Helpers::portion_val( num, denom, as_u8() ));
}

template <ColorString::ComponentName kt_component>
constexpr ColorString ColorString::Component<kt_component>::replace_with
    (const char * part_string) const
{
    using H = Helpers;
    if (!*part_string) return ColorString{*m_str};

    const auto part_len = H::len_of(ColorString::verify_valid_portion(
        "replace_with", part_string));
    const auto parent_len = m_str->length();
    const bool has_alpha =
        kt_component == k_alpha || parent_len == 5 || parent_len == 9;

    if (part_len == 2 || parent_len > 5) {
        const auto large_funnel = Helpers::make_large_funnel
            <kt_component>(m_str->begin(), parent_len, part_string, part_len);
        return ColorString{Helpers::make_with_byte_components(
                        large_funnel(k_red  ),
                        large_funnel(k_green),
                        large_funnel(k_blue ),
            has_alpha, [&large_funnel] { return large_funnel(k_alpha); }
        ).data()};
    }

    const auto small_funnel = Helpers::make_small_funnel
        <kt_component>(m_str->begin(), parent_len, part_string, part_len);
    const std::array strarr = { '#',
                    small_funnel(k_red  ),
                    small_funnel(k_green),
                    small_funnel(k_blue ),
        has_alpha ? small_funnel(k_alpha) : '\0', '\0' };
    return ColorString{strarr.data()};
}
template <ColorString::ComponentName kt_component>
constexpr ColorString::ComponentString
    ColorString::Component<kt_component>::as_string() const
{
    using H = Helpers;
    switch (length()) {
    case 0: return H::make_to_part<0>()(begin());
    case 1: return H::make_to_part<1>()(begin());
    case 2: return H::make_to_part<2>()(begin());
    default: throw "";
    }
}

template <ColorString::ComponentName kt_component>
constexpr uint8_t ColorString::Component<kt_component>::as_u8() const {
    // MSVC is picky with constexpr switch statements
    const auto v = [this] () -> std::optional<uint8_t> {
        switch (length()) {
        // there must be a fallback for alpha
        case 0:
            if constexpr (kt_component == k_alpha)
                { return k_default_alpha; }
            return {};
        case 1: return Helpers::make_to_u8<0, 0>()(begin());
        case 2: return Helpers::make_to_u8<0, 1>()(begin());
        default: return {};
        }

    } ();
    if (v) { return *v; }
    else throw "";
}

template <ColorString::ComponentName kt_component>
constexpr int ColorString::Component<kt_component>::length() const {
    switch (m_str->length()) {
    case 4:
        if constexpr (kt_component == k_alpha) return 0;
    case 5: return 1;
    case 7:
        if constexpr (kt_component == k_alpha) return 0;
    case 9: return 2;
    default: throw std::runtime_error("");
    }
}

// ------------------------------- ColorString --------------------------------

constexpr ColorString::ColorString(const char * str):
    m_str(init(str)), // init verifies str
    m_len(Helpers::len_of(str))
{}

constexpr ColorString::ColorString
    (const char * rpart, const char * gpart, const char * bpart,
     const char * apart):
     m_str(Helpers::make_variable_part_strings(
        rpart, gpart, bpart, apart,
        get_and_verify_part_lens(rpart, gpart, bpart, apart))),
    m_len(Helpers::len_of(m_str.data()))
{}

constexpr ColorString::ColorString
    (uint8_t red_part, uint8_t green_part, uint8_t blue_part,
     uint8_t alpha_part):
    m_str(Helpers::make_from_components(red_part, green_part, blue_part, alpha_part)),
    m_len(Helpers::len_of(m_str.data()))
{}

constexpr ColorString & ColorString::operator = (const char * str) {
    m_str = init(str);
    m_len = Helpers::len_of(str);
    return *this;
}

constexpr uint32_t ColorString::to_rgba_u32() const {
    return   (uint32_t(red ().as_u8()) << 24) | (uint32_t(green().as_u8()) << 16)
           | (uint32_t(blue().as_u8()) <<  8) | (uint32_t(alpha().as_u8())      );
}

constexpr  ColorString ColorString::new_length(int new_len, uint8_t alpha_) const {
    auto shorten_component = [] (uint8_t i) {
        // if n > 5 / 11 then go up
        const auto val = (i / 0x11) + (i % 0x11 > 5 ? 1 : 0);
        if (val < 0 || val > 0xF) throw "";
        return Helpers::as_char(val);
    };
    auto shorten = [=] (char alpha_slot) {
        return ColorString{(std::array{
            '#',
            shorten_component(red  ().as_u8()),
            shorten_component(green().as_u8()),
            shorten_component(blue ().as_u8()),
            alpha_slot,
            '\0'
        }).data()};
    };
    auto lengthen = [=] (std::optional<uint8_t> alpha_) {
        return ColorString{Helpers::make_with_byte_components(
            Helpers::u8_to_string(red  ().as_u8()).data(),
            Helpers::u8_to_string(green().as_u8()).data(),
            Helpers::u8_to_string(blue ().as_u8()).data(),
            alpha_ ? Helpers::u8_to_string(*alpha_).data() : nullptr ).data()};
    };
    const auto grows_an_alpha =
        (length() == 4 || length() == 7) && (new_len == 5 || new_len == 9);
    switch (new_len) {
    case 4: return shorten('\0');
    case 5: return shorten(shorten_component(grows_an_alpha ? alpha_ : alpha().as_u8()));
    case 7: return lengthen({});
    case 9: return lengthen( grows_an_alpha ? alpha_ : alpha().as_u8() );
    default: throw Helpers::make_bad_size("ColorString");
    }
}

constexpr int ColorString::length() const
    { return m_len; }

template <typename T>
constexpr ColorString::EnableForNumeric<T> ColorString::interpolate
    (const ColorString & other, T num, T denom) const
{
    if (denom < num) {
        throw std::invalid_argument(
            "ColorString::portion_with: denominator must be greater than or "
            "equal to the numerator.");
    }
    auto portion_comp = [num, denom] (uint8_t lhs, uint8_t rhs) -> uint8_t {
        return   Helpers::portion_val(num        , denom, lhs)
               + Helpers::portion_val(denom - num, denom, rhs);
    };
    return ColorString{
        portion_comp(red  ().as_u8(), other.red  ().as_u8()),
        portion_comp(green().as_u8(), other.green().as_u8()),
        portion_comp(blue ().as_u8(), other.blue ().as_u8()),
        portion_comp(alpha().as_u8(), other.alpha().as_u8())};
}

/* private static */ constexpr const char *
    ColorString::verify_valid_string
    (const char * caller, const char * value)
{
    using InvArg = std::invalid_argument;
    const auto * end = Helpers::end_of(value);
    switch (end - value) {
    case 4: case 5: case 7: case 9: break;
    default:
        throw InvArg(std::string(caller) + ": color string must have a size "
                     "of 4, 5, 7, or 9 characters.");
    }
    if (*value != '#') {
        throw InvArg(std::string(caller) + ": color string must be prefaced "
                     "with '#'.");
    }
    if (Helpers::all_of(value + 1, end, is_valid_hex_char)) return value;
    else throw InvArg(std::string(caller) + ": invalid hex characters detected.");
}

/* private static */ constexpr const char * ColorString::verify_valid_portion
    (const char * caller, const char * value)
{
    using InvArg = std::invalid_argument;
    const auto * end = Helpers::end_of(value);
    if (end - value != 1 && end - value != 2) {
        throw InvArg(std::string(caller) + ": portion string must be 1 or 2 characters.");
    }
    if (Helpers::all_of(value, end, is_valid_hex_char)) return value;
    else throw InvArg(std::string(caller) + ": invalid hex characters detected.");
}

/* private static */ constexpr bool ColorString::is_valid_hex_char(char c)
    { return std::get<bool>(Helpers::id_hex_char(c)); }

/* private static */ constexpr uint8_t ColorString::char_to_val(char c) {
    const auto [b, rv] = Helpers::id_hex_char(c);
    if (b) return rv;
    else throw "";
}

/* private static */ constexpr ColorString::FullString ColorString::init
    (const char * str)
{
    auto end = Helpers::end_of(verify_valid_string("copy", str));

    FullString rv{};
    auto otr = rv.begin();
    auto wtr = str;
    while (wtr != end) {
        *otr++ = *wtr++;
    }
    return rv;
}

/* private static */ constexpr std::array<int, 4>
    ColorString::get_and_verify_part_lens
    (const char * rpart, const char * gpart,
     const char * bpart, const char * apart)
{
    using H = Helpers;
    const auto alpha_len = H::len_of(apart);
    if (alpha_len) { verify_valid_portion("ColorString", apart); }
    return std::array {
        H::len_of(verify_valid_portion("ColorString", rpart)),
        H::len_of(verify_valid_portion("ColorString", gpart)),
        H::len_of(verify_valid_portion("ColorString", bpart)),
        alpha_len
    };
}

/* private */ constexpr int ColorString::compare(const ColorString & rhs) const
    { return compare(rhs.c_str()); }

/* private */ constexpr int ColorString::compare(const char * cstr) const {
    struct F final {
    static constexpr int compare(const char * l, const char * r) {
        if (!*l || !*r) return *l - *r;
        if (*l == *r) return compare(l + 1, r + 1);
        return *l - *r;
    }
    };
    return F::compare(c_str(), cstr);
}

inline /* private */ ColorString & ColorString::copy(const ColorString & rhs) {
    return copy(rhs.m_str.data());
}

inline /* private */ ColorString & ColorString::copy(const char * s) {
    std::copy(s, Helpers::end_of(verify_valid_string("copy", s)),
        m_str.begin());
    return *this;
}

// ---------------------------- Compile Time Tests ----------------------------

class ColorStringHelpersTests {
    ColorStringHelpersTests() {}
#   ifdef MACRO_ARIAJANKE_CUL_COLORSTRING_STATIC_ASSERT_TESTS
    static_assert(ColorStringHelpers::len_of("hello") == 5, "");
    static_assert(ColorStringHelpers::component_string_offset<ColorString::k_red>(4) == 1, "");
    static_assert(ColorStringHelpers::make_to_u8<0, 0>()("1") == 0x11, "");
    static_assert(ColorStringHelpers::portion_val(1, 2, 8) == 4, "");
    static_assert(ColorStringHelpers::portion_val(1, 2, 0x88) == 0x44, "");
    static_assert(ColorStringHelpers::portion_val( 1, 2, ColorString{"#888"}.green().as_u8() ) == 0x44, "");
    static_assert([] {
        auto s = "0000010";
        return ColorStringHelpers::any_of(s, s + 6, [](char c) { return c == '1'; });
    }(), "");
#   endif
};

#ifdef MACRO_ARIAJANKE_CUL_COLORSTRING_STATIC_ASSERT_TESTS
// constructors, methods tend to be "tied" together
static_assert(ColorString{} == "#FFF", "");
static_assert(ColorString{}.length() == 4, "");
static_assert(ColorString{"#777"}.length() == 4, "");
static_assert(ColorString{"4", "5", "6"} == "#456", "");
static_assert(ColorString{"99", "55", "33"} == "#995533", "");
static_assert(ColorString{"4", "65", "6"} == "#446566", "");
static_assert(ColorString{"4", "65", "6", "F"} == "#446566FF", "");
static_assert(ColorString{0x11, 0x22, 0x33}       == "#123", "");
static_assert(ColorString{0x11, 0x22, 0x33, 0xFF} == "#123", "");
static_assert(ColorString{0x11, 0x22, 0x33, 0x88} == "#1238", "");
static_assert(ColorString{0x66, 0x23, 0x33}       == "#662333", "");
static_assert(ColorString{0x66, 0x23, 0x33, 0xFF} == "#662333", "");
static_assert(ColorString{0x66, 0x23, 0x33, 0x01} == "#66233301", "");

static_assert((ColorString{} = "#777").length() == 4, "");
static_assert((ColorString{} = ColorString{"#777"}).length() == 4, "");

static_assert(ColorString{"#102058"}.to_rgba_u32() == 0x102058FF, "");

static_assert(ColorString{"#55FF22"}.new_length(4) == "#5F2", "");
static_assert(ColorString{"#55FF22"}.new_length(5, 0x18) == "#5F22", "");
static_assert(ColorString{"#F92"}.new_length(7, 0x55) == "#FF9922", "");
static_assert(ColorString{"#F92"}.new_length(9, 0x55) == "#FF992255", "");

static_assert(ColorString{"#777"}.red().length() == 1, "");
static_assert(   ColorString{"#678"}.red().as_string()[0] == '6'
              && ColorString{"#678"}.red().as_string()[1] == '\0', "");

static_assert(ColorString{"#777"}.red().as_u8() == 0x77, "");
static_assert(ColorString{"#123456"}.red().length() == 2, "");
static_assert(ColorString{"#123456"}.red  ().as_u8() == 0x12, "");
static_assert(ColorString{"#123456"}.green().as_u8() == 0x34, "");
static_assert(ColorString{"#123456"}.blue ().as_u8() == 0x56, "");
static_assert(ColorString{"#123456"}.alpha().as_u8() == 0xFF, "");
static_assert(ColorString{"#888"}.green().replace_with("4").green().as_u8() == 0x44, "");
static_assert(ColorString{"#888"}.green().as_u8() == 0x88, "");
static_assert(ColorString{"#888"}.green().replace_with("4").green().as_u8() == 0x44, "");
static_assert(ColorString{"#888"}.green().portion(1, 2).green().as_u8() == 0x44, "");
static_assert(ColorString{"#AEC"}.green().as_u8() == 0xEE, "");
static_assert(ColorString{"#cbe"}.green().as_u8() == 0xBB, "");

static_assert(ColorString{"#888"} > ColorString{"#777"}, "");
static_assert(ColorString{"#EF9"} == "#EF9", "");
#endif

} // end of cul namespace
