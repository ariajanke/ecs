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
#include <ariajanke/ecs3/detail/HashMap.hpp>

#include <memory>

namespace ecs {

/** A hash table for differently, and uniquely typed objects.
 *
 *  This is used to implement entity types, as a means to store, retreive, and
 *  remove components.
 *
 *  @warning This class contains a lot of unsafe code.
 */
class HeterogeneousHashTable final {
public:
    HeterogeneousHashTable() {}

    HeterogeneousHashTable(const HeterogeneousHashTable &) = delete;

    HeterogeneousHashTable(HeterogeneousHashTable &&) = delete;

    ~HeterogeneousHashTable() { remove_all(); }

    HeterogeneousHashTable & operator = (const HeterogeneousHashTable &) = delete;

    HeterogeneousHashTable & operator = (HeterogeneousHashTable &&) = delete;

    template <typename Type, typename ... ArgTypes>
    Type & append(ArgTypes &&... args);

    template <typename Type>
    bool remove();

    void remove_all();

    template <typename Type>
    Type * get() const;

    template <typename ... Types>
    void reserve_for_more(TypeList<Types...>);

    // detail

    struct EmptyKeyMaker final { Size operator () () const { return 0; } };

    using ComponentTable = UnowningHashMap<
        Size,
        Tuple<void *, const MetaFunctions *>,
        std::hash<Size>,
        std::equal_to<void>,
        EmptyKeyMaker>;
    using BucketSpace    = ComponentTable::BucketSpace;
    using Byte           = std::byte;

    class Storage final {
    public:
        /// This is the minimum unit space allocated in memory for components.
        ///
        /// @note This definition is more implementation detail than interface
        ///       (like the class in general) and should be not referenced by
        ///       the client.
        static constexpr const auto k_min_space_for_components = sizeof(std::max_align_t);

        Storage() {}

        Storage(const Storage &) = delete;

        Storage(Storage &&) = default;

        Storage & operator = (const Storage &) = delete;

        Storage & operator = (Storage &&) = default;

        ~Storage();

        static Storage make_new(Size bucket_count, Size for_components);

        Storage make_new_without_lost() const;

        BucketSpace get_bucket_space() const;

        void * next_component_space(Size align, Size size);

        void mark_lost_bytes(Size lost);

        Size lost_space() const { return m_lost; }

        Size total_space() const
            { return Size(m_end - m_buckets_end); }

        Size available_space(Size align) const
            { return std::get<0>(available_space_and_start(align)); }

        Size used_space() const;

        void swap(Storage & rhs);

        void wipe_component_space();

    private:
        using TablePair = ComponentTable::value_type;
        template <typename Func>
        void for_each_bucket_space(Func && f);

        static Size size_in_max_aligns(Size sz_bytes);

        Size get_jump_by(Size align) const;

        Tuple<Size, void *> available_space_and_start(Size align) const;

        Byte * m_buckets_end = nullptr;
        Byte * m_comps_end = nullptr;
        Byte * m_end = nullptr;
        Size m_lost = 0;
        // base pointer *will* be max aligned
        std::unique_ptr<Byte[]> m_storage_space = nullptr;
    };

private:

    template <typename T>
    static const MetaFunctions & metafunctions_for()
        { return MetaFunctions::for_type<T>(); }

    // this is a living space for two data structures...
    // first : a sparse hash table for pointers to a tuple for each "space"
    // second: a tuple of potentially differently sized spaces where
    // component table elements may live
    //
    // there's no pretty way to implement this, even with well defined
    // structures, I'd have to use templates upon templates, defining whole
    // tables of functions for various sizes structures
    //
    // things that I need to implement with the hash table:
    // reservation of space
    // moving elements


    // table layout
    // entries on hashmap:
    // key -> (component ptr, meta functions ptr)

    // have special conditions for realloc been met? if so do it
    void check_to_realloc();

    void move_to(Storage && new_store);

    template <typename Head, typename ... Types>
    void reserve_for_more_(TypeList<Head, Types...>, Size size, Size align, Size count);

    template <typename ... Types>
    void reserve_for_more_(TypeList<Types...>, Size size, Size align, Size count);

    Storage m_storage;
    ComponentTable m_table = ComponentTable{BucketSpace{}};
};

class HashTableEntity;

class HashTableEntityBody final : public EntityBodyIntr<HashTableEntity> {
public:
    // DRY violation!!

    HashTableEntityBody() {}

    HashTableEntityBody(const HashTableEntityBody & body):
        Super(body) {}

    explicit HashTableEntityBody(HomeScene * home): Super(home) {}

