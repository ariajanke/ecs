/****************************************************************************

    MIT License

    Copyright (c) 2022 Aria Janke

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

#include <ariajanke/ecs3/TypeSet.hpp>
#include <ariajanke/ecs3/FunctionTraits.hpp>
#include <ariajanke/ecs3/Scene.hpp>
#include <ariajanke/ecs3/detail/SingleSystem.hpp>

#include <type_traits>
#include <memory>

namespace ecs {

template <typename Type>
class Optional final {
public:
    using ValueType = Type;

    Optional(): m_ptr(nullptr) {}

    explicit Optional(Type * p): m_ptr(p) {}

    Type * operator -> () noexcept { return m_ptr; }

    const Type * operator -> () const noexcept { return m_ptr; }

    Type & operator * () noexcept { return *m_ptr; }

    const Type & operator * () const noexcept { return *m_ptr; }

    explicit operator bool () const noexcept { return m_ptr; }

private:
    Type * m_ptr;
};

template <typename EntityType>
class SingleSystemBase {
public:
    virtual ~SingleSystemBase() {}

    void operator () (const SceneOf<EntityType> & scene) const {
        for (auto e : scene)
            { operate(e); }
    }

    void operator () (EntityType & ent) const
        { operate(ent); }

protected:
    virtual void operate(EntityType &) const = 0;
};

template <typename EntityType, typename ... Functors>
using SinglesSystemFromFunctors =
    typename SinglesSystemFromFunctors_<EntityType, Functors...>::Type;

template <typename EntityType, typename ... Functors>
auto make_singles_system(Functors && ... funcs) {
    return SinglesSystemFromFunctors
        <EntityType, Functors...>
        {std::forward<Functors>(funcs)...};
}

template <typename EntityType, typename ... Functors>
auto make_singles_system_uptr(Functors && ... funcs) {
    return std::make_unique<SinglesSystemFromFunctors
        <EntityType, Functors...>>
        (std::forward<Functors>(funcs)...);
}

} // end of ecs namespace
