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

#include <common/Util.hpp>
#include <common/Vector2Util.hpp>
#include <common/SfmlVectorTraits.hpp>

#include <SFML/System/Vector2.hpp>

#include <iostream>
#include <vector>
#include <stdexcept>
#include <algorithm>

#include <cassert>

// tests performed on the following functions:
// - quad_range
// - normalize
// - and_rectangles
// - rotate_vector
// - project_unto
// - float_equals -> for semantics testing
// - is_nan
//
// other functions are not tested, this is due to the implementation being 
// trivial
// - mag
// - sign
// - pi
// - area_of
// - dot
// - angle_between
// - major
// - rectangle_location
// - right_of
// - bottom_of
// - center_of

namespace {

using namespace cul;
using Error = std::runtime_error;

void test_quad_range();
void test_normalize();
void test_and_rectangles();
void test_rotate_vector();
void test_for_all_of_base();

} // end of <anonymous> namespace

int main() {
    try {
        // quad_range
        test_quad_range();
        // normalize
        test_normalize();
        // and_rectangles
        test_and_rectangles();
        // rotate_vector
        test_rotate_vector();
        // project_unto
        // float_equals -> for semantics testing
        // is_nan
        test_for_all_of_base();
    } catch (std::exception & exp) {
        std::cout << exp.what() << std::endl;
        return ~0;
    }
    return 0;
}

namespace {

class Obj {
public:
    Obj(char c): m_value(c) {}
    static Obj copy_without_marked(const Obj & obj) {
        Obj temp(obj.value());
        return temp;
    }
    void mark_off(const Obj & other) {
        assert(other.m_value != m_value);
        m_marked.push_back(copy_without_marked(other));
    }
    char value() const { return m_value; }
    bool compare(const std::vector<Obj> & col) const {
        auto marked_copy = m_marked;
        marked_copy.push_back(copy_without_marked(*this));
        std::sort(marked_copy.begin(), marked_copy.end());
        return std::equal(col.begin(), col.end(),
                          marked_copy.begin(), marked_copy.end());
    }
    bool operator <  (const Obj & rhs) const { return m_value <  rhs.m_value; }
    bool operator == (const Obj & rhs) const { return m_value == rhs.m_value; }
private:
    char m_value;
    std::vector<Obj> m_marked;
};

// ------------------------- for test_for_all_of_base -------------------------

class Base1 {
protected:
    virtual ~Base1() {}

public:
    virtual char report() const = 0;
};

class Base2 {
public:
    int number() const { return m; }

    void count_off(int & i) { m = ++i; }

private:
    int m = 0;
};

class Base3 {};

class A final : public Base1, public Base2 {
    char report() const final { return 'A'; }
};

class B final : public Base2, public Base3 {

};

class C final : public Base1, public Base2, public Base3 {
    char report() const final { return 'C'; }
};

class D final {};

class E final : public Base1 {
    char report() const final { return 'E'; }
};

class Intr : public Base3 {};

class F final : public Intr {};

void test_quad_range() {
    quad_range<int>({ 4 }, [](int, int) 
        { throw Error("test_quad_range: " + std::to_string(__LINE__)); }
    );
    
    auto do_obj_compare = [](std::initializer_list<char> && charas) {
        std::vector<Obj> objs;
        for (auto c : charas) { objs.emplace_back(c); }
        quad_range(objs, [&](Obj & a, Obj & b) {
            a.mark_off(b);
            b.mark_off(a);
        });
        for (const auto & obj : objs) {
            if (!obj.compare(objs)) {
                throw Error("test_quad_range: size value " + 
                            std::to_string(objs.size())      );
            }
        }
    };
    
    do_obj_compare({ 'a', 'b' });
    do_obj_compare({ 'a', 'b', 'c' });
    do_obj_compare({ 'a', 'b', 'c', 'd' });
    do_obj_compare({ 'a', 'b', 'c', 'd', 'e' });
}

void test_normalize() {
    bool flag = false;
    try {
        normalize(0);
    } catch (std::exception &) {
        flag = true;
    }
    if (!flag) {
        throw Error("test_normalize: " + std::to_string(__LINE__));
    }
    flag = false;
    try {
        cul::normalize(sf::Vector2<double>(0.0, 0.0));
    } catch (std::exception &) {
        flag = true;
    }
    if (!flag) {
        throw Error("test_normalize: " + std::to_string(__LINE__));
    }
}

void test_and_rectangles() {
    // plans for testing?!
}

void test_rotate_vector() {
    // plans for testing?!
}

void test_for_all_of_base() {
    {
    A a;
    B b;
    C c;
    D d;
    E e;
    F f;
    std::string str;
    for_all_of_base<Base1>(std::tie(a, b, c, d, e, f), [&str](const Base1 & base) {
        str += base.report();
    });
    assert(str == "ECA");
    }
    {
    auto t1 = std::make_tuple(A{}, B{}, C{}, D{}, E{}, F{});
    int count = 0;
    std::vector<Base2 *> bases;
    for_all_of_base<Base2>(t1, [&count, &bases](Base2 & base) {
        base.count_off(count);
        bases.push_back(&base);
    });
    std::sort(bases.begin(), bases.end(), [](Base2 * lhs, Base2 * rhs) {
        return lhs->number() < rhs->number();
    });
    assert(std::unique(bases.begin(), bases.end(), [](Base2 * lhs, Base2 * rhs) { return lhs->number() == rhs->number(); }) == bases.end());
    }
    {
    A a;
    B b;
    C c;
    D d;
    E e;
    F f;
    int count = 0;
    std::vector<Base2 *> bases;
    for_all_of_base<Base2>(std::tie(a, b, c, d, e, f), [&count, &bases](Base2 & base) {
        base.count_off(count);
        bases.push_back(&base);
    });
    std::sort(bases.begin(), bases.end(), [](Base2 * lhs, Base2 * rhs) {
        return lhs->number() < rhs->number();
    });
    assert(std::unique(bases.begin(), bases.end(), [](Base2 * lhs, Base2 * rhs) { return lhs->number() == rhs->number(); }) == bases.end());
    }
    {
    A a;
    B b;
    C c;
    D d;
    E e;
    F f;
    int count = 0;
    for_all_of_base<Base3>(std::tie(a, b, c, d, e, f), [&count](const Base3 &) {
        ++count;
    });
    assert(count == 3);
    }
    // multiple of same type... couldn't do this on the old implementation!
    {
    auto t1 = std::make_tuple(A{}, B{}, A{}, E{}, E{}, B{});
    int count = 0;
    for_all_of_base<Base2>(t1, [&count](const Base2 &) {
        ++count;
    });
    assert(count == 4);
    }
    // common base
    {
    // all share base 2
    // A, C are base 1
    // B, C, are base 3
    //                       1     10   10   10   10    1
    auto t1 = std::make_tuple(A{}, B{}, B{}, C{}, B{}, A{});
    for_all_of_base<Base1>(t1, [](Base2 & b2) {
        int i = 0;
        b2.count_off(i);
    });
    for_all_of_base<Base3>(t1, [](Base2 & b2) {
        int i = 9;
        b2.count_off(i);
    });
    int count = 0;
    for_all_of_base<Base2>(t1, [&count](Base2 & b2) { count += b2.number(); });
    assert(count == 42);
    }
}

} // end of <anonymous> namespace
