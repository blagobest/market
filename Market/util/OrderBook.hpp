//
//  OrderBook.hpp
//  Market
//
//  Created by Blagovest on 08/11/20.
//

#ifndef OrderBook_hpp
#define OrderBook_hpp

#include <functional>
#include <set>
#include <unordered_map>
#include <compare>
#include <optional>
#include <numeric>

namespace mkt {
namespace util {
    template <typename Order>
    struct order_id;

    template <typename Order>
    struct cheaper;

    template <typename Order>
    struct more_expensive;

    template <typename OrderDatabase>
    class BestPriceFillPolicy;

    template <typename Price, typename Quantity>
    class SimpleOrder;

    template <typename Order, typename Set = std::multiset<Order, cheaper<Order>>, template <typename> typename FillPolicy = BestPriceFillPolicy, typename OrderIdFunctor = order_id<Order>>
    class MultisetOrderDatabase;

    template <typename Order, typename BidsDatabase, typename AsksDatabase, typename PriceEvaluationPolicy>
    class OrderBook;

    // Price evaluation strategies
    class VolumeWeighedPriceEvaluationPolicy;
    template <long n, long m>
    class WeighedPriceEvaluationPolicy;

    /**
      * Actual Definitions
      */

    template <typename Order, typename BidsDatabase, typename AsksDatabase, typename PriceEvaluationPolicy>
    class OrderBook {
        BidsDatabase bids;
        AsksDatabase asks;
        
        PriceEvaluationPolicy price_evaluation_policy;
    public:
        OrderBook(PriceEvaluationPolicy price_evaluation_policy = PriceEvaluationPolicy {}): bids(), asks(), price_evaluation_policy(price_evaluation_policy) {}
        
        // add only the unfilled part of each order
        void bid(const Order & bid) {
            auto remaining = asks.fill(bid);
            if (remaining) {
                bids.add(Order(bid.price(), remaining));
            }
        }
        
        void ask(const Order & ask) {
            auto remaining = bids.fill(ask);
            if (remaining) {
                asks.add(Order(ask.price(), remaining));
            }
        }
        
        auto price() const {
            return price_evaluation_policy(*this);
        }
        
        auto safe_price() const {
            if (bids.empty() || asks.empty()) {
                return std::nullopt;
            } else {
                return price();
            }
        }
        
        friend PriceEvaluationPolicy;
    };
    
    // maps an order object to a hashable one
    // by default, returns the order itself
    // (so it must be hashable) but if there
    // is a natural notion of unique ID field
    // (preferably something like a long long?)
    // then partially specialize order_id<Order>
    template <typename Order>
    struct order_id {
        const Order & operator() (const Order & order) const { return order; }
    };

    template <typename Order>
    struct cheaper {
        auto operator() (const Order a, const Order b) const {
            return a.price() < b.price();
        }
    };

    template <typename Order>
    struct more_expensive {
        auto operator() (const Order a, const Order  b) const {
            return a.price() > b.price();
        }
    };

    template <typename Order, typename Set, template <typename> typename FillPolicy, typename OrderIdFunctor>
    class BestPriceFillPolicy<MultisetOrderDatabase<Order, Set, FillPolicy, OrderIdFunctor>> {
        using OrderDatabase = MultisetOrderDatabase<Order, Set, FillPolicy, OrderIdFunctor>;
        
        OrderDatabase & order_database;
    public:
        BestPriceFillPolicy(OrderDatabase & order_database): order_database(order_database) {}
        
        auto operator() (const Order & order) const {
            auto quantity = order.quantity();
            
            for (auto it = order_database.order_set.rbegin(); it != order_database.order_set.rend() && quantity > 0 && order_database.compare( order, *it); ) {
                if (quantity < it->quantity()) {
                    Order order = Order(it->price(), it->quantity() - quantity);
                    order_database.put(order);
                    quantity = 0;
                    break;
                } else {
                    quantity -= it->quantity();
                }
                
                order_database.erase(*it++);
            }
            
            return quantity;
        }
    };

    template <typename Order, typename Set, template <typename> typename FillPolicy, typename OrderIdFunctor>
    class MultisetOrderDatabase {
        using OrderId = std::remove_reference_t<decltype(std::declval<OrderIdFunctor>()(std::declval<Order>()))>;
        using Volume = std::remove_reference_t<decltype(std::declval<Order>().volume())>;
        using Iterator = typename Set::iterator;
        using Comparator = typename Set::value_compare;
        using Price = decltype(std::declval<Order>().price());
        
