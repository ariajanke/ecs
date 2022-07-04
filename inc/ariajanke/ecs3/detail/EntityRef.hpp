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

#include <ariajanke/ecs3/detail/defs.hpp>

namespace ecs {

class EntityBodyBase {
public:
    EntityBodyBase() {}

    virtual ~EntityBodyBase() {}

    void * downcast(Size safety) noexcept;

    const void * downcast(Size safety) const noexcept;

protected:

    virtual const void * downcast_(Size) const noexcept = 0;
};

class EntityRefAttn;
class ConstEntityRef;

template <typename EntityType>
class HomeSceneBase {
public:
    virtual ~HomeSceneBase() {}
    virtual void on_create(const EntityType &) = 0;
    virtual void on_deletion_request(const EntityType &) = 0;

    static HomeSceneBase & no_scene();
};

template <typename FullEntity>
class ConstEntityBase;

template <typename FullEntity>
class EntityBase;

class ConstEntityRef;

} // end of ecs namespace
