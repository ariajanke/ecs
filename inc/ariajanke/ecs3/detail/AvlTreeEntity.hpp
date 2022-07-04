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
#include <ariajanke/ecs3/EntityRef.hpp>

#include <memory>

#include <cassert>

namespace ecs {

class NodeSource {
public:
    virtual ~NodeSource() {}
    virtual void decrement(void *) noexcept = 0;
};

class NodeInstanceAttn;

class NodeInstance;

struct NodeDeletor final {
    void operator () (NodeInstance * inst) const noexcept;
};

using NodeOwningPtr = std::unique_ptr<NodeInstance, NodeDeletor>;

template <typename ... Types>
std::array<NodeOwningPtr, sizeof...(Types)> make_multiple_type_nodes();

template <typename Type, typename ... ConstructorTypes>
NodeOwningPtr make_single_type_node(ConstructorTypes && ... args);

NodeOwningPtr move_ptr(NodeInstance *&);

NodeOwningPtr rebalance_left(NodeOwningPtr &);

// :TODO: refactor down to keys (no types!)
// I will absolutely need to test my AVL operations!
//
// on AVL operations:
// I've used numerous websites to figure out how to do this. It appears there
// are few resources that refrain from sharing source (very unfortunate). As I
// am not interested in getting in any legal for just trying to understand and
// implement stuff that's all basic, undergrad CS stuff.
class NodeInstance {
public:
    friend class NodeInstanceAttn;

    NodeInstance(const NodeInstance &) = delete;
    NodeInstance(NodeInstance &&) = delete;

    ~NodeInstance() {}

    NodeInstance & operator = (const NodeInstance &) = delete;
    NodeInstance & operator = (NodeInstance &&) = delete;

    // a ton of subtypes for tree operations

    struct AvlInsertRes final {
        AvlInsertRes() {}
        AvlInsertRes(const AvlInsertRes &) = delete;
        AvlInsertRes(AvlInsertRes &&) = default;

        AvlInsertRes(NodeOwningPtr && root_):
            root(std::move(root_)) {}

        AvlInsertRes(NodeOwningPtr && root_, NodeOwningPtr && given_):
            root(std::move(root_)), given(std::move(given_)) {}

        AvlInsertRes & operator = (AvlInsertRes && rhs) = default;

        NodeOwningPtr root, given;
    };

    struct AvlRemoveRes final {
        AvlRemoveRes() {}
        AvlRemoveRes(const AvlRemoveRes &) = delete;

        explicit AvlRemoveRes(NodeOwningPtr && root_):
            root(std::move(root_)) {}

        AvlRemoveRes(NodeOwningPtr && root_, NodeOwningPtr && removed_):
            root(std::move(root_)), removed(std::move(removed_)) {}

        AvlRemoveRes & operator = (AvlRemoveRes && rhs) = default;

        NodeOwningPtr root, removed;
    };

    struct ChildParentRef final {
        enum class Side { left, right };
        ChildParentRef() {}
        ChildParentRef(NodeInstance * parent_, NodeInstance * side):
            parent(parent_),
            child (parent_->m_left == side ? Side::left : Side::right)
        {
            assert(parent_->m_left == side || parent_->m_right == side);
        }
        NodeInstance * parent = nullptr;
        Side child = Side::left;
        NodeInstance *& child_pointer() const noexcept {
            assert(parent);
            return child == Side::left ? parent->m_left : parent->m_right;
        }
    };

    struct BstRemoveRes final {
        BstRemoveRes() {}
        BstRemoveRes(const BstRemoveRes &) = delete;
        BstRemoveRes(NodeOwningPtr && r_, NodeOwningPtr && rm_):
            root(std::move(r_)),
            removed(std::move(rm_)) {}

        BstRemoveRes(ChildParentRef && affected_,
                     NodeOwningPtr && root_, NodeOwningPtr && rm_):
            affected(std::move(affected_)),
            root(std::move(root_)),
            removed(std::move(rm_))
        {}

        BstRemoveRes append_to_super(NodeOwningPtr && super_root, NodeInstance *& side) {
            using std::move;
            assert(!side);
            side = root.release();
            return BstRemoveRes{move(affected), move(super_root), move(removed)};
        }
        // [NTS]
        // consider what this should look like for various tree structures...
        //
        // if an immediate child is removed, it's not possible to have an
        // "effected" parent-child node pair, as there is only one
        ChildParentRef affected;
        NodeOwningPtr root;
        NodeOwningPtr removed;
    };
public:

