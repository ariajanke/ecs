/***************************************************************************

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

#include "shared.hpp"

namespace {

bool test_storage();

bool test_hashtable();

} // end of <anonymous> namespace

bool test_hashtableentity() {
    // do not shortcut
    return andf(test_storage(), test_hashtable());
}

namespace {

#define mark MACRO_MARK_POSITION_OF_CUL_TEST_SUITE

bool test_storage() {
    using namespace cul::ts;
    TestSuite suite;
    suite.start_series("heterogeneous typed table storage");
    using Storage = ecs::HeterogeneousHashTable::Storage;
    using Byte = std::byte;
    using MaxAlign = std::max_align_t;
    static constexpr const auto k_max_align = sizeof(MaxAlign);
    mark(suite).test([] {
        return test(Storage::make_new(4, 12).total_space() >= 12);
    });
    mark(suite).test([] {
        return test(Storage::make_new(4, 12).available_space(4) >= 12);
    });
    mark(suite).test([] {
        return test(Storage::make_new(4, 12).used_space() == 0);
    });
    mark(suite).test([] {
        auto s = Storage::make_new(4, 16);
        s.next_component_space(1, 12);
        s.mark_lost_bytes(10);
        return test(s.available_space(1) >= 4);
    });
    mark(suite).test([] {
        auto s = Storage::make_new(4, 12);
        return test(!s.next_component_space(1, k_max_align + 1));
    });
    mark(suite).test([] {
        auto s = Storage::make_new(4, k_max_align);
        return test(s.next_component_space(1, k_max_align));
    });
    mark(suite).test([] {
        assert(k_max_align > 8);
        auto s = Storage::make_new(4, k_max_align);
        (void)s.next_component_space(k_max_align / 2, k_max_align / 4);
        (void)s.next_component_space(k_max_align / 2, k_max_align / 2);
        return test(s.available_space(1) == 0);
    });
    mark(suite).test([] {
        auto s = Storage::make_new(4, 16);
        auto myi32 = s.next_component_space(4, 4);
        auto myi64 = s.next_component_space(8, 8);
        return test(reinterpret_cast<Byte *>(myi64) - reinterpret_cast<Byte *>(myi32) == 8);
    });
    mark(suite).test([] {
        auto s = Storage::make_new(4, 16);
        (void)s.next_component_space(4, 4);
        (void)s.next_component_space(8, 8);
        s.mark_lost_bytes(8);
        auto s2 = s.make_new_without_lost();
        return test(   s2.lost_space() == 0
                    && s2.available_space(1) >= 8);
    });
    return suite.has_successes_only();
}

bool test_hashtable() {
    using namespace cul::ts;
    TestSuite suite;
    using HetTable = ecs::HeterogeneousHashTable;
    suite.start_series("heterogeneous typed hash table");
    reset_all_counts();
    mark(suite).test([] {
        int fc = [] {
            HetTable tab;
            (void)tab.append<A>();
            return Counted<A>::count();
        } ();
        return test(fc == 1 && Counted<A>::count() == 0);
    });
    reset_all_counts();
    mark(suite).test([] {
        int fc = [] {
            HetTable tab;
            (void)tab.append<A>();
            (void)tab.append<B>();
            return AllInst::count();
        } ();
        return test(fc == 2 && AllInst::count() == 0);
    });
    reset_all_counts();
    mark(suite).test([] {
        HetTable tab;
        (void)tab.append<A>();
        (void)tab.append<B>();
        return test(tab.get<A>() && tab.get<B>());
    });
    reset_all_counts();
    // reserve needs to be tested
    // "reallocation" *must* not fire following the call to reserve
    mark(suite).test([] {
        struct G final : std::max_align_t {};
        HetTable tab;
        tab.reserve_for_more(TypeList<D, E, G>{});
        void * d = &tab.append<D>();
        tab.append<E>(0.f, true, "");
        tab.append<G>();
        void * gd = tab.get<D>();
        return test(d == gd);
    });
    reset_all_counts();
    // must not double destruct!
    mark(suite).test([] {
        int a_count = [] {
            HetTable tab;
            tab.append<A>();
            tab.append<B>();
            tab.remove<A>();
            return Counted<A>::count();
        } ();
        return test(a_count == 0 && Counted<A>::count() == 0);
    });
    reset_all_counts();
    return suite.has_successes_only();
}

} // end of <anonymous> namespace