    HeterogeneousHashTable table;
private:
    using Super = EntityBodyIntr<HashTableEntity>;
    const void * downcast_(Size safety_) const noexcept final {
        if (safety_ == get_safety()) return this;
        return nullptr;
    }
};

class ConstHashTableEntity;

// -------------------------- HeterogeneousHashTable --------------------------

template <typename Type, typename ... ArgTypes>
Type & HeterogeneousHashTable::append(ArgTypes &&... args) {
    if (get<Type>()) {
        throw std::runtime_error("");
    }
    auto next = [this]()
        { return m_storage.next_component_space(alignof(Type), sizeof(Type)); };
    auto ptr = next();
    if (!ptr || !m_table.can_fit_another()) {
        if (ptr) m_storage.mark_lost_bytes(sizeof(Type));
        // move to new store
        // set ptr to "next" in new store
        move_to(Storage::make_new(m_table.bucket_count()*2 + 1,
                                  m_storage.used_space()*2 + sizeof(Type)));
        assert(m_table.can_fit_another());
        ptr = next();
    }
    const auto & mf = metafunctions_for<Type>();
    auto rv = new (ptr) Type(std::forward<ArgTypes>(args)...);
    m_table.emplace(mf.key(), std::make_tuple(rv, &mf));
    return *rv;
}

template <typename Type>
bool HeterogeneousHashTable::remove() {
    const auto & mf = metafunctions_for<Type>();
    auto itr = m_table.find(mf.key());
    if (itr == m_table.end()) return false;
    assert(std::get<const MetaFunctions *>(itr->second) == &mf);
    m_table.erase_no_preserve_iterators(itr);
    // no real effect(??)
    m_storage.mark_lost_bytes(mf.object_size());

    mf.destroy(std::get<void *>(itr->second));
    return true;
}

inline void HeterogeneousHashTable::remove_all() {
    for (auto entry : m_table) {
        auto [ptr, mf] = entry.second;
        mf->destroy(ptr);
    }
    m_storage.wipe_component_space();
    check_to_realloc();
    m_table.clear();
}

template <typename Type>
Type * HeterogeneousHashTable::get() const {
    auto itr = m_table.find(metafunctions_for<Type>().key());
    if (itr == m_table.end()) return nullptr;
    return reinterpret_cast<Type *>(std::get<void *>(itr->second));
}

template <typename ... Types>
void HeterogeneousHashTable::reserve_for_more(TypeList<Types...>)
    { reserve_for_more_(TypeList<Types...>{}, 0, 0, sizeof...(Types)); }

/* private */ inline void HeterogeneousHashTable::check_to_realloc() {
    bool should_realloc = m_storage.lost_space()*3 > m_storage.total_space();
    if (!should_realloc) return;
    move_to(m_storage.make_new_without_lost());
}

/* private */ inline void HeterogeneousHashTable::move_to
    (Storage && new_store)
{
    ComponentTable new_table{new_store.get_bucket_space()};
    for (auto entry : m_table) {
        auto [ptr, mf] = entry.second;
        auto new_space = new_store.next_component_space
            (mf->object_align(), mf->object_size());
        // oh my!
        new_table.emplace(mf->key(), std::make_tuple(mf->move(ptr, new_space), mf));
        // have to destroy the old
        mf->destroy(ptr);
    }
    m_table.swap(new_table);
    m_storage.swap(new_store);
}

template <typename Head, typename ... Types>
/* private */ void HeterogeneousHashTable::reserve_for_more_
    (TypeList<Head, Types...>, Size size, Size align, Size count)
{
    assert(!get<Head>());
    reserve_for_more_(
        TypeList<Types...>{}, size + sizeof(Head),
        std::max(align, alignof(Head)), count);
}

template <typename ... Types>
/* private */ void HeterogeneousHashTable::reserve_for_more_
    (TypeList<Types...>, Size size, Size align, Size count)
{
    if (   size  < m_storage.available_space(align)
        && count < m_table.bucket_count() - m_table.size())
    { return; }
    move_to(Storage::make_new(
        count + m_table.size(), size + m_storage.used_space()));
}


// --------------------- HeterogeneousHashTable::Storage ----------------------

inline HeterogeneousHashTable::Storage::~Storage() {
    for_each_bucket_space([](void * occupied) {
        reinterpret_cast<TablePair *>(occupied)->~TablePair();
    });
}

template <typename Func>
/* private */ void HeterogeneousHashTable::Storage::for_each_bucket_space
    (Func && f)
{
    for (auto * itr = reinterpret_cast<TablePair *>(m_storage_space.get());
         itr != reinterpret_cast<const TablePair *>(m_buckets_end);
         ++itr)
    {
        // despite the pointer type... nothing lives there!
        void * ptr = itr;
        f(ptr);
    }
}
/* static private */ inline Size HeterogeneousHashTable::Storage::size_in_max_aligns
    (Size sz_bytes)
{
    static constexpr const auto k_max_align = sizeof(std::max_align_t);
    return sz_bytes / k_max_align + ((sz_bytes % k_max_align) ? 1 : 0);
}

/* static */ inline HeterogeneousHashTable::Storage
    HeterogeneousHashTable::Storage::make_new
    (Size component_count, Size for_components)
{
    Storage rv;
    using MaxAlign = std::max_align_t;
    static constexpr const auto k_min_space = k_min_space_for_components;
    static_assert(sizeof(MaxAlign) == k_min_space);

    auto bucket_count = ComponentTable::BucketSpace::high_power_of_2(component_count*2);
    auto mas_for_buckets = size_in_max_aligns(bucket_count*sizeof(TablePair));
    auto mas_for_comps = size_in_max_aligns(for_components);
    rv.m_storage_space.reset(
        reinterpret_cast<Byte *>(new MaxAlign[mas_for_buckets + mas_for_comps]));
    Byte * begin = rv.m_storage_space.get();
    rv.m_buckets_end = reinterpret_cast<Byte *>(
        reinterpret_cast<TablePair *>(begin) + bucket_count);
    rv.m_comps_end   = begin + mas_for_buckets*k_min_space;
    rv.m_end         = rv.m_comps_end + mas_for_comps*k_min_space;
    assert(rv.m_end >= rv.m_comps_end);
    assert(rv.m_buckets_end >= begin);
    assert(Size(rv.m_buckets_end - begin) >= bucket_count*sizeof(TablePair));
    assert(Size(rv.m_end - rv.m_buckets_end) >= for_components);
    assert([&rv] { (void)rv.get_bucket_space(); return true; } ());
    // make sure valid buckets live here
    rv.for_each_bucket_space([](void * space) {
        using Ekm = ComponentTable::EmptyKeyMaker;
        new (space) TablePair{ Ekm{}(), std::make_tuple(nullptr, nullptr) };
    });
#   if 0
    if constexpr (k_report_allocations) {
        using std::cout;
        cout << "New space created for " << bucket_count
             << " buckets and " << for_components << " bytes for components."
             << std::endl;
    }
#   endif
    return rv;
}

inline HeterogeneousHashTable::Storage
    HeterogeneousHashTable::Storage::make_new_without_lost() const
{
    auto bucks = get_bucket_space();
    auto bend = reinterpret_cast<TablePair *>(bucks.end);
    auto bbeg = reinterpret_cast<TablePair *>(bucks.begin);
    assert(bend - bbeg);
#   if 0
    if constexpr (k_report_allocations) {
        using std::cout;
        cout << "Without lost bytes...";
    }
#   endif
    return make_new( bend - bbeg, used_space() );
}

inline HeterogeneousHashTable::BucketSpace
    HeterogeneousHashTable::Storage::get_bucket_space() const
{
    auto to_p = [](void * ptr) { return reinterpret_cast<TablePair *>(ptr); };
    if (!m_storage_space) return BucketSpace{};
    return BucketSpace{to_p(m_storage_space.get()), to_p(m_buckets_end)};
}

/* private */ inline Size HeterogeneousHashTable::Storage::get_jump_by
    (Size align) const
    { return (align - ((m_comps_end - m_buckets_end) % align)) % align; }

/* private */ inline Tuple<Size, void *>
    HeterogeneousHashTable::Storage::available_space_and_start(Size align) const
{
    // jump ahead for alignment
    // example: align of 8 but rem 3, jump ahead 5
    auto jump_by = get_jump_by(align);
    Size left    = m_end - m_comps_end;
    if (jump_by >= left) return std::make_tuple(0, nullptr);
    assert(m_end > m_comps_end);
    auto start = m_comps_end + jump_by;
    assert(m_end > start);
    return std::make_tuple(m_end - start, start);
}

inline void * HeterogeneousHashTable::Storage::next_component_space
    (Size align, Size size)
{
    if (!available_space(align)) return nullptr;
    auto [left, start] = available_space_and_start(align);
    if (left >= size) {
        m_comps_end = reinterpret_cast<Byte *>(start) + size;
        assert(start);
        return start;
    }
    return nullptr;
}

inline void HeterogeneousHashTable::Storage::mark_lost_bytes(Size lost) {
    // cannot lose more than used
    assert(m_comps_end >= m_buckets_end);
    assert(lost <= used_space());
    m_lost += lost;
}

inline Size HeterogeneousHashTable::Storage::used_space() const {
    auto bs = get_bucket_space();
    const auto * start = m_storage_space.get()
        + size_in_max_aligns((bs.end - bs.begin)*sizeof(TablePair))
          *sizeof(std::max_align_t);
    // need to not get lost in the padding
    return (m_comps_end - start) - m_lost;
}

inline void HeterogeneousHashTable::Storage::swap(Storage & rhs) {
    using std::swap;
    swap(m_buckets_end, rhs.m_buckets_end);
    swap(m_comps_end  , rhs.m_comps_end  );
    swap(m_end        , rhs.m_end        );
    swap(m_lost       , rhs.m_lost       );
    m_storage_space.swap(rhs.m_storage_space);
}

inline void HeterogeneousHashTable::Storage::wipe_component_space() {
    m_comps_end = m_storage_space.get();
    m_lost = 0;
}

} // end of ecs namespace