    template <typename T>
    T * ptr() const { return reinterpret_cast<T *>(ptr_(MetaFunctions::key_for_type<T>())); }

    const NodeInstance * left() const noexcept { return m_left; }

    const NodeInstance * right() const noexcept { return m_right; }

    // insert... which... returns the new root node
    static AvlInsertRes avl_insert(NodeOwningPtr root, NodeOwningPtr newnode);

    // static NodeOwningPtr avl_remove(NodeOwningPtr & root, Size key);

    static bool is_avl(const NodeOwningPtr & root)
        { return is_avl(root.get()); }
private:
    static bool is_avl(const NodeInstance * root) {
        // an empty tree is (assumed) AVL
        if (!root) return true;
        if (std::abs(root->balance()) > 1) return false;
        return is_avl(root->m_left) && is_avl(root->m_right);
    }

private:

    static NodeInstance *& find_inorder_successor(NodeInstance *& node, Size key) {
        static NodeInstance * k_null = nullptr;
        if (!node) return k_null;
        if (!node->m_left && !node->m_right && node->key() > key)
            { return node; }
        auto & res = find_inorder_successor(node->m_left, key);
        if (res) return res;
        return find_inorder_successor(node->m_right, key);
    }

    static BstRemoveRes bst_remove_on_side
        (NodeOwningPtr & root, ChildParentRef sided_subtree, Size key)
    {
        // [NTS]
        // side is the parent, and we're always searching through the right child
        // I need to draw this out a bit I think
        // we want side's parent? yes
        assert(sided_subtree.parent);
        assert(sided_subtree.child_pointer()->key() == key);
        auto & side = sided_subtree.child_pointer();
        auto & lc = side->m_left;
        auto & rc = side->m_right;
        using std::move;
        if (lc && rc) {
            // two node case
            // note: reference to pointer
            auto & suc = find_inorder_successor(rc, key);
            assert(suc);
            suc->swap_content(*side);
            // post swap, remove successor
            // the entire subtree is affected
            return BstRemoveRes{move(sided_subtree), move(root), move_ptr(suc)};
        }
        // one or none case
        auto rmvd = move_ptr(side);
        side = lc ? lc : rc;
        if (!side) return BstRemoveRes{move(root), move(rmvd)};

        // only one part of the sub tree is affected
        return BstRemoveRes{move(sided_subtree), move(root), move(rmvd)};
    }

public:
    // bst remove should be tested also
    static BstRemoveRes bst_remove(NodeOwningPtr root, Size key) {
        using std::move;
        auto has_key = [key](NodeInstance * node)
            { return node ? node->key() == key : false; };
        auto do_recur_on_side = [key, &root](NodeInstance *& side)
            { return bst_remove(move_ptr(side), key).append_to_super(move(root), side); };
        auto do_side_has_key = [key, &root](NodeInstance *& side)
            { return bst_remove_on_side(root, ChildParentRef{root.get(), side}, key); };

        if (!root) return BstRemoveRes{};
        if (has_key(root.get())) {
            // still pretty DRY
            auto & lc = root->m_left;
            auto & rc = root->m_right;
            if (lc && rc) {
                // two node case
                // note: reference to pointer
                auto & suc = find_inorder_successor(rc, key);
                assert(suc);
                suc->swap_content(*root);
                // post swap, remove successor
                // the entire subtree is affected
                return BstRemoveRes{move(root), move_ptr(suc)};
            }
            // one or none case
            return BstRemoveRes{move_ptr(lc ? lc : rc), move(root)};
        }
        if (has_key(root->m_left))
            { return do_side_has_key(root->m_left); }
        if (has_key(root->m_right))
            { return do_side_has_key(root->m_right); }
        if (root->m_left && key < root->key())
            { return do_recur_on_side(root->m_left); }
        if (root->m_right && key > root->key())
            { return do_recur_on_side(root->m_right); }
        return BstRemoveRes{move(root), nullptr};
    }

    static AvlRemoveRes avl_remove(NodeOwningPtr root, Size key) {
        using std::move;
        auto [affected, root_, removed_] = bst_remove(move(root), key); {}
        if (removed_ && root_ && affected.parent) {
            affected.child_pointer()->update_balance();
            // for rebalancing we must also the parent of the "affected" node
            // in house knowledge: the right side of the tree may become "lighter"
            auto subtree = move_ptr(affected.child_pointer());
            subtree = rebalance_left(subtree);
            affected.child_pointer() = subtree.release();
        }
        return AvlRemoveRes{move(root_), move(removed_)};
    }
public:
    template <typename Type>
    Tuple<NodeOwningPtr, Type *> split();

