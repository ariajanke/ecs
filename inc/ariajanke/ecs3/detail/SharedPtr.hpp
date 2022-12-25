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

#include <ariajanke/ecs3/defs.hpp>

#include <string>
#include <vector>
#include <atomic>

/// @file detail/SharedPtr.hpp
///
/// @see file SharedPtr.hpp for more info

namespace ecs {

class SwPtrAttn;

class SwPtrPriv final {
    friend class SwPtrAttn;

    template <typename T>
    friend class WeakPtr;

    template <typename T>
    friend class SharedPtr;

    class RefCounter { // must not be final
    public:
        virtual void delete_this() = 0;

        std::atomic_int owners = 0;
        std::atomic_int observers = 0;

    protected:
        RefCounter() {}

        virtual ~RefCounter() {}
    };

    template <typename T>
    struct ParentVectorHolder;

    template <typename T>
    struct VectorObjectHolder final : public RefCounter {
        VectorObjectHolder() {}

        explicit VectorObjectHolder(ParentVectorHolder<T> * parent_):
            RefCounter(),
            parent(parent_) {}

        // hold the compiler's hand like the five year old that it is
        VectorObjectHolder(const VectorObjectHolder & lhs):
            RefCounter(),
            parent(lhs.parent) {}

        VectorObjectHolder(VectorObjectHolder && lhs):
            RefCounter(),
            parent(lhs.parent) {}

        VectorObjectHolder & operator = (const VectorObjectHolder & lhs) {
            parent = lhs.parent;
            return *this;
        }

        VectorObjectHolder & operator = (VectorObjectHolder && lhs) {
            parent = lhs.parent;
            return *this;
        }

        ~VectorObjectHolder() {}

        void delete_this() final {
            // shouldn't we destruct this too?
            // (both are trivially destructable)
            auto parent = this->parent;
            this->~VectorObjectHolder();
            if (--parent->counter) return;
            return delete parent; // <- this will "delete this"
        }

        StorageFor<T> storage;
        ParentVectorHolder<T> * parent = nullptr;
    };

    template <typename T>
    struct ParentVectorHolder final {
        std::vector<VectorObjectHolder<T>> counter_and_storages;
        std::atomic_int counter = 0;
    };

    struct RefCounterPtrHash final {
        std::size_t operator () (const RefCounter * ptr) const noexcept
            { return ptr ? std::hash<const RefCounter *>{}(ptr) : 0; }
    };
};

} // end of ecs namespace
