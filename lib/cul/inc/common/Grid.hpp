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

#include <vector>
#include <stdexcept>
#include <utility>
#include <functional>
#include <limits>

#include <common/Vector2.hpp>

namespace cul {

/** Container class meant to be like std::vector but laid out in two
 *  diminsions.
 */
template <typename T>
class Grid {
public:
    using Iterator           = typename std::vector<T>::iterator       ;
    using ConstIterator      = typename std::vector<T>::const_iterator ;
    using Element            = typename std::vector<T>::value_type     ;
    using ReferenceType      = typename std::vector<T>::reference      ;
    using IndexType          = int;
    using ConstReferenceType = typename std::vector<T>::const_reference;
    using Vector             = Vector2<IndexType>;
    using Size               = Size2<IndexType>;


    Grid() {}
    explicit Grid(std::initializer_list<std::initializer_list<T>>);
    Grid(const Grid &) = default;
    Grid(Grid &&) = default;
    ~Grid() {}

    Grid & operator = (const Grid &) = default;
    Grid & operator = (Grid &&) = default;

    // --------------------------- cul/Grid specific ---------------------------

    /** @return grid width in number of elements */
    int width() const noexcept;
    
    /** @return grid height in number of elements */
    int height() const noexcept;

    /** @brief Sets grid width in number of elements
     *  @note  This behaves exactly like std::vector<T>::resize
     *  @param new_width new grid width 
     *  @param e default element value to fill the space
     */
    void set_width(int new_width, Element && e = Element());
    
    /** @copydoc Grid<T>::set_width(int,Element&&) */
    void set_width(int new_width, const Element & e);
    
    /** @brief sets grid height in number of elements
     *  @note  This behaves exactly like std::vector<T>::resize
     *  @param new_width new grid height
     *  @param e default
     */
    void set_height(int new_height, Element && = Element());

    /** @copydoc Grid<T>::set_height(int,Element&&) */
    void set_height(int new_height, const Element &);

    /** @brief set grid size in number of elements
     *  @note  This behaves exactly like std::vector<T>::resize
     *  @param width new width
     *  @param height new height
     *  @param e default element value to fill the space
     */
    void set_size(int width, int height, Element && e = Element());
    
    /** @copydoc Grid<T>::set_size(int,int,Element&&) */
    void set_size(int width, int height, const Element &);

    /** @brief Reserves memory for exactly n elements in the container
     *  @note Does exactly std::vector<T>::reserve.
     *  @param n reserves memory for n many elements
     */
    void reserve(std::size_t n);

    /** @returns true if position is inside the grid */
    bool has_position(int x, int y) const noexcept;
    
    /** @returns true if position is inside the grid */
    bool has_position(const Vector &) const noexcept;

    /** Advances the given vector to the next grid position.
     *  @returns either a valid grid position (if the given position is inside
     *           the grid), or the "end_position" of the grid, if the given
     *           position is *not* inside the grid, then this value is not
     *           defined.
     */
    Vector next(const Vector &) const noexcept;

    /** The end position of the grid is the "one past the end" position
     *  expressed as a vector.
     *
     *  It is not a valid grid position and should be treated the same way you
     *  would treat a value given by "end()"
     *
     *  @returns the end position
     */
    Vector end_position() const noexcept;

    /** Gets the vector position based on an iterator.
     *  @throws An attempt is made to identify whether the given iterator is
     *          contained in the underlying container. This cannot be guarenteed
     *          to work, as comparisons between iterators of two different
     *          parent containers is not defined by the standard.
     *  @returns the vector position of the iterator.
     */
    Vector position_of(ConstIterator) const;
    
    /** @returns position of the element, if the actual element instance lives 
     *           inside the container (that is passing any copies will trigger
     *           an out of range exception.
     *  @note there's some detail in the C++ standard which says that testing
     *        whether a pointer is inside a container's range is OK and defined
     *        behavior (citation will be provided when Internet becomes 
     *        available).
     */
    Vector position_of(const Element &) const;