    Size key() const noexcept { return m_key; }

    int height() const;

    void update_balance();

    void * ptr_(Size sought) const;

    int balance() const { return m_balance; }

    void swap(NodeInstance & rhs) {
        using std::swap;
        swap(m_datum  , rhs.m_datum  );
        swap(m_key    , rhs.m_key    );
        swap(m_left   , rhs.m_left   );
        swap(m_right  , rhs.m_right  );
        swap(m_source , rhs.m_source );
        swap(m_balance, rhs.m_balance);
    }

    // invalidates balance
    void swap_content(NodeInstance & rhs) {
        using std::swap;
        swap(m_datum , rhs.m_datum );
        swap(m_key   , rhs.m_key   );
        swap(m_source, rhs.m_source);
    }

protected:
    NodeInstance(void * datum_, Size key_):
        m_datum(datum_), m_key(key_)
    { assert(key_ != Size(-1) && key_ != 0); }

private:
    void decrement_children() {
        // NodeInstance may own it's child nodes
        auto on_ = [](NodeInstance * side) {
            if (!side) return;
            side->decrement_children();
            assert(side->m_source);
            side->m_source->decrement(side->m_datum);
        };
        on_(m_left );
        on_(m_right);
    }

    // to rotate -> must rotatable at root

    // there's a *lot* of overhead...
    void * m_datum = nullptr;
    Size m_key = 0;
    NodeInstance * m_left = nullptr;
    NodeInstance * m_right = nullptr;
    // owner calls decrement...
    NodeSource * m_source = nullptr;
    int m_balance = 0;
};

NodeOwningPtr rebalance_right(NodeOwningPtr &);

NodeOwningPtr rebalance_left(NodeOwningPtr &);

NodeOwningPtr rotate_right(NodeOwningPtr);

NodeOwningPtr rotate_left(NodeOwningPtr);

template <typename T>
class SingleNode;

class NodeInstanceAttn {
    template <typename ... Types>
    friend class MultiNodeImpl;

    template <typename T>
    friend class SingleNode;

    friend struct NodeDeletor;

    friend NodeOwningPtr rebalance_right(NodeOwningPtr &);

    friend NodeOwningPtr rebalance_left(NodeOwningPtr &);

    friend NodeOwningPtr rotate_right(NodeOwningPtr);

    friend NodeOwningPtr rotate_left(NodeOwningPtr);

    static void set_source(NodeInstance & node, NodeSource * source)
        { node.m_source = source; }

    static void decrement(NodeInstance & node) {
        node.decrement_children();
        // node may become a dangling reference following this call...
        node.m_source->decrement(node.m_datum);
    }

    static NodeInstance * left_of(NodeInstance & node)
        { return node.m_left; }

    static NodeInstance * right_of(NodeInstance & node)
        { return node.m_right; }

    static void set_left_of(NodeInstance & node, NodeInstance * left)
        { node.m_left = left; }

    static void set_right_of(NodeInstance & node, NodeInstance * right)
        { node.m_right = right; }
};

template <typename ... Types>
class MultiNodeImpl;

template <typename Type>
class NodeForType final : public NodeInstance {
public:
    template <typename ... Types>
    NodeForType(Types && ... args):
        NodeInstance(&m_real_datum, MetaFunctions::key_for_type<Type>())
    { new (&m_real_datum) Type{std::forward<Types>(args)...}; }

    std::ptrdiff_t compare(void * ptr) const noexcept {
        using Byte = std::byte;
        auto as_byte = [](const void * p) { return reinterpret_cast<const Byte *>(p); };
        return as_byte(ptr) - as_byte(&m_real_datum);
    }

    void destruct(void * ptr) const noexcept {
        assert(ptr == &m_real_datum);
        reinterpret_cast<Type *>(ptr)->~Type();
    }

private:
    StorageFor<Type> m_real_datum;
};

template <typename ... Types>
class MultiNodeImpl : public NodeSource {
protected:
    void list_out_to(NodeOwningPtr *, NodeOwningPtr *) {}

    void set_source(NodeSource *) {}

    template <typename T>
    std::ptrdiff_t compare(void *) const noexcept
        { assert(!"Failed to compare for missing type."); }