        Set order_set;
        std::unordered_map<OrderId, Iterator> order_lookup;
        
        Volume total_volume;
        OrderIdFunctor order_id_functor;
        FillPolicy<MultisetOrderDatabase> fill_policy;
        
        Comparator compare;
        
        void erase(const Order & order) {
            // erase from lookup map
            order_lookup.erase(order_id_functor(order));
            // erase from price set
            order_set.erase(order);
            // update total_volume
            total_volume -= order.volume();
        }
        
        void put(const Order & order) {
            // insert into price set
            auto iterator = order_set.insert(order);
            // insert into lookup map
            order_lookup[order_id_functor(order)] = iterator;
            // update total_volume
            total_volume += order.volume();
        }
    public:
        MultisetOrderDatabase(OrderIdFunctor order_id_functor = OrderIdFunctor {}): fill_policy(FillPolicy<MultisetOrderDatabase>(*this)), order_set(), total_volume(), order_id_functor(order_id_functor), compare(order_set.value_comp()) {}
        MultisetOrderDatabase(const MultisetOrderDatabase & order_database) = delete;
        
        void add(const Order & order) {
            put(order);
        }
        
        auto fill(const Order & order) {
            return fill_policy(order);
        }
        
        size_t size() const {
            return order_set.size();
        }
        
        bool empty() const {
            return order_set.empty();
        }
        
        auto price() const { // WARNING: Don't call this if empty - garbage
            return order_set.empty() ? Price() : order_set.begin()->price();
        }
        
        auto safe_price() const -> std::optional<decltype(price())> {
            return order_set.empty() ? std::nullopt : order_set.begin()->price();
        }
        
        auto volume() const {
            return total_volume;
        }
        
        friend FillPolicy<MultisetOrderDatabase>;
    };

    template <typename Price, typename Quantity>
    class SimpleOrder {
        Price _price;
        Quantity _quantity;
    public:
        SimpleOrder(Price price, Quantity quantity): _price(price), _quantity(quantity) {}
        SimpleOrder(const SimpleOrder & order) = default;
        Price price() const { return _price; }
        Quantity quantity() const { return _quantity; }
        auto volume() const { return _price * _quantity; }
        bool operator == (const SimpleOrder & rhs) const = default;
    };
    
    template <long n, long m>
    class WeighedPriceEvaluationPolicy {
        const long double alpha = static_cast<long double>(n) / static_cast<long double>(m);
        const long double beta = static_cast<long double>(m - n) / static_cast<long double>(m);
    public:
        template <long p, long q, typename = std::enable_if_t<std::gcd(n, m) == std::gcd(p, q)>>
        operator WeighedPriceEvaluationPolicy<p, q>() {
            return {};
        }
        
        template <typename OrderBook>
        auto operator() (const OrderBook & order_book) const {
            return alpha * order_book.bids.price() + beta * order_book.asks.price();
        }
    };

    using AskPriceEvaluationPolicy = WeighedPriceEvaluationPolicy<0, 1>;
    using BidPriceEvaluationPolicy = WeighedPriceEvaluationPolicy<1, 1>;
    using AveragePriceEvaluationPolicy = WeighedPriceEvaluationPolicy<1, 2>;

    class VolumeWeighedPriceEvaluationPolicy {
    public:
        template <typename OrderBook>
        auto operator() (const OrderBook & order_book) const {
            auto bid_volume = order_book.bids.volume();
            auto ask_volume = order_book.asks.volume();
            auto total_volume = bid_volume + ask_volume;
            auto bid_weight = total_volume ? bid_volume / total_volume : 0;
            auto ask_weight = total_volume ? ask_volume / total_volume : 0;
            return bid_weight * order_book.bids.price() + ask_weight * order_book.asks.price();
        }
    };
}
}

namespace std {

template <typename Price, typename Quantity>
struct hash<const mkt::util::SimpleOrder<Price, Quantity>> {
    std::size_t operator()(const mkt::util::SimpleOrder<Price, Quantity> & k) const {
        return ((std::hash<Price>()(k.price()) ^ (std::hash<Quantity>()(k.quantity()) << 1)) >> 1);
    }
};

}

#endif /* OrderBook_hpp */
