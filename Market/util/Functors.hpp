//
//  Functors.hpp
//  Market
//
//  Created by Blagovest on 14/11/20.
//

#ifndef Util_Functors_hpp
#define Util_Functors_hpp

#include <utility>

namespace mkt {
namespace util {

    template <typename Type>
    class constant {
        Type value;
    public:
        constexpr constant(Type value = {}): value(value) {}
        template <typename... Rest>
        constexpr auto operator() (Rest...) const { return value; }
        constexpr operator Type () const { return value; }
    };

    // FIXME: remove in favor of std::identity when supported
    class identity {
    public:
        template <typename T>
        auto operator() (T && value) const { return std::forward<T>(value); }
    };

}
}


#endif /* Functors_hpp */