    template <typename T>
    void destruct(void *) const noexcept
        { assert(!"Failed to destruct for missing type."); }
};

template <typename Head, typename ... Types>
class MultiNodeImpl<Head, Types...> : public MultiNodeImpl<Types...> {
protected:
    void list_out_to(NodeOwningPtr * beg, NodeOwningPtr * end);

    void set_source(NodeSource * source_);

    template <typename T>
    std::ptrdiff_t compare(void * ptr) const noexcept {
        // all branches should be elimated at runtime
        if constexpr (std::is_same_v<T, Head>) {
            return m_fullnode.compare(ptr);
        } else {
            return MultiNodeImpl<Types...>::template compare<T>(ptr);
        }
    }

    template <typename T>
    void destruct(void * datum) const noexcept {
        if constexpr (std::is_same_v<T, Head>) {
            m_fullnode.destruct(datum);
        } else {
            MultiNodeImpl<Types...>::template destruct<T>(datum);
        }
    }

private:
    NodeForType<Head> m_fullnode;
};

template <typename ... Types>
class MultiNode final : public MultiNodeImpl<Types...> {
public:
    // reclaimation of components?
    // just use another tree!
    void list_out_to(NodeOwningPtr * beg, NodeOwningPtr * end);

    void set_source(NodeSource * source_)
        { MultiNodeImpl<Types...>::set_source(source_); }

    // how can I go from datum pointer to an index? an offset?
    void decrement(void *) noexcept final;

    using VoidFunc = void(*)(void *);

private:
    // Why am I doing this, do I think it's a "neat" puzzle?!
    // What is wrong with me? :c

    template <typename ... OTypes>
    struct ReverseList final {
        using List = TypeList<>;
    };

    template <typename Head, typename ... OTypes>
    struct ReverseList<Head, OTypes...> final {
        using InheritedList = typename ReverseList<OTypes...>::List;
        using List = typename InheritedList::template CombineWith<Head>::Type;
    };

    template <typename TypeListType>
    using ReverseListOf = typename TypeListType::template DefineWithListTypes<ReverseList>::Type::List;

    template <std::size_t kt_idx, typename ... OTypes>
    struct TypeAtIdx final {
        using Type = void;
    };

    template <std::size_t kt_idx, typename Head, typename ... OTypes>
    struct TypeAtIdx<kt_idx, Head, OTypes...> final {
        using Type = std::conditional_t<kt_idx == 0, Head,
            typename TypeAtIdx<kt_idx - 1, OTypes...>::Type>;
    };

    // left side of types before reaching idx and excluding idx
    template <std::size_t kt_idx, std::size_t kt_mid, typename ... OTypes>
    struct TypesLeftOf final {
        using List = TypeList<>;
    };

    template <std::size_t kt_idx, std::size_t kt_mid, typename Head, typename ... OTypes>
    struct TypesLeftOf<kt_idx, kt_mid, Head, OTypes...> final {
        // hit zero, done adding types!
        using BaseList = typename TypesLeftOf<kt_idx + 1, kt_mid, OTypes...>::List;
        using ListWithHead = ReverseListOf<typename BaseList::template CombineWith<Head>::Type>;
        using List = std::conditional_t<(kt_idx < kt_mid),
            ListWithHead, BaseList>;
    };

    template <std::size_t kt_idx, std::size_t kt_mid, typename ... OTypes>
    struct TypesRightOf final {
        using List = TypeList<>;
    };

    template <std::size_t kt_idx, std::size_t kt_mid, typename Head, typename ... OTypes>
    struct TypesRightOf<kt_idx, kt_mid, Head, OTypes...> final {
        // hit zero, done adding types!
        using BaseList = typename TypesRightOf<kt_idx + 1, kt_mid, OTypes...>::List;
        using ListWithHead = ReverseListOf<typename BaseList::template CombineWith<Head>::Type>;
        using List = std::conditional_t<(kt_idx > kt_mid),
            ListWithHead, BaseList>;
    };

    template <typename T>
    std::ptrdiff_t compare(void * ptr) const noexcept
        { return MultiNodeImpl<Types...>::template compare<T>(ptr); }

    template <typename T>
    void destruct(void * datum) const noexcept
        { MultiNodeImpl<Types...>::template destruct<T>(datum); }

    template <typename ... OTypes>
    void destruct_impl_(TypeList<OTypes...>, void *)
        {  assert(!"Multinode of zero types call destruct!"); }

