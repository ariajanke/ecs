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

#include <common/TypeList.hpp>

namespace ecs {

using cul::TypeList;
using cul::TypeTag;

struct InlinedComponent {};

struct TrueType
    { static constexpr const bool k_value = true; };

struct FalseType
    { static constexpr const bool k_value = false; };

template <typename T>
struct DefineWouldInline : public FalseType {};

namespace detail {

struct ReferenceCounter;

struct ReferenceManager {
    static ReferenceManager & null_instance();

    virtual ~ReferenceManager() {}
    // fixed for vocab limiting... "create" means "unmanaged pointer"
    virtual ReferenceCounter * create_identity() = 0;
};

struct ReferenceCounter {
    virtual ~ReferenceCounter() {}

    int count = 0;
    bool requesting_deletion = false;
    bool expired = false;
    // needed for copying
    ReferenceManager * reference_manager = nullptr;

    using IdFuncPtr = void (*)();
    virtual void * full_downcast(IdFuncPtr) noexcept = 0;

protected:
    ReferenceCounter() {}
};

void decrement(ReferenceCounter *);

void increment(ReferenceCounter *);

// ----------------------------------------------------------------------------

template <typename ... Types>
struct ForTypes {
    template <typename T>
    struct HasConstOf
        { static const constexpr bool value = false; };

    template <typename T>
    struct Has {
        static const constexpr bool value = false;
        static const constexpr bool or_const_of = false;
    };

    template <typename ... OtherTypes>
    struct IsSuperSetOf
        { static const constexpr bool value = true; };
};

template <typename Head, typename ... Types>
struct ForTypes<Head, Types...> : public ForTypes<Types...> {
    template <typename T>
    struct HasConstOf {
        static const constexpr bool value =
            std::is_same<const Head, T>::value ||
            ForTypes<Types...>::template HasConstOf<T>::value;
    };

    template <typename T>
    struct Has {
        static const constexpr bool value =
            std::is_same<Head, T>::value ||
            ForTypes<Types...>::template Has<T>::value;
        static const constexpr bool or_const_of =
            std::is_same<Head, T>::value || std::is_same<const Head, T>::value ||
            ForTypes<Types...>::template Has<T>::or_const_of;
    };

    template <typename ... OtherTypes>
    struct IsSuperSetOf
        { static const constexpr bool value = true; };

    template <typename OtherHead, typename ... OtherTypes>
    struct IsSuperSetOf<OtherHead, OtherTypes...> {
        static const constexpr bool value =
            Has<OtherHead>::value &&
            ForTypes<Head, Types...>::IsSuperSetOf<OtherTypes...>::value;
        static const constexpr bool with_const =
            (Has<OtherHead>::value || HasConstOf<OtherHead>::value) &&
            ForTypes<Head, Types...>::IsSuperSetOf<OtherTypes...>::value;
    };
};

// ----------------------------------------------------------------------------

class EntityRefAtt;
class EntityAtt;

constexpr const std::size_t k_auto_inline_size = 3*sizeof(void *);
constexpr const int         k_no_inline_index  = -1;

template <typename Type>
struct WouldInlineComponent {
    static constexpr const bool k_value =
           std::is_base_of_v<InlinedComponent, Type>
        || DefineWouldInline<Type>::k_value
        || sizeof(Type) <= k_auto_inline_size;
};

template <typename ... Types>
struct CountInlinedComponents {
    template <typename Type>
    struct GetInlineIndex
        { static constexpr const int k_index = k_no_inline_index; };

    static constexpr const int k_count = 0;
};

template <typename Head, typename ... Types>
struct CountInlinedComponents<Head, Types...> : public CountInlinedComponents<Types...> {
    // note: empty structures are counted as "inlined"
    //       though this should occur regardless since empty structures are
    //       "one" byte in size
    static constexpr const bool k_inline_head =
        WouldInlineComponent<Head>::k_value || std::is_empty_v<Head>;

    static constexpr const int  k_count       =
        (k_inline_head ? 1 : 0) + CountInlinedComponents<Types...>::k_count;

