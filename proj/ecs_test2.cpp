#include <ecs/ecs.hpp>
#include <common/TypeList.hpp>

namespace ecs {

template <typename ... Types>
using Tuple = std::tuple<Types...>;

template <typename ... Types>
class SinglesSystem {
public:

    template <typename ... OtherTypes>
    static constexpr const bool k_has_all_types = true;

    template <typename T, typename ... OtherTypes>
    static constexpr const bool k_has_all_types<T, OtherTypes...>
        =    TypeList<Types...>::template HasType<T>::k_value
          && k_has_all_types<OtherTypes...>;

    template <typename ... OtherTypes>
    using TupleForSystem = Tuple<OtherTypes...>;

    //template <typename

    template <typename ... OtherTypes>
    class For {
    protected:
        static_assert(k_has_all_types<OtherTypes...>, "");
        virtual void update(Tuple<OtherTypes...> &) = 0;
    };
};

}

namespace tn {

struct A {};
struct B {};
struct C {};

}

void get_tuples();

int main() {
    get_tuples();
}