    template <typename Head, typename ... OTypes>
    void destruct_impl_(TypeList<Head, OTypes...>, void * datum) {
        // assumption... early type -> higher address
        static constexpr const std::size_t kt_mid_idx = (sizeof...(OTypes) + 1) / 2;
        using MidType = typename TypeAtIdx<kt_mid_idx, Head, OTypes...>::Type;
        using LeftList = typename TypesLeftOf<0, kt_mid_idx, Head, OTypes...>::List;
        using RightList = typename TypesRightOf<0, kt_mid_idx, Head, OTypes...>::List;
        auto diff = compare<MidType>(datum);
        if (diff == 0) {
            destruct<MidType>(datum);
        } else if (diff < 0) {
            // go right (actually)
            assert(RightList::k_count > 0);
            destruct_impl_(RightList{}, datum);
        } else {
            assert(LeftList::k_count > 0);
            destruct_impl_(LeftList{}, datum);
        }
    }

    int m_count = sizeof...(Types);
    NodeInstance * m_reuse_root = nullptr;
};

template <typename T>
class SingleNode final : public NodeSource {
public:
    template <typename ... Types>
    SingleNode(Types && ... args):
        m_real_node(std::forward<Types>(args)...)
    {}

    void set_source(NodeSource * source_)
        { NodeInstanceAttn::set_source(m_real_node, source_); }

    void decrement(void * datum) noexcept final {
        m_real_node.destruct(datum);
        delete this;
    }

    NodeInstance * node_pointer() noexcept { return &m_real_node; }

private:
    NodeForType<T> m_real_node;
};

class AvlTreeEntity;

class AvlTreeEntityBody final : public EntityBodyIntr<AvlTreeEntity> {
public:
    AvlTreeEntityBody() {}

    AvlTreeEntityBody(const AvlTreeEntityBody & body):
        Super(body) {}

    explicit AvlTreeEntityBody(HomeScene * home): Super(home) {}

