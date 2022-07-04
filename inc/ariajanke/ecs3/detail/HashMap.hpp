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

// This is a derivitive of Erik Rigtorp's work. This HashMap depends on
// the client to provide where to store its elements.
//
// Unlike the original it's not meant to be a drop in replacement for a STL
// container.
//
// Much like the original, this too is is released under the MIT license.
//
// © 2017-2020 Erik Rigtorp <erik@rigtorp.se>
// SPDX-License-Identifier: MIT
/*
HashMap

A high performance hash map. Uses open addressing with linear
probing.

Advantages:
  - Predictable performance. Doesn't use the allocator unless load factor
    grows beyond 50%. Linear probing ensures cash efficency.
  - Deletes items by rearranging items and marking slots as empty instead of
    marking items as deleted. This is keeps performance high when there
    is a high rate of churn (many paired inserts and deletes) since otherwise
    most slots would be marked deleted and probing would end up scanning
    most of the table.

Disadvantages:
  - Significant performance degradation at high load factors.
  - Maximum load factor hard coded to 50%, memory inefficient.
  - Memory is not reclaimed on erase.
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

template <typename T>
class DefaultEmptyKeyMaker {
public:
    // default constructed object assumed as empty key
    T operator () () const { return T(); }
};

template <typename Key, typename T, typename Hash = std::hash<Key>,
          typename KeyEqual = std::equal_to<void>,
          typename EmptyKeyMakerT = DefaultEmptyKeyMaker<Key>>
class UnowningHashMap {
public:
    using ElementPair = std::pair<Key, T>;
    using EmptyKeyMaker = EmptyKeyMakerT;

    // types for STL
    using key_type        = Key;
    using mapped_type     = T;
    using value_type      = ElementPair;
    using size_type       = std::size_t;
    using hasher          = Hash;
    using key_equal       = KeyEqual;
    using reference       = value_type &;
    using const_reference = const value_type &;

    /// This important data structure tells the container where it can store
    /// elements and how many it can store.
    ///
    /// If size runs out, the client will need to provide new spaces
    struct BucketSpace final {
        static constexpr const int k_min_space = 2;
        static bool is_power_of_2(std::size_t s) {
            if (s == 0) return true;

            auto t = (s & 0x1) == 0;
            if (t && s >> 1 == 1) return true;
            return t && is_power_of_2(s >> 1);
        }
        static constexpr std::size_t high_power_of_2(std::size_t s) {
            std::size_t rv = 2;
            while (rv < s) { rv <<= 1; }
            return rv;
        }
        static_assert(high_power_of_2(4) == 4);
        static_assert(high_power_of_2(3) == 4);
        static_assert(high_power_of_2(5) == 8);
        BucketSpace() {}
        BucketSpace(value_type * begin_, value_type * space_end_):
            begin(begin_), end(space_end_) // bucket count starts at 0
            //,space_end(space_end_)
        {
            assert((!begin_  && !space_end_) || space_end_ - begin_ >= k_min_space);
            assert(is_power_of_2(space_end_ - begin_));
        }
        value_type * begin = nullptr;
        value_type * end   = nullptr;
    };

    template <bool kt_is_const>
    struct IteratorImpl;

    using iterator       = IteratorImpl<false>;
    using const_iterator = IteratorImpl<true>;

    explicit UnowningHashMap(BucketSpace &&);

    UnowningHashMap(UnowningHashMap &&, BucketSpace &&);

public:
    // ------------------------------ Iterators -------------------------------

    iterator begin() noexcept { return iterator{this}; }

    const_iterator begin() const noexcept { return cbegin(); }

    const_iterator cbegin() const noexcept { return const_iterator{this}; }

    iterator end() noexcept { return iterator{this, bucket_count()}; }

    const_iterator end() const noexcept { return cend(); }

    const_iterator cend() const noexcept
        { return const_iterator{this, bucket_count()}; }

    // ------------------------------- Capacity -------------------------------

    bool empty() const noexcept { return size() == 0; }

    size_type size() const noexcept { return m_size; }

    size_type max_size() const noexcept
        { return (m_buckets.space_end - m_buckets.begin) / 2; }

    bool can_fit_another() const noexcept;

    bool can_fit_this_many(size_type) const noexcept;

    // ------------------------------ Modifiers -------------------------------

    void clear() noexcept;

    std::pair<iterator, bool> insert(const value_type & value)
        { return emplace_impl(value.first, value.second); }

    std::pair<iterator, bool> insert(value_type && value)
        { return emplace_impl(value.first, std::move(value.second)); }

    template <typename... Args>
    std::pair<iterator, bool> emplace(Args &&... args)
        { return emplace_impl(std::forward<Args>(args)...); }

    /// Removes the element pointed to by "it".
    ///
    /// @warning This method still invalidates all iterators. (Though the
    /// present implementation only invalidates iterators previous to "it". It
    /// is best left treated as invalidating all of them.)
    /// @returns an iterator pointing to the element after the erase element
    iterator erase(iterator it) { return erase_impl<true>(it); }

    /// Removes the element pointed to by "it", this method invalidates all
    /// iterators.
    void erase_no_preserve_iterators(iterator it)
        { (void)erase_impl<false>(it); }

    size_type erase(const key_type & key) { return erase_impl(key); }

    template <typename K> size_type erase(const K & x) { return erase_impl(x); }

    void swap(UnowningHashMap & other) noexcept;

public:

    // -------------------------------- Lookup --------------------------------

    mapped_type & at(const key_type & key) { return at_impl(key); }

    template <typename K> mapped_type & at(const K & x) { return at_impl(x); }

    const mapped_type & at(const key_type & key) const { return at_impl(key); }

    template <typename K> const mapped_type & at(const K & x) const
        { return at_impl(x); }

    mapped_type & operator [] (const key_type & key)
        { return emplace_impl(key).first->second; }

    size_type count(const key_type & key) const { return count_impl(key); }

    template <typename K> size_type count(const K & x) const
        { return count_impl(x); }

    iterator find(const key_type & key) { return find_impl(key); }

    template <typename K> iterator find(const K & x) { return find_impl(x); }

    const_iterator find(const key_type & key) const { return find_impl(key); }

    template <typename K> const_iterator find(const K & x) const
        { return find_impl(x); }

    // --------------------------- Bucket interface ---------------------------

    size_type bucket_count() const noexcept
        { return size_type(m_buckets.end - m_buckets.begin); }

    // ----------------------------- Hash policy ------------------------------

    void rehash(BucketSpace && buckets) {
        UnowningHashMap other{*this, std::move(buckets)};
        swap(other);
    }

    // removes buckets (including empty ones)
    // This renders the map empty
    BucketSpace strip_buckets() {
        auto t = m_buckets;
        m_buckets = BucketSpace{};
        m_size    = 0;
        return t;
    }

    // ------------------------------ Observers -------------------------------

    hasher hash_function() const { return hasher{}; }

    key_equal key_eq() const { return key_equal{}; }

public:
    template <bool kt_is_const>
    struct IteratorImpl {
        using difference_type = std::ptrdiff_t;
        using value_type = std::conditional_t<kt_is_const, const ElementPair, ElementPair>;//  IterVal;
        using pointer = value_type *;
        using reference = value_type &;
        using iterator_category = std::forward_iterator_tag;

        bool operator == (const IteratorImpl & other) const
            { return is_equal_to(other); }

        bool operator != (const IteratorImpl & other) const
            { return !is_equal_to(other); }

        IteratorImpl & operator ++ ();

        reference operator * () const { return hm_->bucket_at(idx_); }

        pointer operator -> () const { return &hm_->bucket_at(idx_); }

    private:
        using Container = std::conditional_t<kt_is_const, const UnowningHashMap, UnowningHashMap>;
        using size_type = typename Container::size_type;
        bool is_equal_to(const IteratorImpl & other) const
            { return other.hm_ == hm_ && other.idx_ == idx_; }

        explicit IteratorImpl(Container * hm): hm_(hm) { advance_past_empty(); }

        explicit IteratorImpl(Container * hm, size_type idx): hm_(hm), idx_(idx) {}

        template <bool kt_other_is_const>
        IteratorImpl(const IteratorImpl<kt_other_is_const> & other):
            hm_(other.hm_), idx_(other.idx_) {}

        void advance_past_empty();

        Container * hm_ = nullptr;
        size_type idx_ = 0;
        friend Container;
    };

    // ------------------------ Public Interface Ends -------------------------

private:
    value_type & bucket_at(std::size_t idx) { return m_buckets.begin[idx]; }

    const value_type & bucket_at(std::size_t idx) const
        { return m_buckets.begin[idx]; }

    template <typename K, typename... Args>
    std::pair<iterator, bool> emplace_impl(const K & key, Args &&... args) {
        assert(!key_equal{}(EmptyKeyMaker{}(), key) && "empty key shouldn't be used");
        if (!can_fit_another()) {
            throw std::runtime_error("Cannot emplace new element, out of room.");
        }
        for (size_t idx = key_to_idx(key);; idx = probe_next(idx)) {
            if (key_equal{}(bucket_at(idx).first, EmptyKeyMaker{}())) {
                bucket_at(idx).second = mapped_type(std::forward<Args>(args)...);
                bucket_at(idx).first = key;
                m_size++;
                return {iterator(this, idx), true };
            } else if (key_equal{}(bucket_at(idx).first, key)) {
                return {iterator(this, idx), false};
            }
        }
    }

    template <bool kt_preserve_iterators>
    iterator erase_impl(iterator it) {
        size_t bucket = it.idx_;
        for (size_t idx = probe_next(bucket);; idx = probe_next(idx)) {
            if (   key_equal{}(bucket_at(idx).first, EmptyKeyMaker{}())
                || (kt_preserve_iterators && idx < bucket))
            {
                bucket_at(bucket).first = EmptyKeyMaker{}();
                m_size--;
                if constexpr (kt_preserve_iterators) {
                    it.advance_past_empty();
                    return it;
                } else {
                    // hopefully an optimizing compiler will just skip the call
                    // but is provided to make the code valid
                    return end();
                }
            }
            size_t ideal = key_to_idx(bucket_at(idx).first);
            if (diff(bucket, ideal) < diff(idx, ideal)) {
                // swap, bucket is closer to ideal than idx
                bucket_at(bucket) = bucket_at(idx);
                bucket = idx;
            }
        }
    }

    template <typename K> size_type erase_impl(const K & key) {
        auto it = find_impl(key);
        if (it == end()) { return 0; }
        erase_impl<false>(it);
        return 1;
    }

    template <typename K> mapped_type & at_impl(const K & key) {
        const auto * const_this = this;
        return const_this->at_impl(key);
    }

    template <typename K> const mapped_type & at_impl(const K & key) const {
        auto it = find_impl(key);
        if (it != end()) { return it->second; }
        throw std::out_of_range("HashMap::at");
    }

    template <typename K> size_t count_impl(const K & key) const
        { return find_impl(key) == end() ? 0 : 1; }

    template <typename K> iterator find_impl(const K &key) {
        assert(!key_equal{}(EmptyKeyMaker{}(), key) && "empty key shouldn't be used");
        // I hate adding a branch, but little choice
        if (size() == 0) return end();
        // not good enough
        for (size_t idx = key_to_idx(key);; idx = probe_next(idx)) {
            if (key_equal{}(bucket_at(idx).first, key))
                { return iterator(this, idx); }
            if (key_equal{}(bucket_at(idx).first, EmptyKeyMaker{}()))
                { return end(); }
        }
    }

    template <typename K> const_iterator find_impl(const K & key) const
        { return const_cast<UnowningHashMap *>(this)->find_impl(key); }

    template <typename K>
    size_t key_to_idx(const K &key) const noexcept(noexcept(hasher()(key)))
        { return hasher{}(key) & index_mask(); }

    size_t probe_next(size_t idx) const noexcept
        { return (idx + 1) & index_mask(); }

    size_t diff(size_t a, size_t b) const noexcept
        { return (bucket_count() + (a - b)) & index_mask(); }

    size_t index_mask() const noexcept
        { return (bucket_count() - 1)*bool(bucket_count()); }

    void check_invariants() const noexcept {
        // assert(bucket_count() >= 1); // must be at least one
    }

private:
    BucketSpace m_buckets;
    size_t m_size = 0;
};

// ----------------------------------------------------------------------------

#define MACRO_CLASS_PREFACE UnowningHashMap<Key, T, Hash, KeyEqual, EmptyKeyMaker>

template <typename Key, typename T, typename Hash, typename KeyEqual,
          typename EmptyKeyMaker>
/* explicit */ MACRO_CLASS_PREFACE::UnowningHashMap(BucketSpace && buckets):
    m_buckets(std::move(buckets))
{ check_invariants(); }

