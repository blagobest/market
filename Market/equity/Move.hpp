//
//  Move.hpp
//  Market
//
//  Created by Blagovest on 07/11/20.
//

#ifndef Equity_Move_hpp
#define Equity_Move_hpp

#include <iomanip>
#include <ctime>
#include <iostream>

#include "../util/Move.hpp"
#include "../csv/Schemas.hpp"

namespace mkt {
namespace equity {
    
    template <typename ExchangeCode, typename MarketMakerId = int, typename Flags = std::string>
    struct MoveMetadata {
        ExchangeCode xcode;
        MarketMakerId market_maker;
        Flags flags;
    };
    
    class Move: public mkt::util::Move<std::chrono::time_point<std::chrono::system_clock>, long double, unsigned, unsigned, MoveMetadata<char, std::string>> {
        
        using clock = std::chrono::system_clock;
        
        enum Field: uint8_t {
            DUMMY,
            EVENT_SYMBOL,
            EVENT_TIME,
            EXCHANGE_CODE,
            MARKET_MAKER,
            BID_TIME,
            BID_PRICE,
            BID_SIZE,
            BID_COUNT,
            ASK_TIME,
            ASK_PRICE,
            ASK_SIZE,
            ASK_COUNT,
            FLAGS,
            COUNT
        };
        
        static int ord(char c, int pw = 1) { return pw * (c - '0'); }
        static std::chrono::time_point<clock> parse_date(const std::string & value);
    public:
        bool from(const std::vector<std::string> & values);
        friend std::ostream & operator << (std::ostream & out, Move & move);
    };
}

}

#endif /* Move_hpp */