    template <typename Type>
    struct GetInlineIndex {
        static constexpr const int k_index = cul::ConstIntSelect<
            std::is_same<Type, Head>::value && k_inline_head,
            k_count - 1,
            CountInlinedComponents<Types...>::template GetInlineIndex<Type>::k_index
        >::k_value;
    };
};

// ----------------------------------------------------------------------------

enum TableEntryType { k_inlined, k_pointer, k_empty };

template <bool kt_v, TableEntryType kt_true_v, TableEntryType kt_false_v>
struct ConditionalTet {
    static constexpr const auto k_value = kt_true_v;
};

template <TableEntryType kt_true_v, TableEntryType kt_false_v>
struct ConditionalTet<false, kt_true_v, kt_false_v> {
    static constexpr const auto k_value = kt_false_v;
};

template <bool kt_v, TableEntryType kt_true_v, TableEntryType kt_false_v>
constexpr const auto kt_conditional_tet = ConditionalTet<kt_v, kt_true_v, kt_false_v>::k_value;

template <typename ComponentType>
constexpr const auto kt_component_entry_type =
    kt_conditional_tet<
        std::is_empty_v<ComponentType>, k_empty,
    kt_conditional_tet<
        WouldInlineComponent<ComponentType>::k_value, k_inlined,
    /* else */
        k_pointer
    >>;

template <TableEntryType k_type, typename Type>
class TableEntryImpl;

template <typename Type>
class TableEntryImpl<k_inlined, Type> {
public:
    TableEntryImpl() {}
    TableEntryImpl(const TableEntryImpl &) = delete;
    TableEntryImpl(TableEntryImpl &&) = delete;
    ~TableEntryImpl() {}

    TableEntryImpl & operator = (const TableEntryImpl &) = delete;
    TableEntryImpl & operator = (TableEntryImpl &&) = delete;

    Type * add() { return new (&m) Type(); }

    void remove() { get()->~Type(); }

    const Type * get() const { return reinterpret_cast<const Type *>(&m); }

    Type * get() { return reinterpret_cast<Type *>(&m); }

private:
    using Storage = typename std::aligned_storage<sizeof(Type), alignof(Type)>::type;
    Storage m;
};

template <typename Type>
class TableEntryImpl<k_pointer, Type> {
public:
    TableEntryImpl() {}
    TableEntryImpl(const TableEntryImpl &) = delete;
    TableEntryImpl(TableEntryImpl &&) = delete;
    ~TableEntryImpl() {}

    TableEntryImpl & operator = (const TableEntryImpl &) = delete;
    TableEntryImpl & operator = (TableEntryImpl &&) = delete;

    Type * add() { return (m = new Type()); }

    void remove() {
        delete m;
        m = nullptr;
    }

    Type * get() { return m; }

    const Type * get() const { return m; }

private:
    Type * m = nullptr;
};

template <typename Type>
class TableEntryImpl<k_empty, Type> {
public:
    static_assert(std::is_empty_v<Type>,
        "Only empty types maybe used for this table entry specialization.");

    TableEntryImpl() {}
    TableEntryImpl(const TableEntryImpl &) = delete;
    TableEntryImpl(TableEntryImpl &&) = delete;
    ~TableEntryImpl() {}

    TableEntryImpl & operator = (const TableEntryImpl &) = delete;
    TableEntryImpl & operator = (TableEntryImpl &&) = delete;

    Type * add() { return &s_m; }

    void remove() {}

    Type * get() { return &s_m; }

    const Type * get() const { return &s_m; }

private:
    static Type s_m;
};

template <typename Type>
/* private */ Type TableEntryImpl<k_empty, Type>::s_m = Type{};

// ----------------------------------------------------------------------------

template <typename ... Types>
class ComponentTable {
public:
    static constexpr const char * const k_never_should_be_called_msg =
        "This function should never be called and "
        "exists only to make compliation possible.";
    [[noreturn]] void get()
        { throw std::runtime_error(k_never_should_be_called_msg); }
    [[noreturn]] void add()
        { throw std::runtime_error(k_never_should_be_called_msg); }
    [[noreturn]] void remove()
        { throw std::runtime_error(k_never_should_be_called_msg); }
};

template <typename Head, typename ... Types>
class ComponentTable<Head, Types...> :
    public ComponentTable<Types...>,
    public TableEntryImpl<kt_component_entry_type<Head>, Head>
{
public:
    using ComponentTable<Types...>::get;
    using ComponentTable<Types...>::add;
    using ComponentTable<Types...>::remove;
    using EntryImpl = TableEntryImpl<kt_component_entry_type<Head>, Head>;

    template <typename T>
    const typename std::enable_if<std::is_same<Head, T>::value, T>::
    type * get(TypeTag<Head>) const { return EntryImpl::get(); }

    template <typename T>
    typename std::enable_if<std::is_same<Head, T>::value, T>::
    type * get(TypeTag<Head>) { return EntryImpl::get(); }

    template <typename T>
    typename std::enable_if<std::is_same<Head, T>::value, T>::
    type * add(TypeTag<Head>) { return EntryImpl::add(); }

    void remove(TypeTag<Head>) { EntryImpl::remove(); }
};

template <typename ... Types>
class ComponentTableHead final : public ReferenceCounter {
public:
    using FullTypeList = TypeList<Types...>;
    using CountInlined = CountInlinedComponents<Types...>;
    using RtError      = std::runtime_error;

