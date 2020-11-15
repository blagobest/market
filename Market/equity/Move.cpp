//
//  Move.cpp
//  Market
//
//  Created by Blagovest on 07/11/20.
//

#include <iomanip>
#include <ctime>
#include <iostream>

#include "Move.hpp"

namespace mkt {
namespace equity {

std::chrono::time_point<Move::clock> Move::parse_date(const std::string & value) {
    // parses stuff like 20190806-150007-0500
    
    using std::chrono::duration_cast;
    
    tm t{};
    
    t.tm_year = ord(value[0], 1000) + ord(value[1], 100) + ord(value[2], 10) + ord(value[3]) - 1900;
    t.tm_mon = ord(value[4], 10) + ord(value[5]) - 1;
    t.tm_mday = ord(value[6], 10) + ord(value[7]);
    
    t.tm_hour = ord(value[9], 10) + ord(value[10]);
    t.tm_min = ord(value[11], 10) + ord(value[12]);
    t.tm_sec = ord(value[13], 10) + ord(value[14]);
    
    long long millis = 0;
    
    if (value[15] == '.') {
        millis = ord(value[16], 100) + ord(value[17], 10) + ord(value[18]);
    } else if (value[15] != '-') {
        throw new std::invalid_argument("bad value" + value);
    }

    time_t time = mktime(&t);
    
    return clock::from_time_t(time) + std::chrono::milliseconds(millis);
}

bool Move::from(const std::vector<std::string> & values) {
    if (values.size() != Field::COUNT && values.size() + 1 != Field::COUNT) {
        std::cerr << "[EquityMove] Unexpected number of fields: " << values.size() << " (expected: " << Field::COUNT << " (or 1 fewer), was " << values.size() << ")" << std::endl;
        return false;
    }
    // MarketMaker,EventSymbol,EventTime,ExchangeCode,MarketMaker,BidTime,BidPrice,BidSize,BidCount,AskTime,AskPrice,AskSize,AskCount
    event.symbol = values[Field::EVENT_SYMBOL];
    event.time = parse_date(values[Field::EVENT_TIME]);
    
    bid.time = parse_date(values[Field::BID_TIME]);
    bid.count = std::stoi(values[Field::BID_COUNT]);
    bid.price = std::stold(values[Field::BID_PRICE]);
    bid.size = std::stoi(values[Field::BID_SIZE]);
    
    ask.time = parse_date(values[Field::ASK_TIME]);
    ask.count = std::stoi(values[Field::ASK_COUNT]);
    ask.price = std::stold(values[Field::ASK_PRICE]);
    ask.size = std::stoi(values[Field::ASK_SIZE]);
    
    market.xcode = values[Field::EXCHANGE_CODE][0];
    market.market_maker = values[Field::MARKET_MAKER];
    market.flags = values.size() == Field::COUNT ? values[Field::FLAGS] : "";
    
    return true;
}

std::ostream & operator << (std::ostream & out, Move & move) {
    return out << "EqMove(" << move.event.symbol << ", "
        << "bid(price=" << move.bid.price << ", count=" << move.bid.count << ", size=" << move.bid.size << ", time=" << std::chrono::duration_cast<std::chrono::milliseconds>(move.bid.time.time_since_epoch()).count() << "), "
        << "ask(price=" << move.ask.price << ", count=" << move.ask.count << ", size=" << move.ask.size << ", time=" << std::chrono::duration_cast<std::chrono::milliseconds>(move.ask.time.time_since_epoch()).count() << ")";
}

}
}
