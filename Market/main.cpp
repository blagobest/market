#include <iostream>
#include <utility>
#include <iostream>
#include <fstream>
#include <cmath>

#include "csv/Parser.hpp"
#include "csv/Schemas.hpp"
#include "csv/Reader.hpp"
#include "csv/Interpreter.hpp"

#include "equity/Move.hpp"

#include "math/Stats.hpp"

#include "util/CandleStick.hpp"
#include "util/OrderBook.hpp"

#include "simulate/RandomWalk.hpp"
#include "fix/Parser.hpp"

void test_order_book() {
    using Order = mkt::util::SimpleOrder<double, unsigned>;
    using OrderBook = mkt::util::OrderBook<Order,
        mkt::util::MultisetOrderDatabase<Order, std::multiset<Order, mkt::util::cheaper<Order>>>,
        mkt::util::MultisetOrderDatabase<Order, std::multiset<Order, mkt::util::more_expensive<Order>>>,
        mkt::util::VolumeWeighedPriceEvaluationPolicy>;
    
    OrderBook book;
    
    book.bid({2, 1u});
    std::cerr << book.price() << std::endl;
    book.bid({1.5, 5u});
    std::cerr << book.price() << std::endl;
    
    book.ask({1.7, 2u});
    std::cerr << book.price() << std::endl;
}

void test_csv_reader() {
    std::ifstream in("/Users/blagovest/Projects/Market/Market/data/apple-price-level-book.csv");
    
    if (!in) {
        std::cerr << strerror(errno);
        return;
    }
    
    mkt::csv::Reader reader (in);
    
    mkt::csv::Interpreter<mkt::equity::Move> csv(reader);
    
    auto headers = csv.get_headers();
    auto row = mkt::equity::Move();
    
    std::vector<decltype(row)> rows;
    
    mkt::util::CandleStick<decltype(row.bid.price)> w {};
    
    int i = 0;
    
    while (csv.get_next(row)) {
        w.update(row.bid.price);
        if (++i % 10 == 0) {
            std::cerr << w << std::endl;
            w.reset();
        }
        rows.push_back(row);
    }
}

void test_random_walk() {
    using Time = std::chrono::system_clock::time_point;
    using Price = double;
    using Duration = decltype(std::declval<Time>() - std::declval<Time>());
    using Mean = std::function<double(Duration)>;
    using Volatility = std::function<double(Duration)>;
    using unit = std::chrono::seconds;
    
    auto now = std::chrono::system_clock::now();
    auto durations = mkt::simulate::DurationSinceLast<Price, Time>(0.0, now);
    
    auto walk = mkt::simulate::AbsoluteStatefulStochasticRandomWalk
    <mkt::simulate::DurationSinceLast<Price, Time>, Price, Time, Mean, Volatility> (
        durations,
        [] (auto dt) { return 0; },
        [] (auto dt) { return sqrt(std::chrono::duration_cast<unit>(dt).count()); }
    );
    
    for (int i = 1; i <= 80; i++) {
        std::cerr << walk(now + std::chrono::hours(i)) << std::endl;
    }
}

void test_fix_parser() {
    std::string message = "8=FIX.4.2|9=65|35=A|49=SERVER|56=CLIENT|34=177|52=20090107-18:15:16|98=0|108=30|10=062|";
    std::replace(message.begin(), message.end(), '|', static_cast<char>(0x1));
    using String = std::string_view;
    mkt::fix::Parser<mkt::fix::MapBasedStoragePolicy<String>, String> map_parser (message);
    mkt::fix::Parser<mkt::fix::SequenceBasedStoragePolicy<String>, String> seq_parser (message);
    assert (map_parser.length() == seq_parser.length());
    std::cerr << "length: " << map_parser.length() << std::endl;
}

int main() {
    test_fix_parser();

    return 0;
}
