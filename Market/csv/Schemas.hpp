//
//  Schemas.hpp
//  Market
//
//  Created by Blagovest on 06/11/20.
//

#ifndef Csv_Schemas_hpp
#define Csv_Schemas_hpp

#include <vector>
#include <string>
#include <tuple>
#include <chrono>

namespace mkt {
namespace csv {
namespace schema {

    struct Raw {
        std::vector<std::string> values;
        bool from(const std::vector<std::string> & values) noexcept { this->values = values; return true; }
    };
    
    template <template <typename> typename Parser, typename... Args>
    class CustomizableTuple {
        template <int i>
        bool set(const std::vector<std::string> & values) {
            if constexpr (i == sizeof...(Args)) {
                return true;
            }
            
            try {
                using ith = typename std::tuple_element<i, decltype(tuple)>::type;
                std::get<i>(tuple) = Parser<ith>(values[i]);
                set<i + 1>(values);
            } catch (...) {
                return false;
            }
        }
    public:
        std::tuple<Args...> tuple;
        
        bool from(const std::vector<std::string> & values) noexcept {
            if (values.size() != sizeof...(Args)) {
                return false;
            }
            
            return set<0>(values);
        }
    };
    
    template <typename T>
    struct DefaultParser;
    
    template <>
    struct DefaultParser<int> {
        static int parse(const std::string & value) {
            return std::stoi(value);
        }
    };
    
    template <>
    struct DefaultParser<std::string> {
        static std::string parse(const std::string & value) noexcept {
            return value;
        }
    };
    
    template <typename... Args>
    using Tuple = CustomizableTuple<DefaultParser, Args...>;

}
}
}

#endif /* Schemas_hpp */
