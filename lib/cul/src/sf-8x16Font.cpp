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

const char * verify_8x16(const char *);

const char * get_char_impl(char);

} // end of <anonymous> namespace

namespace cul {

namespace detail {

const char * get_8x16_char(char c) {
    return verify_8x16(get_char_impl(c));
}

} // end of detail namespace -> into cul

} // end of cul namespace

namespace {

const char * verify_8x16(const char * str) {
    assert(strlen(str) == 8*16);
    return str;
}

const char * get_char_impl(char c) {
    // 1 high line
    // 6 mid  line
    // C base line
    // weight on left
    switch (c) {
    // ------------------------------- Capitals -------------------------------
    case 'A': return
        // 01234567
        """        " // 0
        """   XXX  " // 1
        """  XXXXX " // 2
        """ XX    X" // 3
        """ XX    X" // 4
        """ XX    X" // 5
        """ XXXXXXX" // 6
        """ XX    X" // 7
        """ XX    X" // 8
        """ XX    X" // 9
        """ XX    X" // A
        """ XX    X" // B
        """ XX    X" // C
        """        " // D
        """        " // E
        """        ";// F
    case 'B': return
        // 01234567
        """        " // 0
        """ XXXXX  " // 1
        """ XXXXXX " // 2
        """ XX   XX" // 3
        """ XX   XX" // 4
        """ XX   XX" // 5
        """ XXXXXX " // 6
        """ XX   XX" // 7
        """ XX   XX" // 8
        """ XX   XX" // 9
        """ XX   XX" // A
        """ XX   X " // B
        """ XXXXX  " // C
        """        " // D
        """        " // E
        """        ";// F
    case 'C': return
        // 01234567
        """        " // 0
        """   XXXXX" // 1
        """  XXXXXX" // 2
        """ XX     " // 3
        """ XX     " // 4
        """ XX     " // 5
        """ XX     " // 6
        """ XX     " // 7
        """ XX     " // 8
        """ XX     " // 9
        """ XX     " // A
        """  XX    " // B
        """   XXXXX" // C
        """        " // D
        """        " // E
        """        ";// F
    case 'D': return
        // 01234567
        """        " // 0
        """ XXXXX  " // 1
        """ XXXXXX " // 2
        """ XX    X" // 3
        """ XX    X" // 4
        """ XX    X" // 5
        """ XX    X" // 6
        """ XX    X" // 7
        """ XX    X" // 8
        """ XX    X" // 9
        """ XX    X" // A
        """ XX   X " // B
        """ XXXXX  " // C
        """        " // D
        """        " // E
        """        ";// F
    case 'E': return
        // 01234567
        """        " // 0
        """ XXXXXXX" // 1
        """ XXXXXXX" // 2
        """ XX     " // 3
        """ XX     " // 4
        """ XX     " // 5
        """ XXXXXXX" // 6
        """ XX     " // 7
        """ XX     " // 8
        """ XX     " // 9
        """ XX     " // A
        """ XX     " // B
        """ XXXXXXX" // C
        """        " // D
        """        " // E
        """        ";// F
    case 'F': return
        // 01234567
        """        " // 0
        """ XXXXXXX" // 1
        """ XXXXXXX" // 2
        """ XX     " // 3
        """ XX     " // 4
        """ XX     " // 5
        """ XXXXXXX" // 6
        """ XX     " // 7
        """ XX     " // 8
        """ XX     " // 9
        """ XX     " // A
        """ XX     " // B
        """ XX     " // C
        """        " // D
        """        " // E
        """        ";// F
    case 'G': return
        // 01234567
        """        " // 0
        """   XXX  " // 1
        """  XXXXX " // 2
        """ XX    X" // 3
        """ XX     " // 4
        """ XX     " // 5
        """ XX XXXX" // 6
        """ XX    X" // 7
        """ XX    X" // 8
        """ XX    X" // 9
        """ XX    X" // A
        """  XX  X " // B
        """   XXX  " // C
        """        " // D
        """        " // E
        """        ";// F
    case 'H': return
        // 01234567
        """        " // 0
        """ XX    X" // 1
        """ XX    X" // 2
        """ XX    X" // 3
        """ XX    X" // 4
        """ XX    X" // 5
        """ XXXXXXX" // 6
        """ XX    X" // 7
        """ XX    X" // 8
        """ XX    X" // 9
        """ XX    X" // A
        """ XX    X" // B
        """ XX    X" // C
        """        " // D
        """        " // E
        """        ";// F
    case 'I': return
        // 01234567
        """        " // 0
        """ XXXXXXX" // 1
        """   XX   " // 2
        """   XX   " // 3
        """   XX   " // 4
        """   XX   " // 5
        """   XX   " // 6
        """   XX   " // 7
        """   XX   " // 8
        """   XX   " // 9
        """   XX   " // A
        """   XX   " // B
        """ XXXXXXX" // C
        """        " // D
        """        " // E
        """        ";// F
    case 'J': return
        // 01234567
        """        " // 0
        """ XXXXXX " // 1
        """     XX " // 2
        """     XX " // 3
        """     XX " // 4
        """     XX " // 5
        """     XX " // 6
        """     XX " // 7
        """     XX " // 8
        """ X   XX " // 9
        """ XX  XX " // A
        """  XX XX " // B
        """   XXX  " // C
        """        " // D
        """        " // E
        """        ";// F
    case 'K': return
        // 01234567
        """        " // 0
        """ XX    X" // 1
        """ XX   X " // 2
        """ XX  X  " // 3
        """ XX X   " // 4
        """ XXX    " // 5
        """ XXXX   " // 6
        """ XX  X  " // 7
        """ XX  X  " // 8
        """ XX   X " // 9
        """ XX   X " // A
        """ XX    X" // B
        """ XX    X" // C
        """        " // D
        """        " // E
        """        ";// F
    case 'L': return
        // 01234567
        """        " // 0
        """ XX     " // 1
        """ XX     " // 2
        """ XX     " // 3
        """ XX     " // 4
        """ XX     " // 5
        """ XX     " // 6
        """ XX     " // 7
        """ XX     " // 8
        """ XX     " // 9
        """ XX     " // A
        """ XX     " // B
        """ XXXXXXX" // C
        """        " // D
        """        " // E
        """        ";// F
        // 234567
    case 'M': return
        // 01234567
        """        " // 0
        """ XX    X" // 1
        """ XXX  XX" // 2
        """ XXX  XX" // 3
        """ XX XXXX" // 4
        """ XX XX X" // 5
        """ XX XX X" // 6
        """ XX    X" // 7
        """ XX    X" // 8
        """ XX    X" // 9
        """ XX    X" // A
        """ XX    X" // B
        """ XX    X" // C
        """        " // D
        """        " // E
        """        ";// F
    case 'N': return
        // 01234567
        """        " // 0
        """ XXX   X" // 1
        """ XXX   X" // 2
        """ XX X  X" // 3
        """ XX X  X" // 4
        """ XX X  X" // 5
        """ XX X  X" // 6
        """ XX X  X" // 7
        """ XX  X X" // 8
        """ XX  X X" // 9
        """ XX  X X" // A
        """ XX   XX" // B
        """ XX   XX" // C
        """        " // D
        """        " // E
        """        ";// F
        // 234567
    case 'O': return
        // 01234567
        """        " // 0
        """  XXXXX " // 1
        """ XXX  XX" // 2
        """ XX    X" // 3
        """ XX    X" // 4
        """ XX    X" // 5
        """ XX    X" // 6
        """ XX    X" // 7
        """ XX    X" // 8
        """ XX    X" // 9
        """ XX    X" // A
        """ XXX  XX" // B
        """  XXXXX " // C
        """        " // D
        """        " // E
        """        ";// F
        // 234567
    case 'P': return
        // 01234567
        """        " // 0
        """ XXXXXX " // 1
        """ XXXXXXX" // 2
        """ XX   XX" // 3
        """ XX    X" // 4
        """ XX    X" // 5
        """ XXXXXX " // 6
        """ XX     " // 7
        """ XX     " // 8
        """ XX     " // 9
        """ XX     " // A
        """ XX     " // B
        """ XX     " // C
        """        " // D
        """        " // E
        """        ";// F
    case 'Q': return
        // 01234567
        """        " // 0
        """   XXX  " // 1
        """  XXXXX " // 2
        """ XX    X" // 3
        """ XX    X" // 4
        """ XX    X" // 5
        """ XX    X" // 6
        """ XX    X" // 7
        """ XX    X" // 8
        """ XX X  X" // 9
        """ XX  X X" // A
        """  X   X " // B
        """   XXX X" // C
        """        " // D
        """        " // E
        """        ";// F
    case 'R': return
        // 01234567
        """        " // 0
        """ XXXXX  " // 1
        """ XXXXXX " // 2
        """ XX    X" // 3
        """ XX    X" // 4
        """ XX    X" // 5
        """ XX  XX " // 6
        """ XXXX   " // 7
        """ XXX    " // 8
        """ XX X   " // 9
        """ XX  X  " // A
        """ XX   X " // B
        """ XX    X" // C
        """        " // D
        """        " // E
        """        ";// F
    case 'S': return
        // 01234567
        """        " // 0
        """   XXXXX" // 1
        """  XXXXX " // 2
        """ XXX    " // 3
        """ XX     " // 4
        """ XX     " // 5
        """ XXXXXX " // 6
        """  XXXXXX" // 7
        """       X" // 8
        """       X" // 9
        """       X" // A
        """      X " // B
        """ XXXXX  " // C
        """        " // D
        """        " // E
        """        ";// F
    case 'T': return
        // 01234567
        """        " // 0
        """ XXXXXXX" // 1
        """ XX X   " // 2
        """    X   " // 3
        """    X   " // 4
        """    X   " // 5
        """    X   " // 6
        """    X   " // 7
        """    X   " // 8
        """    X   " // 9
        """    X   " // A
        """    X   " // B
        """    X   " // C
        """        " // D
        """        " // E
        """        ";// F
    case 'U': return
        // 01234567
        """        " // 0
        """ XX    X" // 1
        """ XX    X" // 2
        """ XX    X" // 3
        """ XX    X" // 4
        """ XX    X" // 5
        """ XX    X" // 6
        """ XX    X" // 7
        """ XX    X" // 8
        """ XX    X" // 9
        """ XX    X" // A
        """  XX  X " // B
        """   XXX  " // C
        """        " // D
        """        " // E
        """        ";// F
    case 'V': return
        // 01234567
        """        " // 0
        """ XX    X" // 1
        """ XX    X" // 2
        """ XX    X" // 3
        """ XX    X" // 4
        """ XX    X" // 5
        """ XX    X" // 6
        """  XX  X " // 7
        """  XX  X " // 8
        """   XXX  " // 9
        """   XXX  " // A
        """    X   " // B
        """    X   " // C
        """        " // D
        """        " // E
        """        ";// F
    case 'W': return
        // 01234567
        """        " // 0
        """        " // 1
        """ X     X" // 2
        """ X     X" // 3
        """ X     X" // 4
        """ X     X" // 5
        """ X  X  X" // 6
        """ X  X  X" // 7
        """ X  X  X" // 8
        """ X XXX X" // 9
        """ XXXXXXX" // A
        """ XX XX X" // B
        """ X  X  X" // C
        """        " // D
        """        " // E
        """        ";// F
    case 'X': return
        // 01234567
        """        " // 0
        """ XX    X" // 1
        """ XX    X" // 2
        """ XX   X " // 3
        """  XX  X " // 4
        """   X X  " // 5
        """    X   " // 6
        """   X X  " // 7
        """  XX  X " // 8
        """  XX  X " // 9
        """ XX    X" // A
        """ XX    X" // B
        """ XX    X" // C
        """        " // D
        """        " // E
        """        ";// F
    case 'Y': return
        // 01234567
        """        " // 0
        """ XX    X" // 1
        """ XX    X" // 2
        """ XX    X" // 3
        """  XX  X " // 4
        """   X X  " // 5
        """    X   " // 6
        """    X   " // 7
        """    X   " // 8
        """    X   " // 9
        """    X   " // A
        """    X   " // B
        """    XX  " // C
        """        " // D
        """        " // E
        """        ";// F
    case 'Z': return
        // 01234567
        """        " // 0
        """ XXXXXXX" // 1
        """ XX   XX" // 2
        """       X" // 3
        """      X " // 4
        """     X  " // 5
        """    X   " // 6
        """   X    " // 7
        """   X    " // 8
        """  X     " // 9
        """  X     " // A
        """ X     X" // B
        """ XXXXXXX" // C
        """        " // D
        """        " // E
        """        ";// F
    // ------------------------------ Lowercase -------------------------------
    case 'a': return
        // 01234567
        """        " // 0
        """        " // 1
        """        " // 2
        """        " // 3
        """        " // 4
        """        " // 5
        """   XXXX " // 6
        """ XX    X" // 7
        """ XX    X" // 8
        """ XX    X" // 9
        """ XX   XX" // A
        """ XX  X X" // B
        """  XXX  X" // C
        """        " // D
        """        " // E
        """        ";// F
    case 'b': return
        // 01234567
        """        " // 0
        """        " // 1
        """ X      " // 2
        """ X      " // 3
        """ X      " // 4
        """ X      " // 5
        """ XXXXXX " // 6
        """ XX    X" // 7
        """ XX    X" // 8
        """ XX    X" // 9
        """ XX    X" // A
        """ XX    X" // B
        """ X XXXX " // C
        """        " // D
        """        " // E
        """        ";// F
    case 'c': return
        // 01234567
        """        " // 0
        """        " // 1
        """        " // 2
        """        " // 3
        """        " // 4
        """        " // 5
        """   XXXX " // 6
        """  XX    " // 7
        """ XX     " // 8
        """ XX     " // 9
        """ XX     " // A
        """  X     " // B
        """   XXXX " // C
        """        " // D
        """        " // E
        """        ";// F
    case 'd': return
        // 01234567
        """        " // 0
        """        " // 1
        """       X" // 2
        """       X" // 3
        """       X" // 4
        """       X" // 5
        """   XXXXX" // 6
        """ XX    X" // 7
        """ XX    X" // 8
        """ XX    X" // 9
        """ XX    X" // A
        """ XX   XX" // B
        """  XXXX X" // C
        """        " // D
        """        " // E
        """        ";// F
    case 'e': return
        // 01234567
        """        " // 0
        """        " // 1
        """        " // 2
        """        " // 3
        """        " // 4
        """        " // 5
        """   XXXX " // 6
        """ XX    X" // 7
        """ XX    X" // 8
        """ XXXXXXX" // 9
        """ XX     " // A
        """ XX     " // B
        """  XXXXX " // C
        """        " // D
        """        " // E
        """        ";// F
    case 'f': return
        // 01234567
        """        " // 0
        """        " // 1
        """        " // 2
        """     XX " // 3
        """    X   " // 4
        """    X   " // 5
        """  XXXXX " // 6
        """    X   " // 7
        """    X   " // 8
        """    X   " // 9
        """    X   " // A
        """    X   " // B
        """    X   " // C
        """        " // D
        """        " // E
        """        ";// F
    case 'g': return
        // 01234567
        """        " // 0
        """        " // 1
        """        " // 2
        """        " // 3
        """        " // 4
        """        " // 5
        """  XXXXX " // 6
        """ XX   XX" // 7
        """ X    XX" // 8
        """ X     X" // 9
        """ X     X" // A
        """ XX    X" // B
        """  XXXXXX" // C
        """      XX" // D
        """ X     X" // E
        """  XXXXX ";// F
    case 'h': return
        // 01234567
        """        " // 0
        """ XX     " // 1
        """ XX     " // 2
        """ XX     " // 3
        """ XX     " // 4
        """ XX     " // 5
        """ XXXXXX " // 6
        """ XX    X" // 7
        """ XX    X" // 8
        """ XX    X" // 9
        """ XX    X" // A
        """ XX    X" // B
        """ XX    X" // C
        """        " // D
        """        " // E
        """        ";// F
    case 'i': return
        // 01234567
        """        " // 0
        """        " // 1
        """        " // 2
        """   XX   " // 3
        """   XX   " // 4
        """        " // 5
        """  XXX   " // 6
        """  XXX   " // 7
        """    X   " // 8
        """    X   " // 9
        """    X   " // A
        """    X   " // B
        """    X   " // C
        """        " // D
        """        " // E
        """        ";// F
    case 'j': return
        // 01234567
        """        " // 0
        """        " // 1
        """        " // 2
        """   XX   " // 3
        """   XX   " // 4
        """        " // 5
        """  XXX   " // 6
        """  XXX   " // 7
        """    X   " // 8
        """    X   " // 9
        """    X   " // A
        """    X   " // B
        """    X   " // C
        """    X   " // D
        """    X   " // E
        """  XX    ";// F
    case 'k': return
        // 01234567
        """        " // 0
        """        " // 1
        """  XX    " // 2
        """  XX    " // 3
        """  XX    " // 4
        """  XX    " // 5
        """  XXXXXX" // 6
        """  XXX   " // 7
        """  XX X  " // 8
        """  XX  X " // 9
        """  XX  X " // A
        """  XX   X" // B
        """  XX   X" // C
        """        " // D
        """        " // E
        """        ";// F
    case 'l': return
        // 01234567
        """        " // 0
        """        " // 1
        """ XX     " // 2
        """  X     " // 3
        """  X     " // 4
        """  X     " // 5
        """  X     " // 6
        """  X     " // 7
        """  X     " // 8
        """  X     " // 9
        """  X     " // A
        """  XX    " // B
        """   XX   " // C
        """        " // D
        """        " // E
        """        ";// F
    case 'm': return
        // 01234567
        """        " // 0
        """        " // 1
        """        " // 2
        """        " // 3
        """        " // 4
        """        " // 5
        """ X XX X " // 6
        """ XX  X X" // 7
        """ XX  X X" // 8
        """ XX  X X" // 9
        """ XX  X X" // A
        """ XX  X X" // B
        """ XX  X X" // C
        """        " // D
        """        " // E
        """        ";// F
    case 'n': return
        // 01234567
        """        " // 0
        """        " // 1
        """        " // 2
        """        " // 3
        """        " // 4
        """        " // 5
        """ X XXXX " // 6
        """ XX    X" // 7
        """ XX    X" // 8
        """ XX    X" // 9
        """ XX    X" // A
        """ XX    X" // B
        """ XX    X" // C
        """        " // D
        """        " // E
        """        ";// F
    case 'o': return
        // 01234567
        """        " // 0
        """        " // 1
        """        " // 2
        """        " // 3
        """        " // 4
        """        " // 5
        """  XXXXX " // 6
        """ XX    X" // 7
        """ XX    X" // 8
        """ XX    X" // 9
        """ XX    X" // A
        """ XX    X" // B
        """  XXXXX " // C
        """        " // D
        """        " // E
        """        ";// F
    case 'p': return
        // 01234567
        """        " // 0
        """        " // 1
        """        " // 2
        """        " // 3
        """        " // 4
        """        " // 5
        """ X XXXX " // 6
        """ XX    X" // 7
        """ XX    X" // 8
        """ XX    X" // 9
        """ XX    X" // A
        """ XX    X" // B
        """ XXXXXX " // C
        """ XX     " // D
        """ XX     " // E
        """ XX     ";// F
    case 'q': return
        // 01234567
        """        " // 0
        """        " // 1
        """        " // 2
        """        " // 3
        """        " // 4
        """        " // 5
        """  XXXX X" // 6
        """ XX   XX" // 7
        """ XX    X" // 8
        """ XX    X" // 9
        """ XX    X" // A
        """ XX    X" // B
        """  XXXXXX" // C
        """       X" // D
        """       X" // E
        """      XX";// F
    case 'r': return
        // 01234567
        """        " // 0
        """        " // 1
        """        " // 2
        """        " // 3
        """        " // 4
        """        " // 5
        """ X  XXX " // 6
        """ X X   X" // 7
        """ XX     " // 8
        """ XX     " // 9
        """ XX     " // A
        """ XX     " // B
        """ XX     " // C
        """        " // D
        """        " // E
        """        ";// F
    case 's': return
        // 01234567
        """        " // 0
        """        " // 1
        """        " // 2
        """        " // 3
        """        " // 4
        """        " // 5
        """  XXXXX " // 6
        """ XX    X" // 7
        """ XX     " // 8
        """  XXXXX " // 9
        """       X" // A
        """  X    X" // B
        """   XXXX " // C
        """        " // D
        """        " // E
        """        ";// F
    case 't': return
        // 01234567
        """        " // 0
        """        " // 1
        """        " // 2
        """        " // 3
        """    X   " // 4
        """    X   " // 5
        """ XXXXXXX" // 6
        """    X   " // 7
        """    X   " // 8
        """    X   " // 9
        """    X   " // A
        """    X   " // B
        """     XX " // C
        """        " // D
        """        " // E
        """        ";// F
    case 'u': return
        // 01234567
        """        " // 0
        """        " // 1
        """        " // 2
        """        " // 3
        """        " // 4
        """        " // 5
        """ XX    X" // 6
        """ XX    X" // 7
        """ XX    X" // 8
        """ XX    X" // 9
        """ XX    X" // A
        """  XX  XX" // B
        """   XXX X" // C
        """        " // D
        """        " // E
        """        ";// F
    case 'v': return
        // 01234567
        """        " // 0
        """        " // 1
        """        " // 2
        """        " // 3
        """        " // 4
        """        " // 5
        """ XX    X" // 6
        """ XX    X" // 7
        """  XX  X " // 8
        """  XX  X " // 9
        """   XXX  " // A
        """   XXX  " // B
        """    X   " // C
        """        " // D
        """        " // E
        """        ";// F
    case 'w': return
        // 01234567
        """        " // 0
        """        " // 1
        """        " // 2
        """        " // 3
        """        " // 4
        """        " // 5
        """ XX X  X" // 6
        """ XX X  X" // 7
        """ XX X  X" // 8
        """ XX X  X" // 9
        """ XX X  X" // A
        """ XXX XX " // B
        """  X  XX " // C
        """        " // D
        """        " // E
        """        ";// F
    case 'x': return
        // 01234567
        """        " // 0
        """        " // 1
        """        " // 2
        """        " // 3
        """        " // 4
        """        " // 5
        """ X     X" // 6
        """ XX   X " // 7
        """  XX X  " // 8
        """   XX   " // 9
        """  XX X  " // A
        """ XX   X " // B
        """ X     X" // C
        """        " // D
        """        " // E
        """        ";// F
    case 'y': return
        // 01234567
        """        " // 0
        """        " // 1
        """        " // 2
        """        " // 3
        """        " // 4
        """        " // 5
        """ X     X" // 6
        """ X     X" // 7
        """ X     X" // 8
        """  X   X " // 9
        """   X X  " // A
        """   X X  " // B
        """    X   " // C
        """    X   " // D
        """    X   " // E
        """  XX    ";// F
    case 'z': return
        // 01234567
        """        " // 0
        """        " // 1
        """        " // 2
        """        " // 3
        """        " // 4
        """        " // 5
        """ XXXXXXX" // 6
        """       X" // 7
        """     XX " // 8
        """    X   " // 9
        """  XX    " // A
        """ X      " // B
        """ XXXXXXX" // C
        """        " // D
        """        " // E
        """        ";// F
    // ------------------------ Punctuation / Symbols -------------------------
    case '`': return
        // 01234567
        """        " // 0
        """ XX     " // 1
        """ XXX    " // 2
        """  XXX   " // 3
        """   XX   " // 4
        """    X   " // 5
        """        " // 6
        """        " // 7
        """        " // 8
        """        " // 9
        """        " // A
        """        " // B
        """        " // C
        """        " // D
        """        " // E
        """        ";// F
    case '-': return
        // 01234567
        """        " // 0
        """        " // 1
        """        " // 2
        """        " // 3
        """        " // 4
        """        " // 5
        """ XXXXXXX" // 6
        """ XXXXXXX" // 7
        """        " // 8
        """        " // 9
        """        " // A
        """        " // B
        """        " // C
        """        " // D
        """        " // E
        """        ";// F
    case '=': return
        // 01234567
        """        " // 0
        """        " // 1
        """        " // 2
        """        " // 3
        """        " // 4
        """        " // 5
        """ XXXXXXX" // 6
        """ XXXXXXX" // 7
        """        " // 8
        """ XXXXXXX" // 9
        """ XXXXXXX" // A
        """        " // B
        """        " // C
        """        " // D
        """        " // E
        """        ";// F
    case '[': return
        // 01234567
        """        " // 0
        """ XXXXX  " // 1
        """ XXXXX  " // 2
        """ XX     " // 3
        """ XX     " // 4
        """ XX     " // 5
        """ XX     " // 6
        """ XX     " // 7
        """ XX     " // 8
        """ XX     " // 9
        """ XX     " // A
        """ XX     " // B
        """ XXXXX  " // C
        """        " // D
        """        " // E
        """        ";// F
    case ']': return
        // 01234567
        """        " // 0
        """   XXXXX" // 1
        """   XXXXX" // 2
        """       X" // 3
        """       X" // 4
        """       X" // 5
        """       X" // 6
        """       X" // 7
        """       X" // 8
        """       X" // 9
        """       X" // A
        """       X" // B
        """   XXXXX" // C
        """        " // D
        """        " // E
        """        ";// F
    case '\\': return
        // 01234567
        """        " // 0
        """ XX     " // 1
        """ XX     " // 2
        """ XXX    " // 3
        """  XXX   " // 4
        """   XX   " // 5
        """   XXX  " // 6
        """    XX  " // 7
        """    XXX " // 8
        """     XX " // 9
        """     XXX" // A
        """      XX" // B
        """      XX" // C
        """        " // D
        """        " // E
        """        ";// F
    case ';': return
        // 01234567
        """        " // 0
        """        " // 1
        """        " // 2
        """        " // 3
        """        " // 4
        """        " // 5
        """    XX  " // 6
        """    XX  " // 7
        """    XX  " // 8
        """        " // 9
        """        " // A
        """    XX  " // B
        """    XX  " // C
        """     X  " // D
        """    X   " // E
        """        ";// F
    case '\'': return
        // 01234567
        """        " // 0
        """    XX  " // 1
        """    XX  " // 2
        """    XX  " // 3
        """    XX  " // 4
        """     X  " // 5
        """        " // 6
        """        " // 7
        """        " // 8
        """        " // 9
        """        " // A
        """        " // B
        """        " // C
        """        " // D
        """        " // E
        """        ";// F
    case ',': return
        // 01234567
        """        " // 0
        """        " // 1
        """        " // 2
        """        " // 3
        """        " // 4
        """        " // 5
        """        " // 6
        """        " // 7
        """        " // 8
        """        " // 9
        """        " // A
        """    XX  " // B
        """    XX  " // C
        """     X  " // D
        """    X   " // E
        """        ";// F
    case '.': return
        // 01234567
        """        " // 0
        """        " // 1
        """        " // 2
        """        " // 3
        """        " // 4
        """        " // 5
        """        " // 6
        """        " // 7
        """        " // 8
        """        " // 9
        """        " // A
        """  XX    " // B
        """  XX    " // C
        """        " // D
        """        " // E
        """        ";// F
    case '/': return
        // 01234567
        """        " // 0
        """      XX" // 1
        """      XX" // 2
        """     XXX" // 3
        """    XXX " // 4
        """    XX  " // 5
        """   XXX  " // 6
        """   XX   " // 7
        """  XXX   " // 8
        """  XX    " // 9
        """ XXX    " // A
        """ XX     " // B
        """ XX     " // C
        """        " // D
        """        " // E
        """        ";// F
    case '~':  return
        // 01234567
        """        " // 0
        """        " // 1
        """        " // 2
        """        " // 3
        """        " // 4
        """  XX    " // 5
        """ XX X XX" // 6
        """     XX " // 7
        """        " // 8
        """        " // 9
        """        " // A
        """        " // B
        """        " // C
        """        " // D
        """        " // E
        """        ";// F
    case '_': return
        // 01234567
        """        " // 0
        """        " // 1
        """        " // 2
        """        " // 3
        """        " // 4
        """        " // 5
        """        " // 6
        """        " // 7
        """        " // 8
        """        " // 9
        """        " // A
        """ XXXXXXX" // B
        """ XXXXXXX" // C
        """        " // D
        """        " // E
        """        ";// F
    case '+': return
        // 01234567
        """        " // 0
        """        " // 1
        """        " // 2
        """        " // 3
        """   XX   " // 4
        """   XX   " // 5
        """ XXXXXX " // 6
        """ XXXXXX " // 7
        """   XX   " // 8
        """   XX   " // 9
        """        " // A
        """        " // B
        """        " // C
        """        " // D
        """        " // E
        """        ";// F
    case '{': return
        // 01234567
        """        " // 0
        """   XXX  " // 1
        """  XXX   " // 2
        """  XX    " // 3
        """  XX    " // 4
        """  XX    " // 5
        """ XX     " // 6
        """  XX    " // 7
        """  XX    " // 8
        """  XX    " // 9
        """  XX    " // A
        """  XXX   " // B
        """   XXX  " // C
        """        " // D
        """        " // E
        """        ";// F
    case '}': return
        // 01234567
        """        " // 0
        """  XXX   " // 1
        """   XXX  " // 2
        """    XX  " // 3
        """    XX  " // 4
        """    XX  " // 5
        """     XX " // 6
        """    XX  " // 7
        """    XX  " // 8
        """    XX  " // 9
        """   XXX  " // A
        """  XXX   " // B
        """        " // C
        """        " // D
        """        " // E
        """        ";// F
    case '|': return
        // 01234567
        """    XX  " // 0
        """    XX  " // 1
        """    XX  " // 2
        """    XX  " // 3
        """    XX  " // 4
        """    XX  " // 5
        """    XX  " // 6
        """    XX  " // 7
        """    XX  " // 8
        """    XX  " // 9
        """    XX  " // A
        """    XX  " // B
        """    XX  " // C
        """        " // D
        """        " // E
        """        ";// F
    case ':': return
        // 01234567
        """        " // 0
        """        " // 1
        """        " // 2
        """    XX  " // 3
        """    XX  " // 4
        """        " // 5
        """        " // 6
        """        " // 7
        """        " // 8
        """    XX  " // 9
        """    XX  " // A
        """        " // B
        """        " // C
        """        " // D
        """        " // E
        """        ";// F
    case '"': return
        // 01234567
        """        " // 0
        """  XX  XX" // 1
        """  XX  XX" // 2
        """  XX  XX" // 3
        """  XX  XX" // 4
        """   X   X" // 5
        """        " // 6
        """        " // 7
        """        " // 8
        """        " // 9
        """        " // A
        """        " // B
        """        " // C
        """        " // D
        """        " // E
        """        ";// F
    case '<': return
        // 01234567
        """        " // 0
        """        " // 1
        """     XX " // 2
        """    XX  " // 3
        """   XX   " // 4
        """  XX    " // 5
        """ XX     " // 6
        """  XX    " // 7
        """   XX   " // 8
        """    XX  " // 9
        """     XX " // A
        """        " // B
        """        " // C
        """        " // D
        """        " // E
        """        ";// F
    case '>': return
        // 01234567
        """        " // 0
        """        " // 1
        """ XX     " // 2
        """  XX    " // 3
        """   XX   " // 4
        """    XX  " // 5
        """     XX " // 6
        """    XX  " // 7
        """   XX   " // 8
        """  XX    " // 9
        """ XX     " // A
        """        " // B
        """        " // C
        """        " // D
        """        " // E
        """        ";// F
    case '?': return
        // 01234567
        """        " // 0
        """   XXX  " // 1
        """  XXXXX " // 2
        """ X    XX" // 3
        """      XX" // 4
        """      XX" // 5
        """     XX " // 6
        """    XX  " // 7
        """    XX  " // 8
        """    XX  " // 9
        """        " // A
        """    XX  " // B
        """    XX  " // C
        """        " // D
        """        " // E
        """        ";// F
    // ------------------------------- Numerics -------------------------------
    case '0': return
        // 01234567
        """        " // 0
        """   XXX  " // 1
        """  XXXXX " // 2
        """ XX   XX" // 3
        """ XX   XX" // 4
        """ XX   XX" // 5
        """ XX   XX" // 6
        """ XX   XX" // 7
        """ XX   XX" // 8
        """ XX   XX" // 9
        """ XX   XX" // A
        """  XXXXX " // B
        """   XXX  " // C
        """        " // D
        """        " // E
        """        ";// F
    case '1': return
        // 01234567
        """        "
        """    XX  "
        """   XXX  "
        """  XX X  "
        """     X  "
        """    XX  "
        """    XX  "
        """    XX  "
        """    XX  "
        """    XX  "
        """    XX  "
        """   XXXX "
        """ XXXXXXX"
        """        "
        """        "
        """        ";
    case '2': return
        // 01234567
        """        "
        """  XXXX  "
        """ XX  XX "
        """ X    XX"
        """      XX"
        """    XXXX"
        """   XXXX "
        """ XXX    "
        """ XX     "
        """ XX     "
        """ XX     "
        """ XXXXX  "
        """  XXXXX "
        """        "
        """        "
        """        ";
    case '3': return
        // 01234567
        """        "
        """  XXXX  "
        """ XX  XX "
        """ X    XX"
        """      XX"
        """      XX"
        """  XXXXXX"
        """ XXXXXX "
        """     XXX"
        """      XX"
        """ X    XX"
        """ XX  XX "
        """  XXXX  "
        """        "
        """        "
        """        ";
    case '4': return
        // 01234567
        """        "
        """ XX   XX"
        """ XX   XX"
        """ XX   XX"
        """ XX   XX"
        """ XX   XX"
        """  XXXXX "
        """   XXXX "
        """     XX "
        """     XX "
        """     XX "
        """     XX "
        """     XX "
        """        "
        """        "
        """        ";
    case '5': return
        // 01234567
        """        "
        """ XXXXXXX"
        """ XX XXXX"
        """ XX     "
        """ XX     "
        """ XXXXXX "
        """  XXXXXX"
        """      XX"
        """      XX"
        """      XX"
        """      XX"
        """ XX XXXX"
        """  XXXX  "
        """        "
        """        "
        """        ";
    case '6':  return
        // 01234567
        """        "
        """  XXXXX "
        """ XXX  XX"
        """ XX    X"
        """ XX   XX"
        """ XX     "
        """ XX     "
        """ XX XXX "
        """ XXXX XX"
        """ XX   XX"
        """ XX   XX"
        """ XXXXXX "
        """  XXXXX "
        """        "
        """        "
        """        ";
    case '7': return
        // 01234567
        """        "
        """ XXXXXXX"
        """ XXXXXXX"
        """ XX   XX"
        """ X    XX"
        """      XX"
        """     XX "
        """     XX "
        """    XX  "
        """    XX  "
        """   XX   "
        """   XX   "
        """   XX   "
        """        "
        """        "
        """        ";
    case '8': return
        // 01234567
        """        "// 0
        """  XXXXX "// 1
        """ XXX XXX"// 2
        """ XX   XX"// 3
        """ XX   XX"// 4
        """ XX  XXX"// 5
        """  XXXXX "// 6
        """  XXXXX "// 7
        """ XXX  XX"// 8
        """ XX   XX"// 9
        """ XX   XX"// A
        """ XXX XXX"// B
        """  XXXXX "// C
        """        "// D
        """        "// E
        """        "/* F */;
    case '9': return
        // 01234567
        """        "// 0
        """  XXXXX "// 1
        """ XX   XX"// 2
        """ XX   XX"// 3
        """ XX   XX"// 4
        """ XXX  XX"// 5
        """  XXXXXX"// 6
        """      XX"// 7
        """      XX"// 8
        """     XX "// 9
        """    XXX "// A
        """   XX   "// B
        """  XX    "// C
        """        "// D
        """        "// E
        """        "/* F */;
    // ------------------------- Symbols on Numerics --------------------------
    case ')': return
        // 01234567
        """        " // 0
        """    XX  " // 1
        """     XX " // 2
        """      XX" // 3
        """      XX" // 4
        """      XX" // 5
        """      XX" // 6
        """      XX" // 7
        """      XX" // 8
        """      XX" // 9
        """      XX" // A
        """     XX " // B
        """    XX  " // C
        """        " // D
        """        " // E
        """        ";// F
    case '!': return
        // 01234567
        """        " // 0
        """   XXX  " // 1
        """   XXX  " // 2
        """   XXX  " // 3
        """    XX  " // 4
        """    XX  " // 5
        """    XX  " // 6
        """    XX  " // 7
        """    XX  " // 8
        """        " // 9
        """        " // A
        """    XX  " // B
        """    XX  " // C
        """        " // D
        """        " // E
        """        ";// F
    case '@': return
        // 01234567
        """        " // 0
        """   XXX  " // 1
        """  XXXXX " // 2
        """ XX    X" // 3
        """ XX XX X" // 4
        """ XX X  X" // 5
        """ XX XXXX" // 6
        """ XX     " // 7
        """ XX    X" // 8
        """ XX    X" // 9
        """ XX    X" // A
        """  X   X " // B
        """   XXX  " // C
        """        " // D
        """        " // E
        """        ";// F
    case '#': return
        // 01234567
        """        " // 0
        """  XX XX " // 1
        """  XX XX " // 2
        """ XXXXXXX" // 3
        """ XXXXXXX" // 4
        """  XX XX " // 5
        """  XX XX " // 6
        """ XXXXXXX" // 7
        """ XXXXXXX" // 8
        """  XX XX " // 9
        """  XX XX " // A
        """  XX XX " // B
        """        " // C
        """        " // D
        """        " // E
        """        ";// F
    case '$': return
        // 01234567
        """    X   " // 0
        """   XXXXX" // 1
        """  X X   " // 2
        """ XX X   " // 3
        """ XX X   " // 4
        """ XX X   " // 5
        """  XXXXX " // 6
        """    X  X" // 7
        """    X  X" // 8
        """    X  X" // 9
        """    X  X" // A
        """    X X " // B
        """ XXXXX  " // C
        """    X   " // D
        """        " // E
        """        ";// F
    case '%': return
        // 01234567
        """        " // 0
        """  X   XX" // 1
        """ X X  XX" // 2
        """ X X XXX" // 3
        """  X XXX " // 4
        """    XX  " // 5
        """   XXX  " // 6
        """   XX   " // 7
        """  XXX   " // 8
        """  XX  X " // 9
        """ XXX X X" // A
        """ XX  X X" // B
        """ XX   X " // C
        """        " // D
        """        " // E
        """        ";// F
    case '^': return
        // 01234567
        """        " // 0
        """    X   " // 1
        """   XXX  " // 2
        """  XX  X " // 3
        """ XX    X" // 4
        """        " // 5
        """        " // 6
        """        " // 7
        """        " // 8
        """        " // 9
        """        " // A
        """        " // B
        """        " // C
        """        " // D
        """        " // E
        """        ";// F
    case '&': return
        // 01234567
        """    X   "// 0
        """  XXXX  "// 1
        """ X  X X "// 2
        """ X  X  X"// 3
        """    X  X"// 4
        """    X X "// 5
        """  XXXXX "// 6
        """    X X "
        """    X  X"
        """    X  X"
        """ X  X  X"
        """ XX X X "
        """  XXXX  "
        """    X   "
        """        "
        """        ";
    case '*': return
        // 01234567
        """        " // 0
        """        " // 1
        """    XX  " // 2
        """  XXXXXX" // 3
        """   XXX  " // 4
        """  XX  X " // 5
        """  X    X" // 6
        """        " // 7
        """        " // 8
        """        " // 9
        """        " // A
        """        " // B
        """        " // C
        """        " // D
        """        " // E
        """        ";// F
    case '(': return
        // 01234567
        """        " // 0
        """   XX   " // 1
        """  XX    " // 2
        """ XX     " // 3
        """ XX     " // 4
        """ XX     " // 5
        """ XX     " // 6
        """ XX     " // 7
        """ XX     " // 8
        """ XX     " // 9
        """ XX     " // A
        """  XX    " // B
        """   XX   " // C
        """        " // D
        """        " // E
        """        ";// F
    // ---------------------------- Non-printables ----------------------------
    case ' ': default: return
        // 01234567
        """        " // 0
        """        " // 1
        """        " // 2
        """        " // 3
        """        " // 4
        """        " // 5
        """        " // 6
        """        " // 7
        """        " // 8
        """        " // 9
        """        " // A
        """        " // B
        """        " // C
        """        " // D
        """        " // E
        """        ";// F
    }
}

} // end of <anonymous> namespace
