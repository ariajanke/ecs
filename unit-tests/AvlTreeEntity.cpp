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

Tuple<bool, int> is_avl_and_height(const ecs::NodeInstance * node) {
    using std::make_tuple, std::abs, std::max;
    static Tuple<bool, int> k_avl_failed{false, -1};

    if (!node) return make_tuple(true, 0);
    if (node->left() && node->right()) {
        auto [left_avl , left_h ] = is_avl_and_height(node->left ());
        auto [right_avl, right_h] = is_avl_and_height(node->right());
        {}
        if (!left_avl || !right_avl || abs(left_h - right_h) > 1)
            { return k_avl_failed; }
        return make_tuple(true, max(left_h, right_h) + 1);
    }
    auto set_side = node->left() ? node->left() : node->right();
    if (!set_side)
        { return make_tuple(true, 1); }

    return is_avl_and_height(set_side);
}

bool is_avl(const ecs::NodeInstance * node)
    { return std::get<0>(is_avl_and_height(node)); }

bool is_avl(const ecs::NodeOwningPtr & node)
    { return is_avl(node.get()); }

using AvlInsertRes = ecs::NodeInstance::AvlInsertRes;

template <std::size_t k_count>
AvlInsertRes insert_nodes
    (std::array<ecs::NodeOwningPtr, k_count> & nodes, AvlInsertRes res = AvlInsertRes{})
{
    using Ni = ecs::NodeInstance;
    for (auto & node : nodes) {
        res = Ni::avl_insert(move(res.root), move(node));
        assert(!res.given);
    }
    return res;
}

template <typename T>
T & rvalue_as_ref(T && obj) { return obj; }

AvlInsertRes insert_nodes
    (std::initializer_list<ecs::NodeOwningPtr *> nodes, AvlInsertRes res = AvlInsertRes{})
{
    using Ni = ecs::NodeInstance;
    for (auto & node : nodes) {
        res = Ni::avl_insert(move(res.root), move(*node));
        assert(!res.given);
    }
    return res;
}

#define mark MACRO_MARK_POSITION_OF_CUL_TEST_SUITE

} // end of <anonymous> namespace

