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

#include <common/sf/Util.hpp>

namespace {

using GridVector = cul::Grid<sf::Color>::Vector;

} // end of <anonymous> namespace

namespace cul {

sf::Image to_image(const Grid<sf::Color> & grid) {
    sf::Image img;
    img.create(unsigned(grid.width()), unsigned(grid.height()));
    for (GridVector r; r != grid.end_position(); r = grid.next(r)) {
        img.setPixel(unsigned(r.x), unsigned(r.y), grid(r));
    }
    return img;
}

Grid<sf::Color> to_color_grid(const sf::Image & image) {
    Grid<sf::Color> rv;
    rv.set_size(int(image.getSize().x), int(image.getSize().y));
    for (GridVector r; r != rv.end_position(); r = rv.next(r)) {
        rv(r) = image.getPixel(unsigned(r.x), unsigned(r.y));
    }
    return rv;
}

} // end of cul namespace