template <typename Key, typename T, typename Hash, typename KeyEqual,
          typename EmptyKeyMaker>
MACRO_CLASS_PREFACE
    ::UnowningHashMap(UnowningHashMap && other, BucketSpace && buckets):
    m_buckets(std::move(buckets))
{
    if (!can_fit_this_many(other.size())) {
        throw std::invalid_argument("HashMap::HashMap: not enough room was given for the hash map.");
    }
    for (auto & val : other) {
        emplace(val.first, std::move(val.second));
    }
    other.clear();
    check_invariants();
}

template <typename Key, typename T, typename Hash, typename KeyEqual,
          typename EmptyKeyMaker>
bool MACRO_CLASS_PREFACE::can_fit_another() const noexcept
    { return can_fit_this_many(size() + 1); }

template <typename Key, typename T, typename Hash, typename KeyEqual,
          typename EmptyKeyMaker>
bool MACRO_CLASS_PREFACE::can_fit_this_many(size_type amount) const noexcept
    { return amount*2 <= bucket_count(); }

template <typename Key, typename T, typename Hash, typename KeyEqual,
          typename EmptyKeyMaker>
void MACRO_CLASS_PREFACE::clear() noexcept {
    for (auto itr = m_buckets.begin; itr != m_buckets.end; ++itr) {
        itr->first = EmptyKeyMaker{}();
    }
    m_buckets.end = m_buckets.begin;
}

template <typename Key, typename T, typename Hash, typename KeyEqual,
          typename EmptyKeyMaker>
void MACRO_CLASS_PREFACE::swap(UnowningHashMap & other) noexcept {
    std::swap(m_buckets, other.m_buckets);
    std::swap(m_size   , other.m_size   );
}

// ----------------------------------------------------------------------------

template <typename Key, typename T, typename Hash, typename KeyEqual,
          typename EmptyKeyMaker>
template <bool kt_is_const>
    typename MACRO_CLASS_PREFACE::template IteratorImpl<kt_is_const> &
    MACRO_CLASS_PREFACE::IteratorImpl<kt_is_const>
    ::operator ++ () {
    ++idx_;
    advance_past_empty();
    return *this;
}

template <typename Key, typename T, typename Hash, typename KeyEqual,
          typename EmptyKeyMaker>
template <bool kt_is_const>
/* private */ void MACRO_CLASS_PREFACE::IteratorImpl<kt_is_const>::
    advance_past_empty()
{
    while (   idx_ < hm_->bucket_count()
           && key_equal{}(hm_->bucket_at(idx_).first, EmptyKeyMaker{}()/* hm_->m_empty_key*/))
    { ++idx_; }
}

#undef MACRO_CLASS_PREFACE