    template <typename Type>
    static constexpr const auto kt_inline_index =
        CountInlined::template GetInlineIndex<Type>::k_index;

    static constexpr const auto k_sizeof_components =
        sizeof(ComponentTable<Types...>);

    ComponentTableHead() {}
    ComponentTableHead(const ComponentTableHead &) = delete;
    ComponentTableHead(ComponentTableHead &&) = delete;
    ~ComponentTableHead() { remove_all(); }

    ComponentTableHead & operator = (const ComponentTableHead &) = delete;
    ComponentTableHead & operator = (ComponentTableHead &&) = delete;

    template <typename Type>
    typename std::enable_if<FullTypeList::template HasType<Type>::k_value, Type>::
    type & add();

    template <typename Type>
    const typename std::enable_if<FullTypeList::template HasType<Type>::k_value, Type>::
    type * get_ptr() const noexcept;

    template <typename Type>
    typename std::enable_if<FullTypeList::template HasType<Type>::k_value, Type>::
    type * get_ptr() noexcept;

    template <typename Type>
    typename std::enable_if<FullTypeList::template HasType<Type>::k_value, void>::
    type remove();

    void remove_all() { remove_all_(TypeList<Types...>()); }

    static void id_func() {}

    virtual void * full_downcast(IdFuncPtr ptr) noexcept
        { return ptr == id_func ? this : nullptr; }

private:
    template <typename ... OtherTypes>
    void remove_all_(TypeList<OtherTypes...>) {}

    template <typename Head, typename ... OtherTypes>
    void remove_all_(TypeList<Head, OtherTypes...>);

    using BitSet = std::bitset<CountInlined::k_count>;
    ComponentTable<Types...> m_table;
    BitSet m_inlined_present;
};

template <typename ... Types>
template <typename Type>
typename std::enable_if<TypeList<Types...>::template HasType<Type>::k_value, Type>::
type & ComponentTableHead<Types...>::add() {
    static const constexpr int k_index = kt_inline_index<Type>;
    if (get_ptr<Type>()) {
        throw RtError("ComponentTableHead::add(): component of this type is already present.");
    }
    Type * rv = m_table.template add<Type>(TypeTag<Type>());
    if (k_index != k_no_inline_index) {
        m_inlined_present.set(k_index);
    }
    return *rv;
}

template <typename ... Types>
template <typename Type>
const typename std::enable_if<TypeList<Types...>::template HasType<Type>::k_value, Type>::
type * ComponentTableHead<Types...>::get_ptr() const noexcept {
    static const constexpr int k_index = kt_inline_index<Type>;
    if constexpr (k_index == k_no_inline_index) {
        return m_table.template get<Type>(TypeTag<Type>());
    }
    if (m_inlined_present.test(k_index)) {
        return m_table.template get<Type>(TypeTag<Type>());
    }
    return nullptr;
}

template <typename ... Types>
template <typename Type>
typename std::enable_if<TypeList<Types...>::template HasType<Type>::k_value, Type>::
type * ComponentTableHead<Types...>::get_ptr() noexcept {
    const auto * const_this = this;
    return const_cast<Type *>(const_this->template get_ptr<Type>());
}

template <typename ... Types>
template <typename Type>
typename std::enable_if<TypeList<Types...>::template HasType<Type>::k_value, void>::
type ComponentTableHead<Types...>::remove() {
    if (!get_ptr<Type>()) {
        throw RtError("ComponentTableHead::remove(): cannot remove a "
                      "component that is not present.");
    }
    static const constexpr int k_index = kt_inline_index<Type>;
    if (k_index != k_no_inline_index)
        { m_inlined_present.reset(k_index); }
    (void)m_table.remove(TypeTag<Type>());
}

template <typename ... Types>
template <typename Head, typename ... OtherTypes>
/* private */ void ComponentTableHead<Types...>::remove_all_(TypeList<Head, OtherTypes...>) {
    if (get_ptr<Head>())
        { remove<Head>(); }
    remove_all_(TypeList<OtherTypes...>());
}

// ----------------------------------------------------------------------------

template <typename Iterator>
class Range {
public:
    Range() {}
    Range(Iterator beg, Iterator end_): m_beg(beg), m_end(end_) {}
    Iterator begin() const noexcept { return m_beg; }
    Iterator end  () const noexcept { return m_end; }
private:
    Iterator m_beg, m_end;
};

} // end of detail namespace

template <typename ... Types>
class EntityManager;

} // end of ecs namespace
