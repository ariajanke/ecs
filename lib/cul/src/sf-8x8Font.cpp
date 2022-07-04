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

#include <cassert>
#include <cstring>

namespace {

const char * check_8x8_char(const char *);

const char * get_8x8_char_impl(char);

} // end of <anonymous> namespace

namespace cul {

namespace detail {

const char * get_8x8_char(char c)
    { return check_8x8_char(get_8x8_char_impl(c)); }

bool is_on_pixel(char c) { return c != ' '; }

} // end of detail namespace -> into cul

} // end of cul namespace

namespace {

const char * get_8x8_char_impl(char c) {
    switch (c) {
    // ------------------------------- Capitals -------------------------------
    case 'A': return
        // 234567
        "        "  // 0
        "   XXX  "  // 1
        "  X   X "  // 2
        "  XXXXX "  // 3
        "  X   X "  // 4
        "  X   X "  // 5
        "        "  // 6
        "        "; // 7
        // 234567
    case 'B': return
        // 234567
        "        "  // 0
        "  XXXX  "  // 1
        "  X   X "  // 2
        "  XXXX  "  // 3
        "  X   X "  // 4
        "  XXXX  "  // 5
        "        "  // 6
        "        "; // 7
        // 234567
    case 'C': return
        // 234567
        "        "  // 0
        "   XXXX "  // 1
        "  X     "  // 2
        "  X     "  // 3
        "  X     "  // 4
        "   XXXX "  // 5
        "        "  // 6
        "        "; // 7
        // 234567
    case 'D': return
        // 234567
        "        "  // 0
        "  XXXX  "  // 1
        "  X   X "  // 2
        "  X   X "  // 3
        "  X   X "  // 4
        "  XXXX  "  // 5
        "        "  // 6
        "        "; // 7
        // 234567
    case 'E': return
        // 234567
        "        "  // 0
        "  XXXXX "  // 1
        "  X     "  // 2
        "  XXXXX "  // 3
        "  X     "  // 4
        "  XXXXX "  // 5
        "        "  // 6
        "        "; // 7
        // 234567
    case 'F': return
        // 234567
        "        "  // 0
        "  XXXXX "  // 1
        "  X     "  // 2
        "  XXXXX "  // 3
        "  X     "  // 4
        "  X     "  // 5
        "        "  // 6
        "        "; // 7
        // 234567
    case 'G': return
        // 234567
        "        "  // 0
        "   XXX  "  // 1
        "  X     "  // 2
        "  X  XX "  // 3
        "  X   X "  // 4
        "   XXX  "  // 5
        "        "  // 6
        "        "; // 7
        // 234567
    case 'H': return
        // 234567
        "        "  // 0
        "  X   X "  // 1
        "  X   X "  // 2
        "  XXXXX "  // 3
        "  X   X "  // 4
        "  X   X "  // 5
        "        "  // 6
        "        "; // 7
        // 234567
    case 'I': return
        // 234567
        "        "  // 0
        "  XXXXX "  // 1
        "    X   "  // 2
        "    X   "  // 3
        "    X   "  // 4
        "  XXXXX "  // 5
        "        "  // 6
        "        "; // 7
        // 234567
    case 'J': return
        // 234567
        "        "  // 0
        "  XXXXX "  // 1
        "     X  "  // 2
        "     X  "  // 3
        "     X  "  // 4
        "  XXXX  "  // 5
        "        "  // 6
        "        "; // 7
        // 234567
    case 'K': return
        // 234567
        "        "  // 0
        "  X  X  "  // 1
        "  X X   "  // 2
        "  XX    "  // 3
        "  X X   "  // 4
        "  X  X  "  // 5
        "        "  // 6
        "        "; // 7
        // 234567
    case 'L': return
        // 234567
        "        "  // 0
        "  X     "  // 1
        "  X     "  // 2
        "  X     "  // 3
        "  X     "  // 4
        "  XXXXX "  // 5
        "        "  // 6
        "        "; // 7
        // 234567
    case 'M': return
        // 234567
        "        "  // 0
        "  X   X "  // 1
        "  XX XX "  // 2
        "  X X X "  // 3
        "  X   X "  // 4
        "  X   X "  // 5
        "        "  // 6
        "        "; // 7
        // 234567
    case 'N': return
        // 234567
        "        "  // 0
        "  X   X "  // 1
        "  XX  X "  // 2
        "  X X X "  // 3
        "  X  XX "  // 4
        "  X   X "  // 5
        "        "  // 6
        "        "; // 7
        // 234567
    case 'O': return
        // 234567
        "        "  // 0
        "   XXX  "  // 1
        "  X   X "  // 2
        "  X   X "  // 3
        "  X   X "  // 4
        "   XXX  "  // 5
        "        "  // 6
        "        "; // 7
        // 234567
    case 'P': return
        // 234567
        "        "  // 0
        "  XXXX  "  // 1
        "  X   X "  // 2
        "  XXXX  "  // 3
        "  X     "  // 4
        "  X     "  // 5
        "        "  // 6
        "        "; // 7
        // 234567
    case 'Q': return
        // 234567
        "        "  // 0
        "   XXX  "  // 1
        "  X   X "  // 2
        "  X X X "  // 3
        "  X  X  "  // 4
        "   XX X "  // 5
        "        "  // 6
        "        "; // 7
        // 234567
    case 'R': return
        // 234567
        "        "  // 0
        "  XXXX  "  // 1
        "  X   X "  // 2
        "  XXXX  "  // 3
        "  X  X  "  // 4
        "  X   X "  // 5
        "        "  // 6
        "        "; // 7
        // 234567
    case 'S': return
        // 234567
        "        "  // 0
        "   XXXX "  // 1
        "  X     "  // 2
        "   XXX  "  // 3
        "      X "  // 4
        "  XXXX  "  // 5
        "        "  // 6
        "        "; // 7
        // 234567
    case 'T': return
        // 234567
        "        "  // 0
        "  XXXXX "  // 1
        "    X   "  // 2
        "    X   "  // 3
        "    X   "  // 4
        "    X   "  // 5
        "        "  // 6
        "        "; // 7
        // 234567
    case 'U': return
        // 234567
        "        "  // 0
        "  X   X "  // 1
        "  X   X "  // 2
        "  X   X "  // 3
        "  X   X "  // 4
        "   XXX  "  // 5
        "        "  // 6
        "        "; // 7
        // 234567
    case 'V': return
        // 234567
        "        "  // 0
        "  X   X "  // 1
        "  X   X "  // 2
        "   X X  "  // 3
        "   X X  "  // 4
        "    X   "  // 5
        "        "  // 6
        "        "; // 7
        // 234567
    case 'W': return
        // 234567
        "        "  // 0
        "  X   X "  // 1
        "  X   X "  // 2
        "  X X X "  // 3
        "  XX XX "  // 4
        "  X   X "  // 5
        "        "  // 6
        "        "; // 7
    case 'X': return
        // 234567
        "        "  // 0
        "  X   X "  // 1
        "   X X  "  // 2
        "    X   "  // 3
        "   X X  "  // 4
        "  X   X "  // 5
        "        "  // 6
        "        "; // 7
    case 'Y': return
        // 234567
        "        "  // 0
        "  X   X "  // 1
        "   X X  "  // 2
        "    X   "  // 3
        "    X   "  // 4
        "    X   "  // 5
        "        "  // 6
        "        "; // 7
    case 'Z': return
        // 234567
        "        "  // 0
        "  XXXXX "  // 1
        "     X  "  // 2
        "    X   "  // 3
        "   X    "  // 4
        "  XXXXX "  // 5
        "        "  // 6
        "        "; // 7
    // ------------------------------ Lowercase -------------------------------
    case 'a': return
        // 234567
        "        "  // 0
        "        "  // 1
        "   XX   "  // 2
        "  X  X  "  // 3
        "  X  X  "  // 4
        "   XX X "  // 5
        "        "  // 6
        "        "; // 7
    case 'b': return
        // 234567
        "        "  // 0
        "  X     "  // 1
        "  X     "  // 2
        "  XXXX  "  // 3
        "  X   X "  // 4
        "  XXXX  "  // 5
        "        "  // 6
        "        "; // 7
    case 'c': return
        // 234567
        "        "  // 0
        "        "  // 1
        "   XXXX "  // 2
        "  X     "  // 3
        "  X     "  // 4
        "   XXXX "  // 5
        "        "  // 6
        "        "; // 7
    case 'd': return
        // 234567
        "        "  // 0
        "      X "  // 1
        "      X "  // 2
        "   XXXX "  // 3
        "  X   X "  // 4
        "   XXXX "  // 5
        "        "  // 6
        "        "; // 7
    case 'e': return
        // 234567
        "        "  // 0
        "   XXX  "  // 1
        "  X   X "  // 2
        "  XXXXX "  // 3
        "  X     "  // 4
        "   XXX  "  // 5
        "        "  // 6
        "        "; // 7
    case 'f': return
        // 234567
        "        "  // 0
        "    XX  "  // 1
        "   X    "  // 2
        "  XXXX  "  // 3
        "   X    "  // 4
        "   X    "  // 5
        "        "  // 6
        "        "; // 7
    case 'g': return
        // 234567
        "        "  // 0
        "        "  // 1
        "   XXX  "  // 2
        "  X   X "  // 3
        "   XXXX "  // 4
        "      X "  // 5
        "   XXX  "  // 6
        "        "; // 7
    case 'h': return
        // 234567
        "        "  // 0
        "  X     "  // 1
        "  X     "  // 2
        "  XXXX  "  // 3
        "  X   X "  // 4
        "  X   X "  // 5
        "        "  // 6
        "        "; // 7
    case 'i': return
        // 234567
        "        "  // 0
        "    X   "  // 1
        "        "  // 2
        "   XX   "  // 3
        "    X   "  // 4
        "    X   "  // 5
        "        "  // 6
        "        "; // 7
    case 'j': return
        // 234567
        "        "  // 0
        "     X  "  // 1
        "        "  // 2
        "    XX  "  // 3
        "     X  "  // 4
        "     X  "  // 5
        "   XX   "  // 6
        "        "; // 7
    case 'k': return
        // 234567
        "        "  // 0
        "  X     "  // 1
        "  X     "  // 2
        "  XXXX  "  // 3
        "  X X   "  // 4
        "  X  X  "  // 5
        "        "  // 6
        "        "; // 7
    case 'l': return
        // 234567
        "        "  // 0
        "   XX   "  // 1
        "    X   "  // 2
        "    X   "  // 3
        "    X   "  // 4
        "    XX  "  // 5
        "        "  // 6
        "        "; // 7
    case 'm': return
        // 234567
        "        "  // 0
        "        "  // 1
        "   X X  "  // 2
        "  X X X "  // 3
        "  X X X "  // 4
        "  X   X "  // 5
        "        "  // 6
        "        "; // 7
    case 'n': return
        // 234567
        "        "  // 0
        "        "  // 1
        "  XXX   "  // 2
        "  X  X  "  // 3
        "  X   X "  // 4
        "  X   X "  // 5
        "        "  // 6
        "        "; // 7
    case 'o': return
        // 234567
        "        "  // 0
        "        "  // 1
        "   XXX  "  // 2
        "  X   X "  // 3
        "  X   X "  // 4
        "   XXX  "  // 5
        "        "  // 6
        "        "; // 7
    case 'p': return
        // 234567
        "        "  // 0
        "        "  // 1
        "   XXX  "  // 2
        "  X   X "  // 3
        "  XXXX  "  // 4
        "  X     "  // 5
        "  X     "  // 6
        "        "; // 7
    case 'q': return
        // 234567
        "        "  // 0
        "        "  // 1
        "   XXX  "  // 2
        "  X   X "  // 3
        "   XXXX "  // 4
        "      X "  // 5
        "      X "  // 6
        "        "; // 7
    case 'r': return
        // 234567
        "        "  // 0
        "        "  // 1
        "  X XXX "  // 2
        "  XX    "  // 3
        "  X     "  // 4
        "  X     "  // 5
        "        "  // 6
        "        "; // 7
    case 's': return
        // 234567
        "        "  // 0
        "        "  // 1
        "   XXX  "  // 2
        "  X     "  // 3
        "   XXX  "  // 4
        "      X "  // 5
        "   XXX  "  // 6
        "        "; // 7
    case 't': return
        // 234567
        "        "  // 0
        "        "  // 1
        "    X   "  // 2
        "   XXX  "  // 3
        "    X   "  // 4
        "    XX  "  // 5
        "        "  // 6
        "        "; // 7
    case 'u': return
        // 234567
        "        "  // 0
        "        "  // 1
        "  X   X "  // 2
        "  X   X "  // 3
        "  X   X "  // 4
        "   XXXX "  // 5
        "        "  // 6
        "        "; // 7
    case 'v': return
        // 234567
        "        "  // 0
        "        "  // 1
        "  X   X "  // 2
        "  X   X "  // 3
        "   X X  "  // 4
        "    X   "  // 5
        "        "  // 6
        "        "; // 7
    case 'w': return
        // 234567
        "        "  // 0
        "        "  // 1
        "  X   X "  // 2
        "  X X X "  // 3
        "  X X X "  // 4
        "   X X  "  // 5
        "        "  // 6
        "        "; // 7
    case 'x': return
        // 234567
        "        "  // 0
        "        "  // 1
        "   X  X "  // 2
        "    XX  "  // 3
        "    XX  "  // 4
        "   X  X "  // 5
        "        "  // 6
        "        "; // 7
    case 'y': return
        // 234567
        "        "  // 0
        "        "  // 1
        "        "  // 2
        "  X   X "  // 3
        "   XXX  "  // 4
        "     X  "  // 5
        "   XX   "  // 6
        "        "; // 7
    case 'z': return
        // 234567
        "        "  // 0
        "        "  // 1
        "  XXXXX "  // 2
        "    XX  "  // 3
        "   XX   "  // 4
        "  XXXXX "  // 5
        "        "  // 6
        "        "; // 7
    // ------------------------ Punctuation / Symbols -------------------------
    case '`': return
        // 234567
        "        "  // 0
        "  XX    "  // 1
        "   X    "  // 2
        "        "  // 3
        "        "  // 4
        "        "  // 5
        "        "  // 6
        "        "; // 7
    case '-': return
        // 234567
        "        "  // 0
        "        "  // 1
        "        "  // 2
        "  XXXXX "  // 3
        "        "  // 4
        "        "  // 5
        "        "  // 6
        "        "; // 7
    case '=': return
        // 234567
        "        "  // 0
        "        "  // 1
        "  XXXXX "  // 2
        "        "  // 3
        "  XXXXX "  // 4
        "        "  // 5
        "        "  // 6
        "        "; // 7
    case '[': return
        // 234567
        "        "  // 0
        "   XX   "  // 1
        "   X    "  // 2
        "   X    "  // 3
        "   X    "  // 4
        "   XX   "  // 5
        "        "  // 6
        "        "; // 7
    case ']': return
        // 234567
        "        "  // 0
        "    XX  "  // 1
        "     X  "  // 2
        "     X  "  // 3
        "     X  "  // 4
        "    XX  "  // 5
        "        "  // 6
        "        "; // 7
    case '\\': return
        // 234567
        "        "  // 0
        "  X     "  // 1
        "   X    "  // 2
        "    X   "  // 3
        "     X  "  // 4
        "      X "  // 5
        "        "  // 6
        "        "; // 7
    case ';': return
        // 234567
        "        "  // 0
        "        "  // 1
        "     X  "  // 2
        "        "  // 3
        "     X  "  // 4
        "    X   "  // 5
        "        "  // 6
        "        "; // 7
    case '\'': return
        // 234567
        "        "  // 0
        "     X  "  // 1
        "     X  "  // 2
        "        "  // 3
        "        "  // 4
        "        "  // 5
        "        "  // 6
        "        "; // 7
    case ',': return
        // 234567
        "        "  // 0
        "        "  // 1
        "        "  // 2
        "        "  // 3
        "        "  // 4
        "     X  "  // 5
        "    X   "  // 6
        "        "; // 7
    case '.': return
        // 234567
        "        "  // 0
        "        "  // 1
        "        "  // 2
        "        "  // 3
        "        "  // 4
        "     X  "  // 5
        "        "  // 6
        "        "; // 7
    case '/': return
        // 234567
        "        "  // 0
        "      X "  // 1
        "     X  "  // 2
        "    X   "  // 3
        "   X    "  // 4
        "  X     "  // 5
        "        "  // 6
        "        "; // 7
    case '~':  return
        // 234567
        "        "  // 0
        "        "  // 1
        "   X    "  // 2
        "  X X X "  // 3
        "     X  "  // 4
        "        "  // 5
        "        "  // 6
        "        "; // 7
    case '_': return
       // 234567
       "        "  // 0
       "        "  // 1
       "        "  // 2
       "        "  // 3
       "        "  // 4
       "  XXXXX "  // 5
       "        "  // 6
       "        "; // 7
    case '+': return
        // 234567
        "        "  // 0
        "    X   "  // 1
        "    X   "  // 2
        "  XXXXX "  // 3
        "    X   "  // 4
        "    X   "  // 5
        "        "  // 6
        "        "; // 7
    case '{': return
        // 234567
        "        "  // 0
        "    XX  "  // 1
        "   X    "  // 2
        "  XX    "  // 3
        "   X    "  // 4
        "    XX  "  // 5
        "        "  // 6
        "        "; // 7
    case '}': return
        // 234567
        "        "  // 0
        "   XX   "  // 1
        "     X  "  // 2
        "     XX "  // 3
        "     X  "  // 4
        "   XX   "  // 5
        "        "  // 6
        "        "; // 7
    case '|': return
        // 234567
        "        "  // 0
        "    X   "  // 1
        "    X   "  // 2
        "    X   "  // 3
        "    X   "  // 4
        "    X   "  // 5
        "    X   "  // 6
        "        "; // 7
    case ':': return
        // 234567
        "        "  // 0
        "    XX  "  // 1
        "    XX  "  // 2
        "        "  // 3
        "    XX  "  // 4
        "    XX  "  // 5
        "        "  // 6
        "        "; // 7
    case '"': return
        // 234567
        "        "  // 0
        "   X X  "  // 1
        "   X X  "  // 2
        "        "  // 3
        "        "  // 4
        "        "  // 5
        "        "  // 6
        "        "; // 7
    case '<': return
        // 234567
        "        "  // 0
        "    X   "  // 1
        "   X    "  // 2
        "  X     "  // 3
        "   X    "  // 4
        "    X   "  // 5
        "        "  // 6
        "        "; // 7
    case '>': return
        // 234567
        "        "  // 0
        "    X   "  // 1
        "     X  "  // 2
        "      X "  // 3
        "     X  "  // 4
        "    X   "  // 5
        "        "  // 6
        "        "; // 7
    case '?': return
        // 234567
        "        "  // 0
        "    XX  "  // 1
        "   X  X "  // 2
        "      X "  // 3
        "    XX  "  // 4
        "        "  // 5
        "    X   "  // 6
        "        "; // 7
    // ------------------------------- Numerics -------------------------------
    case '0': return
        // 234567
        "        "  // 0
        "   XXX  "  // 1
        "  X   X "  // 2
        "  X X X "  // 3
        "  X   X "  // 4
        "   XXX  "  // 5
        "        "  // 6
        "        "; // 7
    case '1': return
        // 234567
        "        "  // 0
        "   XX   "  // 1
        "    X   "  // 2
        "    X   "  // 3
        "    X   "  // 4
        "  xXXXX "  // 5
        "        "  // 6
        "        "; // 7
    case '2': return
        // 234567
        "        "  // 0
        "   XXX  "  // 1
        "      X "  // 2
        "   XXX  "  // 3
        "  X     "  // 4
        "  XXXXX "  // 5
        "        "  // 6
        "        "; // 7
    case '3': return
        // 234567
        "        "  // 0
        "  XXXX  "  // 1
        "      X "  // 2
        "   XXX  "  // 3
        "      X "  // 4
        "  XXXX  "  // 5
        "        "  // 6
        "        "; // 7
    case '4': return
        // 234567
        "        "  // 0
        "  X  X  "  // 1
        "  X  X  "  // 2
        "  XXXXX "  // 3
        "     X  "  // 4
        "     X  "  // 5
        "        "  // 6
        "        "; // 7
    case '5': return
        // 234567
        "        "  // 0
        "  XXXXX "  // 1
        "  X     "  // 2
        "   XXX  "  // 3
        "      X "  // 4
        "   XXX  "  // 5
        "        "  // 6
        "        "; // 7
    case '6':  return
        // 234567
        "        "  // 0
        "   XXX  "  // 1
        "  X     "  // 2
        "  XXXX  "  // 3
        "  X   X "  // 4
        "   XXX  "  // 5
        "        "  // 6
        "        "; // 7
    case '7': return
        // 234567
        "        "  // 0
        "  XXXXX "  // 1
        "      X "  // 2
        "    X   "  // 3
        "    X   "  // 4
        "    X   "  // 5
        "        "  // 6
        "        "; // 7
    case '8': return
        // 234567
        "        "  // 0
        "   XXX  "  // 1
        "  X   X "  // 2
        "   XXX  "  // 3
        "  X   X "  // 4
        "   XXX  "  // 5
        "        "  // 6
        "        "; // 7
    case '9': return
        // 234567
        "        "  // 0
        "   XXX  "  // 1
        "  X   X "  // 2
        "   XXXX "  // 3
        "      X "  // 4
        "   XXX  "  // 5
        "        "  // 6
        "        "; // 7
    // ------------------------- Symbols on Numerics --------------------------
    case ')': return
        // 234567
        "        "  // 0
        "    X   "  // 1
        "     X  "  // 2
        "     X  "  // 3
        "     X  "  // 4
        "    X   "  // 5
        "        "  // 6
        "        "; // 7
    case '!': return
        // 234567
        "        "  // 0
        "    X   "  // 1
        "    X   "  // 2
        "    X   "  // 3
        "    X   "  // 4
        "        "  // 5
        "    X   "  // 6
        "        "; // 7
    case '@': return
        // 234567
        "        "  // 0
        "   XXX  "  // 1
        "  X   X "  // 2
        "  X XXX "  // 3
        "  X     "  // 4
        "   XXX  "  // 5
        "        "  // 6
        "        "; // 7
    case '#': return
        // 234567
        "        "  // 0
        "   X X  "  // 1
        "  XXXXX "  // 2
        "   X X  "  // 3
        "  XXXXX "  // 4
        "   X X  "  // 5
        "        "  // 6
        "        "; // 7
    case '$': return
        // 234567
        "        "  // 0
        "   XXX  "  // 1
        "  X X   "  // 2
        "   XXX  "  // 3
        "    X X "  // 4
        "   XXX  "  // 5
        "    X   "  // 6
        "        "; // 7
    case '%': return
        // 234567
        "        "  // 0
        "  XX  X "  // 1
        "  X  X  "  // 2
        "    X   "  // 3
        "   X  X "  // 4
        "  X  XX "  // 5
        "        "  // 6
        "        "; // 7
    case '^': return
        // 234567
        "        "  // 0
        "    X   "  // 1
        "   X X  "  // 2
        "  X   X "  // 3
        "        "  // 4
        "        "  // 5
        "        "  // 6
        "        "; // 7
    case '&': return
        // 234567
        "        "  // 0
        "   XXXX "  // 1
        "  X X   "  // 2
        "   XXX  "  // 3
        "  X X   "  // 4
        "   XXXX "  // 5
        "    X   "  // 6
        "        "; // 7
    case '*': return
        // 234567
        "        "  // 0
        "        "  // 1
        "   XX   "  // 2
        "   XXX  "  // 3
        "   XXX  "  // 4
        "        "  // 5
        "        "  // 6
        "        "; // 7
    case '(': return
        // 234567
        "        "  // 0
        "    X   "  // 1
        "   X    "  // 2
        "   X    "  // 3
        "   X    "  // 4
        "    X   "  // 5
        "        "  // 6
        "        "; // 7
    // ---------------------------- Non-printables ----------------------------
    case ' ': default:
    //    01234567
    return "        "  // 0
           "        "  // 1
           "        "  // 2
           "        "  // 3
           "        "  // 4
           "        "  // 5
           "        "  // 6
           "        "; // 7
    }
}

const char * check_8x8_char(const char * uc) {
    static constexpr const auto k_font_char_size = 8;
    assert(uc);
    assert(strlen(uc) == k_font_char_size*k_font_char_size);
    return uc;
}

} // end of <anonymous> namespace