    /** @returns the size of the grid in two dimensions: width and height */
    Size size2() const noexcept;

    // -------------------------- STL like functions ---------------------------

    ReferenceType      operator () (const Vector &);
    ConstReferenceType operator () (const Vector &) const;

    ReferenceType      operator () (int x, int y);
    ConstReferenceType operator () (int x, int y) const;

    Iterator begin() { return m_elements.begin(); }
    Iterator end  () { return m_elements.end  (); }
    
    ConstIterator begin() const { return m_elements.begin(); }
    ConstIterator end  () const { return m_elements.end  (); }

    std::size_t size() const noexcept { return m_elements.size(); }
    
    void clear() { m_elements.clear(); }

    void swap(Grid<T> &) noexcept;
    
    /** same behavior as std::vector<T>::empty */
    bool is_empty() const noexcept;

protected:
    ReferenceType      element(int x, int y);
    ConstReferenceType element(int x, int y) const;

    ReferenceType      element(const Vector &);
    ConstReferenceType element(const Vector &) const;

private:
    std::size_t to_index(int x, int y) const noexcept;
    Vector to_position(std::ptrdiff_t) const noexcept;

    std::invalid_argument make_out_of_range_error() const noexcept;

    std::vector<T> m_elements;
    int m_width = 0;
};

// ----------------------------------------------------------------------------

template <typename T>
Grid<T>::Grid(std::initializer_list<std::initializer_list<T>> init_list) {
    static constexpr const int k_uninit = -1;
    int width_ = k_uninit;
    for (const auto & inner_list : init_list) {
        // I cannot test this, I cannot create a text file large enough
        if (inner_list.size() > std::numeric_limits<int>::max()) {
            throw std::invalid_argument("Grid<T>::Grid: exceeds maximum value of integer type.");
        }
        if (width_ == k_uninit) {
            width_ = int(inner_list.size());
        } else if (width_ != int(inner_list.size())) {
            throw std::invalid_argument("Grid<T>::Grid: all inner lists must be the same size.");
        }
    }
    m_elements.reserve(width_*init_list.size());
    for (const auto & inner_list : init_list) {
        m_elements.insert(m_elements.end(), inner_list.begin(), inner_list.end());
    }
    m_width = width_;
}

template <typename T>
int Grid<T>::width() const noexcept { return m_width; }

template <typename T>
int Grid<T>::height() const noexcept
    { return (m_elements.empty()) ? 0 : int(m_elements.size()) / m_width; }

template <typename T>
void Grid<T>::set_width(int width_, Element && obj)
    { set_size(width_, height(), std::move(obj)); }

template <typename T>
void Grid<T>::set_width(int width_, const Element & obj)
    { set_size(width_, height(), obj); }

template <typename T>
void Grid<T>::set_height(int height_, Element && obj)
    { set_size(width(), height_, std::move(obj)); }

template <typename T>
void Grid<T>::set_height(int height_, const Element & obj)
    { set_size(width(), height_, obj); }

template <typename T>
void Grid<T>::set_size(int width_, int height_, Element && obj)
    { set_size(width_, height_, std::cref(obj)); }

template <typename T>
void Grid<T>::set_size(int width_, int height_, const Element & obj) {
    if (width_ < 0 || height_ < 0) {
        throw std::invalid_argument("Grid::set_size: both dimensions must be non-negative integers.");
    }
    m_elements.resize(std::size_t(width_*height_), obj);
    m_width = width_;
}

template <typename T>
void Grid<T>::reserve(std::size_t n) { m_elements.reserve(n); }

template <typename T>
typename Grid<T>::ReferenceType Grid<T>::operator ()(const Vector & r)
    { return element(r); }

template <typename T>
typename Grid<T>::ConstReferenceType
    Grid<T>::operator () (const Vector & r) const
    { return element(r); }

template <typename T>
typename Grid<T>::ReferenceType Grid<T>::operator () (int x, int y)
    { return element(x, y); }

template <typename T>
typename Grid<T>::ConstReferenceType Grid<T>::operator () (int x, int y) const
    { return element(x, y); }

template <typename T>
bool Grid<T>::has_position(int x, int y) const noexcept
    { return x >= 0 && y >= 0 && x < width() && y < height(); }

template <typename T>
bool Grid<T>::has_position(const Vector & r) const noexcept
    { return has_position(r.x, r.y); }

template <typename T>
typename Grid<T>::Vector Grid<T>::next(const Vector & r) const noexcept {
    // possible invalid argument (r is out of range)
    auto pos = r;
    if (++pos.x == width()) {
        pos.x = 0;
        ++pos.y;
    }
    return pos;
}

template <typename T>
typename Grid<T>::Vector Grid<T>::end_position() const noexcept
    { return Vector(0, height()); }

template <typename T>
typename Grid<T>::Vector Grid<T>::position_of(ConstIterator itr) const {
    if (is_empty() ? true : itr < begin() || itr > end()) {
        throw std::out_of_range("Grid::position_of: positions are only "
                                "findable for iterators contained in this "
                                "container.");
    }
    return to_position(std::ptrdiff_t(size()) - (end() - itr));
}

template <typename T>
typename Grid<T>::Vector Grid<T>::position_of(const Element & obj) const {
    static constexpr const char * const k_oor_msg = "Grid::position_of: "
        "positions are only findable for references contained in this "
        "container.";
    
    if (is_empty()) {
        throw std::out_of_range(k_oor_msg);
    } else {
        const T * first = &*begin();
        const T * last  = &*(end() - 1);
        if (&obj < first || &obj > last) {
            throw std::out_of_range(k_oor_msg);
        }
        const T * end_ptr = first + size();
        return to_position(std::ptrdiff_t(size()) - (end_ptr - &obj));
    }
}

template <typename T>
typename Grid<T>::Size Grid<T>::size2() const noexcept
    { return Size{width(), height()}; }

template <typename T>
void Grid<T>::swap(Grid<T> & other) noexcept {
    m_elements.swap(other.m_elements);
    std::swap(m_width, other.m_width);
}

template <typename T>
bool Grid<T>::is_empty() const noexcept { return m_elements.empty(); }

template <typename T>
/* protected */ typename Grid<T>::ReferenceType Grid<T>::element(int x, int y) {
    if (!has_position(x, y)) throw make_out_of_range_error();
    return m_elements[to_index(x, y)];
}

template <typename T>
/* protected */ typename Grid<T>::ConstReferenceType
    Grid<T>::element(int x, int y) const
{
    if (!has_position(x, y)) throw make_out_of_range_error();
    return m_elements[to_index(x, y)];
}

template <typename T>
/* protected */ typename Grid<T>::ReferenceType
    Grid<T>::element(const Vector & r)
    { return element(r.x, r.y); }

template <typename T>
/* protected */ typename Grid<T>::ConstReferenceType
    Grid<T>::element(const Vector & r) const
    { return element(r.x, r.y); }

// keep these two functions together so that index/position conversion stays 
// consistent

template <typename T>
/* private */ std::size_t Grid<T>::to_index(int x, int y) const noexcept
    { return std::size_t(x + y*width()); }

template <typename T>
/* private */ typename Grid<T>::Vector Grid<T>::to_position
    (std::ptrdiff_t r) const noexcept
{ return Vector(r % width(), r / width()); }

template <typename T>
/* private */ std::invalid_argument Grid<T>::make_out_of_range_error() const noexcept {
    return std::invalid_argument("Grid::element: requested element is out of range, "
                                 "field size: width " + std::to_string(width()) +
                                 " height " + std::to_string(height()));
}

} // end of cul namespace
