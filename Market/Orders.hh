#ifndef ORDERS_HPP
#define ORDERS_HPP

#include <unordered_map>
#include <mutex>
#include <thread>

template <typename Quantity, typename Price, typename Time>
class Move {
    Quantity quantity;
    Price price;
    Time time;

public:
    Move(Quantity quantity, Price price, Time time) : quantity(quantity), price(price), time(time) {}

    Quantity getQuantity() const {
        return quantity;
    }
    
    Price getPrice() const {
        return price;
    }
    
    Time getTime() const {
        return time;
    }
    
    decltype(std::declval<Quantity>() * std::declval<Price>()) volume() const {
        return quantity * price;
    }
};

template <typename Quantity, typename Price, typename Time>
class MovesSequence {
public:
    using move_t = Move<Quantity, Price, Time>;

    MovesSequence() = default;
    MovesSequence(std::vector<move_t> moves) : moves(moves) {}

private:
    std::vector<move_t> moves;
};

template <typename MarketData, typename Quantity, typename Price, typename Time>
class Order {
public:
    using market_data_t = MarketData;
    using quantity_t = Quantity;
    using price_t = Price;
    using time_t = Time;
    using move_t = Move<Quantity, Price, Time>;
    
    Order(market_data_t marketData, move_t move): marketData(marketData), move(move) {}
    Order(const Order& other): marketData(other.marketData), move(other.move) {}
    // TOOD assigment

    const move_t & getMove() const {
        return move;
    }

    const market_data_t & getMarketData() const {
        return marketData;
    }

private:
    market_data_t marketData;
    move_t move;
};

struct MockMarketData {
    enum OrderType {
        BUY, SELL
    };

    int id;
    OrderType type;
    
    MockMarketData(int id, OrderType type): id(id), type(type) {}
};

template <typename Quantity, typename Price, typename Time>
class OrderExecutor {
    
};

// Schedulers
template <typename MarketData, typename Quantity, typename Price, typename Time>
class Scheduler {
public:
    using order_t = Order<MarketData, Quantity, Price, Time>;
    using executor_t = OrderExecutor<Quantity, Price, Time>;
    
    Scheduler(
       std::shared_ptr<executor_t> executor): executor(executor) {}
    
private:
    std::shared_ptr<executor_t> executor;
};

template <typename MarketData, typename Quantity, typename Price, typename Time>
class ImmediateScheduler: Scheduler<MarketData, Quantity, Price, Time> {
public:
    ImmediateScheduler(std::shared_ptr<typename ImmediateScheduler::executor_t> executor): Scheduler<MarketData, Quantity, Price, Time>(executor) {}
    
    ImmediateScheduler & schedule(const typename ImmediateScheduler::order_t & order) {
        executor->execute(order);
        return *this;
    }
private:
    std::shared_ptr<typename ImmediateScheduler::executor_t> executor;
};

template <typename Order>
struct BatchOrderHasher;

template <typename Order>
struct BatchOrderEqual;

template <typename MarketData, typename Quantity, typename Price, typename Time, typename Interval = std::chrono::milliseconds>
class BatchOrderScheduler: Scheduler<MarketData, Quantity, Price, Time>  {
    /**
     * keeps running a background thread which sends batched orders
     * every interval
     */
public:
    BatchOrderScheduler(
        std::shared_ptr<typename BatchOrderScheduler::executor_t> executor,
        std::binary_function<typename BatchOrderScheduler::order_t, typename BatchOrderScheduler::order_t, typename BatchOrderScheduler::order_t> combine,
        Interval interval):
        executor(executor), combine(std::move(combine)), interval(interval), batches(), batchesMutex(), processing(true), thread([this, interval]() {
                while (this->processing) {
                    auto x = std::chrono::steady_clock::now() + interval;
                    this->tick();
                    std::this_thread::sleep_until(x);
                }
            }) {}

    BatchOrderScheduler & schedule(const typename BatchOrderScheduler::order_t & order) {
        std::lock_guard<std::mutex> lock(batchesMutex);
        auto batch = batches.find(order);
        if (batch != batches.end()) {
            batches[order] = combine(*batch, order);
        } else {
            batches[order] = order;
        }
        return *this;
    }
    
    ~BatchOrderScheduler() {
        this->processing = false;
        thread.join();
    }
private:
    void tick() {
        std::lock_guard<std::mutex> lock(batchesMutex);
        for (auto order: batches) {
            if (order)
            executor->execute(order);
        }
        batches.clear();
    }
    
    bool processing;
    std::mutex batchesMutex;
    std::thread thread;
    std::unordered_map<typename BatchOrderScheduler::order_t, typename BatchOrderScheduler::order_t, BatchOrderHasher<typename BatchOrderScheduler::order_t>, BatchOrderEqual<typename BatchOrderScheduler::order_t>> batches;
    std::shared_ptr<typename BatchOrderScheduler::executor_t> executor;
    std::binary_function<typename BatchOrderScheduler::order_t, typename BatchOrderScheduler::order_t, typename BatchOrderScheduler::order_t> combine;
    Interval interval;
};

/**
 * Specializations
 */
template <typename Quantity, typename Price, typename Time>
struct BatchOrderHasher<Order<MockMarketData, Quantity, Price, Time>> {
    size_t operator() (const Order<MockMarketData, Quantity, Price, Time> & x) const {
        return std::hash<Price>()(x.getMove().getPrice()) * 1337
        + std::hash<int>()(x.getMarketData().id) * 11
        + std::hash<int>()(static_cast<int>(x.getMarketData().type));
    };
};

template <typename Quantity, typename Price, typename Time>
struct BatchOrderEqual<Order<MockMarketData, Quantity, Price, Time>> {
    size_t operator() (const Order<MockMarketData, Quantity, Price, Time> & x, const Order<MockMarketData, Quantity, Price, Time> & y) const {
        return x.getMove().getPrice() == y.getMove().getPrice()
            && x.getMarketData().id == y.getMarketData().id
            && x.getMarketData().type == y.getMarketData().type;
    };
};

#endif
