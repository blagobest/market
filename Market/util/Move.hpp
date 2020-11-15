//
//  Move.hpp
//  Market
//
//  Created by Blagovest on 07/11/20.
//

#ifndef Util_Move_hpp
#define Util_Move_hpp

#include <string>
#include <chrono>

namespace mkt {
namespace util {
    
    template <typename Time, typename Symbol>
    struct Event {
        Symbol symbol;
        Time time;
    };
    
    template <typename Time, typename Price, typename Size, typename Count>
    struct MarketSide {
        Time time;
        Price price;
        Count count;
        Size size;
    };
    
    // #=MarketMaker,EventSymbol,EventTime,ExchangeCode,MarketMaker,BidTime,BidPrice,BidSize,BidCount,AskTime,AskPrice,AskSize,AskCount
    
    template <typename Time, typename Price, typename Size, typename Count, typename MarketMetadata, typename Symbol = std::string>
    struct Move {
        Event<Time, Symbol> event;
        MarketSide<Time, Price, Size, Count> bid, ask;
        MarketMetadata market;
    };
    
}
}

#endif /* Move_hpp */