    NodeOwningPtr root;

private:
    using Super = EntityBodyIntr<AvlTreeEntity>;
    const void * downcast_(Size safety_) const noexcept final {
        if (safety_ == get_safety()) return this;
        return nullptr;
    }
};

// ------------------------- Tree Node implementation -------------------------

inline void NodeDeletor::operator () (NodeInstance * inst) const noexcept
    { NodeInstanceAttn::decrement(*inst); }

template <typename ... Types>
std::array<NodeOwningPtr, sizeof...(Types)> make_multiple_type_nodes() {
    std::array<NodeOwningPtr, sizeof...(Types)> rv;
    // still no owner :c
    auto mnode = new MultiNode<Types...>{};
    mnode->list_out_to(&rv.front(), &rv.front() + rv.size());
    mnode->set_source(mnode);
    return rv;
}

template <typename Type, typename ... ConstructorTypes>
NodeOwningPtr make_single_type_node(ConstructorTypes && ... args) {
    auto node = new SingleNode<Type>{std::forward<ConstructorTypes>(args)...};
    node->set_source(node);
    return NodeOwningPtr{ node->node_pointer() };
}

// level 1 helpers (tree node implementations)

int height_of(NodeInstance *);

int height_of(const NodeOwningPtr &);

NodeOwningPtr move_ptr(NodeInstance *&);

// NodeInstance

inline /* static */ NodeInstance::AvlInsertRes NodeInstance::avl_insert
    (NodeOwningPtr root, NodeOwningPtr newnode)
{
    using AvlRes = NodeInstance::AvlInsertRes;
    using std::move;
    if (!root) return AvlRes{move(newnode), nullptr};

    auto nkey = newnode->key();
    auto rkey = root->key();

    if (nkey == rkey) return AvlRes{move(root), move(newnode)};

    bool should_go_left = nkey < rkey;
    auto & side = should_go_left ? root->m_left : root->m_right;
    if (!side) {
        // newnode is "consumed"
        side = newnode.release();
        root->update_balance();
        return AvlRes{move(root)};
    }

    // balance assumed updated post call
    assert(side);
    auto [nroot, nnode] = avl_insert(move_ptr(side), move(newnode));
    side = nroot.release();
    return AvlRes{
        should_go_left ? rebalance_left(root) : rebalance_right(root),
        move(nnode)};
}

inline int NodeInstance::height() const {
    if (!m_left && !m_right) return 1;
    if ( m_left &&  m_right)
        return std::max( m_left->height(), m_right->height() ) + 1;
    if (m_left) return m_left->height() + 1;
    return m_right->height() + 1;
}

inline void NodeInstance::update_balance() {
    m_balance =   (m_left  ? m_left ->height() : 0)
                - (m_right ? m_right->height() : 0);
}

inline void * NodeInstance::ptr_(Size sought) const {
    if (m_key == sought) return m_datum;
    if (m_key > sought && m_left ) return m_left ->ptr_(sought);
    if (m_key < sought && m_right) return m_right->ptr_(sought);
    return nullptr;
}

// Helpers

inline NodeOwningPtr rebalance_right(NodeOwningPtr & ptr) {
    // the beauty of this is that it...
    // if it is already balanced... no problem
    // if not, it's fixed at the end
    using Nia = NodeInstanceAttn;
    using std::move;
    assert(ptr);
    assert(Nia::right_of(*ptr));
    ptr->update_balance();
    if (ptr->balance() != -2) return move(ptr);
    auto right = Nia::right_of(*ptr);
    if (right->balance() > 0)
        Nia::set_right_of(*ptr, rotate_right(move_ptr(right)).release());
    else
        assert(right->balance() < 0);
    return rotate_left(move(ptr));
}

inline NodeOwningPtr rebalance_left(NodeOwningPtr & ptr) {
    using Nia = NodeInstanceAttn;
    using std::move;
    assert(ptr);
    assert(Nia::left_of(*ptr));
    ptr->update_balance(); // some kind of update or whatever...
    // balance is height_of(left) - height_of(right)
    if (ptr->balance() != 2) return move(ptr);
    // is it a line or kink case?
    // done on kink only
    auto left = Nia::left_of(*ptr);
    if (left->balance() < 0)
        Nia::set_left_of(*ptr, rotate_left(move_ptr(left)).release());
    else
        assert(left->balance() > 0);
    return rotate_right(move(ptr));
}

inline NodeOwningPtr rotate_right(NodeOwningPtr ptr) {
    using Nia = NodeInstanceAttn;
    auto t  = ptr.release();
    auto l  = Nia::left_of (*t);
    auto lr = Nia::right_of(*l);

    Nia::set_right_of(*l, t );
    Nia::set_left_of (*t, lr);

    t->update_balance();
    l->update_balance();

    // l must own the other two!
    assert(l->ptr_(t ->key()));
    assert(lr ? !!l->ptr_(lr->key()) : true);

    return NodeOwningPtr{l};
}

inline NodeOwningPtr rotate_left(NodeOwningPtr ptr) {
    using Nia = NodeInstanceAttn;
    auto t  = ptr.release();
    auto r  = Nia::right_of(*t);
    auto rl = Nia::left_of (*r);

    Nia::set_left_of (*r, t);
    Nia::set_right_of(*t, rl);

    t->update_balance();
    r->update_balance();

    assert(r->ptr_(t ->key()));
    assert(rl ? !!r->ptr_(rl->key()) : true);

    return NodeOwningPtr{r};
}

// More helper types

template <typename Head, typename ... Types>
/* protected */ void MultiNodeImpl<Head, Types...>::
    list_out_to(NodeOwningPtr * beg, NodeOwningPtr * end)
{
    assert(beg != end);
    beg->reset(&m_fullnode);
    MultiNodeImpl<Types...>::list_out_to(beg + 1, end);
}

template <typename Head, typename ... Types>
/* protected */ void MultiNodeImpl<Head, Types...>::set_source
    (NodeSource * source_)
{
    NodeInstanceAttn::set_source(m_fullnode, source_);
    MultiNodeImpl<Types...>::set_source(source_);
}


template <typename ... Types>
void MultiNode<Types...>::list_out_to(NodeOwningPtr * beg, NodeOwningPtr * end) {
    assert(end - beg == sizeof...(Types));
    MultiNodeImpl<Types...>::list_out_to(beg, end);
}

template <typename ... Types>
void MultiNode<Types...>::decrement(void * datum) noexcept {
    destruct_impl_(TypeList<Types...>{}, datum);
    if (--m_count) return;
    // I've... lost it
    delete this;
}

// level 1 helpers (tree node implementations) impl

inline int height_of(NodeInstance * ptr)
    { return ptr ? ptr->height() : 0; }

inline int height_of(const NodeOwningPtr & ptr)
    { return ptr ? ptr->height() : 0; }

inline NodeOwningPtr move_ptr(NodeInstance *& ptrref) {
#   if 0
    assert(ptrref);
#   endif
    auto t = ptrref;
    ptrref = nullptr;
    return NodeOwningPtr{t};
}

} // end of ecs namespace