bool test_avltreeentity() {
    using namespace cul::ts;
    using std::move, ecs::NodeOwningPtr;
    using Ni = ecs::NodeInstance;
    TestSuite suite;
    suite.start_series("Polymorphic AVL tree nodes");
    reset_all_counts();
    mark(suite).test([] {
        int fc = [] {

            auto nodes = ecs::make_multiple_type_nodes<A, B, C>();
            //nodes[0]->link(std::move(nodes[1]));
            //nodes[0]->link(std::move(nodes[2]));
            return AllInst::count();
        } ();
        return test(fc == 3 && AllInst::count() == 0);
    });
    reset_all_counts();
    // play types -> keys in order of their alphabit placement
    mark(suite).test([] {
        int fc = [] {
            auto nodes = ecs::make_multiple_type_nodes<A, B, C>();
            // this creates a LL case c:
            auto res = Ni::avl_insert(move(nodes[0]), move(nodes[1]));
            assert(!res.given);
            res = Ni::avl_insert(move(res.root), move(nodes[2]));
            assert(res.root->ptr<A>());
            assert(res.root->ptr<B>());
            assert(res.root->ptr<C>());
            return AllInst::count();
        } ();
        return test(fc == 3 && AllInst::count() == 0);
    });
    reset_all_counts();
    // LR case, should also check by debugger that control hits the correct
    // handling code
    mark(suite).test([] {
        bool is_avl_ = [] {
            auto nodes = ecs::make_multiple_type_nodes<A, B, C>();
            auto res = Ni::avl_insert(move(nodes[2]), move(nodes[0]));
            res = Ni::avl_insert(move(res.root), move(nodes[1]));
            return is_avl(res.root);
        } ();
        return test(AllInst::count() == 0 && is_avl_);
    });
    reset_all_counts();
    // basic tree operation like bst remove should be tested!
    // bst remove tests:
    // null root
    mark(suite).test([] {
        auto [aff, root, rmv] = Ni::bst_remove(NodeOwningPtr{}, 3);
        return test(!aff.parent && !root  && !rmv);
    });
    // remove 1
    mark(suite).test([] {
        auto anode = ecs::make_single_type_node<A>();
        auto [aff, root, rmv] = Ni::bst_remove(move(anode), k_a_key); {}
        return test(!aff.parent && !root && rmv->ptr<A>());
    });
    // leaf of right heavy 2
    mark(suite).test([] {
        auto nodes = ecs::make_multiple_type_nodes<A, B>();
        auto root = insert_nodes(nodes).root;
        auto [aff, nroot, rmv] = Ni::bst_remove(move( root ), k_b_key); {}
        return test(!aff.parent && nroot->ptr<A>() && rmv->ptr<B>());
    });
    // leaf of left heavy 2
    mark(suite).test([] {
        auto nodes = ecs::make_multiple_type_nodes<B, A>();
        auto root = insert_nodes(nodes).root;
        auto [aff, nroot, rmv] = Ni::bst_remove(move( root ), k_a_key); {}
        return test(!aff.parent && nroot->ptr<B>() && rmv->ptr<A>());
    });
    // root of right heavy 2
    mark(suite).test([] {
        auto nodes = ecs::make_multiple_type_nodes<A, B>();
        auto root = insert_nodes(nodes).root;
        auto [aff, nroot, rmv] = Ni::bst_remove(move( root ), k_a_key); {}
        return test(!aff.parent && nroot->ptr<B>() && rmv->ptr<A>());
    });
    // root of left heavy 2
    mark(suite).test([] {
        auto nodes = ecs::make_multiple_type_nodes<B, A>();
        auto root = insert_nodes(nodes).root;
        auto [aff, nroot, rmv] = Ni::bst_remove(move( root ), k_b_key); {}
        return test(!aff.parent && nroot->ptr<A>() && rmv->ptr<B>());
    });
    // leaf balanced 3
    // root balanced 3
    // intr of deeper left heavy
    // intr of deeper right heavy
    // intr that's root of balanced 3
    // leaf of deeper
    mark(suite).test([] {
        return test([] {
            auto nodes = ecs::make_multiple_type_nodes<A, B>();
            auto res = Ni::avl_insert(move(nodes[0]), move(nodes[1]));
            auto [affected, root, removed] = Ni::bst_remove(move(res.root), k_b_key); {}
            return    removed && root && removed->ptr<B>() && root->ptr<A>()
                   && AllInst::count() == 2;
        } () && AllInst::count() == 0);
    });
    reset_all_counts();
    // bst remove test: removing root element should not remove all elements!
    mark(suite).test([] {
        auto root = insert_nodes(rvalue_as_ref(ecs::make_multiple_type_nodes<A, B, C, D, F>())).root;
        auto root_key = root->key();
        root = Ni::bst_remove(std::move(root), root_key).root;
        // a's key is 0
        if (!root) return test(false);
        return test(root->ptr<A>());
    });
    // trees must remain AVL, must query okay, must clean up after themselves
    //
    // insertion: one regular case, one reject case, all four inbalance cases
    // deletion: something that's different from the BST tests
    // (idea: is there a bst case that results in an inbalanced tree that I can use?)
    //
    // I need more tests! Test correct lifetime for individual types on
    // multinodes (current impl will fail this!)
    // tests to catch "on the fly" problems
    mark(suite).test([] {
        bool preokay = [] {
            auto nodes = ecs::make_multiple_type_nodes<A, B, C>();
            auto enode = ecs::make_single_type_node<E>(9.f, true, "hello");
            auto dnode = ecs::make_single_type_node<D>();

            auto res = insert_nodes({ &enode, &dnode }, insert_nodes(nodes));
            bool found_everything = res.root->ptr<A>();
            found_everything &= !!res.root->ptr<B>();
            found_everything &= !!res.root->ptr<C>();
            found_everything &= !!res.root->ptr<D>();
            found_everything &= !!res.root->ptr<E>();
            return found_everything && AllInst::count() == 5 && is_avl(res.root);
        } ();
        return test(preokay && AllInst::count() == 0);
    });
    ecs::make_multiple_type_nodes<A, B, C, D, F>();
    mark(suite).test([] {
        // size 16 align 8
        struct Pa final {
            // depends on little-endianness of the machine
            Pa() { reinterpret_cast<char &>(m.front()) = 'a'; }
            std::array<uint64_t, 16 / 8> m;
        };
        // size 24 align 8
        struct Pb final {
            Pb() { reinterpret_cast<char &>(m.front()) = 'b'; }
            std::array<uint64_t, 24 / 8> m;
        };
        // size 8 align 8
        struct Pc final {
            Pc() { reinterpret_cast<char &>(m.front()) = 'c'; }
            std::array<uint64_t, 8 / 8> m;
        };
        // size 16 align 8
        struct Pd final {
            Pd() { reinterpret_cast<char &>(m.front()) = 'd'; }
            std::array<uint64_t, 16 / 8> m;
        };
        // size 16 align 8
        struct Pe final {
            Pe() { reinterpret_cast<char &>(m.front()) = 'e'; }
            std::array<uint64_t, 16 / 8> m;
        };
        // size 128 align 8
        struct Pf final {
            Pf() { reinterpret_cast<char &>(m.front()) = 'f'; }
            std::array<uint64_t, 128 / 8> m;
        };

        {
        ecs::make_multiple_type_nodes<Pa, Pb, Pc, Pd, Pe, Pf>();
        }
        return test(true);
    });
    return suite.has_successes_only();
}


namespace {


} // end of <anonymous> namespace
