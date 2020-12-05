#pragma once

#include <common/TypeList.hpp>

#include <type_traits>
#include <stdexcept>
#include <unordered_map>
#include <bitset>

namespace ecs {

struct InlinedComponent {};

namespace detail {

struct ReferenceCounter;

struct ReferenceManager {
    static ReferenceManager & null_instance();

    virtual ~ReferenceManager();
    virtual ReferenceCounter * provide_new_identity() = 0;
};

struct ReferenceCounter {
    virtual ~ReferenceCounter();

    int count = 0;
    bool requesting_deletion = false;
    bool expired = false;
    // needed for copying
    ReferenceManager * reference_manager = nullptr;
#   ifdef MACRO_ECS_DEADBEEF_SAFETY
    std::size_t safety = 0;
#   endif

    using IdFuncPtr = void (*)();
    virtual void * full_downcast(IdFuncPtr) noexcept = 0;
protected:
    ReferenceCounter() {}
};

void decrement(ReferenceCounter *);

void increment(ReferenceCounter *);

template <typename ... Types>
struct ForTypes {
    template <typename T>
    struct HasConstOf {
        static const constexpr bool value = false;
    };
    template <typename T>
    struct Has {
        static const constexpr bool value = false;
        static const constexpr bool or_const_of = false;
    };
    template <typename ... OtherTypes>
    struct IsSuperSetOf {
        static const constexpr bool value = true;
    };
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
    struct IsSuperSetOf {
        static const constexpr bool value = true;
    };
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

class EntityRefAtt;
class EntityAtt;

constexpr const std::size_t k_auto_inline_size = 3*sizeof(void *);
constexpr const int k_no_inline_index = -1;

template <typename Type>
struct WouldInlineComponent {
    static constexpr const bool k_value =
        std::is_base_of<InlinedComponent, Type>::value ||
        sizeof(Type) < k_auto_inline_size;
};

template <typename ... Types>
struct CountInlinedComponents {
    template <typename Type>
    struct GetInlineIndex {
        static constexpr const int k_index = k_no_inline_index;
    };
    static constexpr const int k_count = 0;
};

template <typename Head, typename ... Types>
struct CountInlinedComponents<Head, Types...> : public CountInlinedComponents<Types...> {
    static constexpr const bool k_inline_head = WouldInlineComponent<Head>::k_value;
    static constexpr const int k_count =
        (k_inline_head ? 1 : 0) + CountInlinedComponents<Types...>::k_count;
    template <typename Type>
    struct GetInlineIndex {
        static constexpr const int k_index = ConstIntSelect<
            std::is_same<Type, Head>::value && k_inline_head,
            k_count - 1,
            CountInlinedComponents<Types...>::template GetInlineIndex<Type>::k_index
        >::k_value;
    };
};

struct UscDelNotifier {
    virtual ~UscDelNotifier();
    virtual void notify_deletion(std::size_t) = 0;
};

template <bool k_is_inlined, typename Type>
struct TableEntryImpl;

template <typename Type>
struct TableEntryImpl<true, Type> {
    using Storage = typename std::aligned_storage<sizeof(Type), alignof(Type)>::type;

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

    Storage m;
};

template <typename Type>
struct TableEntryImpl<false, Type> {
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

    Type * m = nullptr;
};

template <typename ... Types>
struct ComponentTable {
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
struct ComponentTable<Head, Types...> : public ComponentTable<Types...> {
    static constexpr const bool k_component_inlined = WouldInlineComponent<Head>::k_value;
    using ComponentTable<Types...>::get;
    using ComponentTable<Types...>::add;
    using ComponentTable<Types...>::remove;

    template <typename T>
    const typename std::enable_if<std::is_same<Head, T>::value, T>::
    type * get(TypeTag<Head>) const { return m.get(); }

    template <typename T>
    typename std::enable_if<std::is_same<Head, T>::value, T>::
    type * get(TypeTag<Head>) { return m.get(); }

    template <typename T>
    typename std::enable_if<std::is_same<Head, T>::value, T>::
    type * add(TypeTag<Head>) { return m.add(); }

    void remove(TypeTag<Head>) { m.remove(); }

    TableEntryImpl<k_component_inlined, Head> m;
};

template <typename ... Types>
class ComponentTableHead final : public ReferenceCounter {
public:
    using FullTypeList = TypeList<Types...>;
    using CountInlined = CountInlinedComponents<Types...>;
    using RtError      = std::runtime_error;
    template <typename Type>
    struct HasType {
        static constexpr const bool k_value =
            FullTypeList::template HasType<Type>::k_value;
    };
    template <typename Type>
    struct InlineIndex {
        static const constexpr int k_index =
            CountInlined::template GetInlineIndex<Type>::k_index;
    };

    ComponentTableHead() {}
    ComponentTableHead(const ComponentTableHead &) = delete;
    ComponentTableHead(ComponentTableHead &&) = delete;
    ~ComponentTableHead()
        { remove_all(); }

    ComponentTableHead & operator = (const ComponentTableHead &) = delete;
    ComponentTableHead & operator = (ComponentTableHead &&) = delete;

    template <typename Type>
    typename std::enable_if<HasType<Type>::k_value, Type>::
    type & add() {
        static const constexpr int k_index = InlineIndex<Type>::k_index;
        if (get_ptr<Type>()) {
            throw RtError("ComponentTableHead::add(): component of this type is already present.");
        }
        Type * rv = m_table.template add<Type>(TypeTag<Type>());
        if (k_index != k_no_inline_index) {
            m_inlined_present.set(k_index);
        }
        return *rv;
    }

    template <typename Type>
    const typename std::enable_if<HasType<Type>::k_value, Type>::
    type * get_ptr() const noexcept {
        static const constexpr int k_index = InlineIndex<Type>::k_index;
        if constexpr (k_index == k_no_inline_index) {
            return m_table.template get<Type>(TypeTag<Type>());
        }
        if (m_inlined_present.test(k_index)) {
            return m_table.template get<Type>(TypeTag<Type>());
        }
        return nullptr;
    }

    template <typename Type>
    typename std::enable_if<HasType<Type>::k_value, Type>::
    type * get_ptr() noexcept {
        const auto * const_this = this;
        return const_cast<Type *>(const_this->template get_ptr<Type>());
    }

    template <typename Type>
    typename std::enable_if<HasType<Type>::k_value, void>::
    type remove() {
        if (!get_ptr<Type>()) {
            throw RtError("ComponentTableHead::remove(): cannot remove a "
                          "component that is not present.");
        }
        static const constexpr int k_index = InlineIndex<Type>::k_index;
        if (k_index != k_no_inline_index)
            { m_inlined_present.reset(k_index); }
        (void)m_table.template remove(TypeTag<Type>());
    }

    void remove_all()
        { remove_all_(TypeList<Types...>()); }

    static void id_func() {}

    virtual void * full_downcast(IdFuncPtr ptr) noexcept
        { return ptr == id_func ? this : nullptr; }

private:
    template <typename ... OtherTypes>
    void remove_all_(TypeList<OtherTypes...>) {}

    template <typename Head, typename ... OtherTypes>
    void remove_all_(TypeList<Head, OtherTypes...>) {
        if (get_ptr<Head>())
            { remove<Head>(); }
        remove_all_(TypeList<OtherTypes...>());
    }

    using BitSet = std::bitset<CountInlined::k_count>;
    ComponentTable<Types...> m_table;
    BitSet m_inlined_present;
};

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
