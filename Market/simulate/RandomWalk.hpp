//
//  RandomWalk.hpp
//  Market
//
//  Created by Blagovest on 13/11/20.
//

#ifndef Simulate_RandomWalk_hpp
#define Simulate_RandomWalk_hpp

#include <random>
#include <optional>
#include <functional>
#include <map>

#include "../util/Functors.hpp"

namespace mkt {
namespace simulate {

    template <typename Volatility>
    class FixedVolatility;

    template <typename Mean>
    class FixedMean;

    template <typename Float, typename Distribution, typename Generator, typename RandomDevice = std::random_device>
    class StandardDistribution;

    template <typename Float, typename Distribution>
    class RescaledDistribution;

    template <typename Float>
    using StandardNormalDistribution = StandardDistribution<Float, std::normal_distribution<Float>, std::mt19937, std::random_device>;

    template <typename Float>
    class NormalDistribution;

    // Generates a random walk sampling from a given distribution and rescaling with time-dependent mean/volatility
    // Works with absolute time
    template <typename Price, typename Time, typename Mean = FixedMean<double>, typename Volatility = FixedVolatility<double>, typename Distribution = StandardNormalDistribution<double>>
    class AbsoluteStochasticRandomWalk;

    // Generates a random walk sampling from a given distribution and rescaling with duration-dependent mean/volatility
    // Works with incremental durations (from the last sample)
    template <typename Price, typename Duration, typename Mean = FixedMean<double>, typename Volatility = FixedVolatility<double>, typename Distribution = StandardNormalDistribution<double>>
    class IncrementalStochasticRandomWalk;

    // Generates a random walk sampling from a given distribution and rescaling with time-dependent mean/volatility
    // Works with absolute time, but uses state policy to extract a duration to use
    // with the underlying incremental stochastic random walk
    template <typename DurationsPolicy, typename Price, typename Time, typename Mean, typename Volatility, typename Distribution = StandardNormalDistribution<double>>
    class AbsoluteStatefulStochasticRandomWalk;

    template <typename Price, typename Time, size_t MaxSize, typename Map = std::map<Time, Price>>
    class DurationBetweenLastN;

    template <typename Price, typename Time>
    using DurationSinceLast = DurationBetweenLastN<Price, Time, 1, void>;

    // Implementation

    template <typename Float, typename Distribution, typename Generator, typename RandomDevice>
    class StandardDistribution {
        Generator generator;
        Distribution distribution;
    public:
        constexpr StandardDistribution(Distribution distribution = Distribution(), RandomDevice random_device = std::random_device()): generator(random_device()), distribution(distribution) {}
        constexpr auto operator () () { return distribution(generator); }
    };

    template <typename Float, typename Distribution>
    class RescaledDistribution {
        Distribution distribution;
        Float mean, stddev;
    public:
        constexpr RescaledDistribution(Distribution distribution = {}, Float mean = static_cast<Float>(0.0), Float stddev = static_cast<Float>(1.0)): mean(mean), stddev(stddev), distribution(distribution) {}
        constexpr auto operator () () { return mean + stddev * distribution(); }
    };

    template <typename Float>
    class NormalDistribution: public RescaledDistribution<Float, StandardNormalDistribution<Float>> {
    public:
        constexpr NormalDistribution(Float mean = static_cast<Float>(0.0), Float stddev = static_cast<Float>(1.0)): RescaledDistribution<Float, StandardNormalDistribution<Float>>({}, mean, stddev) {}
    };

    template <typename Distribution>
    class StochasticRandomWalk {
    protected:
        Distribution distribution;
        constexpr StochasticRandomWalk(Distribution distribution = {}): distribution(distribution) {}
    };

    template <typename Price, typename Time, typename Mean, typename Volatility, typename Distribution>
    class AbsoluteStochasticRandomWalk: public StochasticRandomWalk<Distribution> {
        Mean mean;
        Volatility volatility;
    public:
        constexpr AbsoluteStochasticRandomWalk(Mean mean = {}, Volatility volatility = {}, Distribution distribution = {}): StochasticRandomWalk<Distribution>(distribution), mean(mean), volatility(volatility) {}
        
        constexpr auto operator() (Time t) {
            auto mu = mean(t);
            auto sigma = volatility(t);
            return mu + sigma * this->distribution();
        }
    };

    template <typename Price, typename Duration, typename Mean, typename Volatility, typename Distribution>
    class IncrementalStochasticRandomWalk: public StochasticRandomWalk<Distribution> {
        Mean mean;
        Volatility volatility;
    public:
        constexpr IncrementalStochasticRandomWalk(Mean mean = Mean(), Volatility volatility = Volatility(), Distribution distribution = {}): StochasticRandomWalk<Distribution>(distribution), mean(mean), volatility(volatility) {}
        
        constexpr auto operator() (Duration dt) {
            auto mu = mean(dt);
            auto sigma = volatility(dt);
            return mu + sigma * this->distribution();
        }
    };

    template <typename Durations, typename Price, typename Time, typename Mean, typename Volatility, typename Distribution>
    class AbsoluteStatefulStochasticRandomWalk {
        using Duration = decltype(std::declval<Durations>().duration(std::declval<Time>()).first);
        using IncrementalRandomWalk = IncrementalStochasticRandomWalk<Price, Duration, Mean, Volatility, Distribution>;
        
        Durations duration;
        IncrementalRandomWalk walk;
    public:
        constexpr AbsoluteStatefulStochasticRandomWalk(Durations state, Mean mean, Volatility volatility, Distribution distribution = Distribution()): duration(state), walk(mean, volatility, distribution) {}
        
        constexpr auto operator() (Time t) {
            auto [dt, last_price] = duration.duration(t);
            auto incr = walk(dt);
            auto price = last_price + incr;
            duration.store(price, t);
            return price;
        }
    };

    template <typename Price, typename Time, size_t MaxSize, typename Map>
    class DurationBetweenLastN {
        Map cache;
    public:
        constexpr DurationBetweenLastN(Price price, Time t): cache() { cache[t] = price; }
        constexpr auto duration(Time t) const {
            auto e = cache.lower_bound(t);
            auto forwards = t, backwards = t;
            if (e != cache.end()) {
                if (e != cache.begin()) {
                    auto forwards = e->first();
                    --e;
                    auto backwards = e->first();
                    return std::make_pair(std::min(forwards - t < t - backwards), e->second());
                } else {
                    return std::make_pair(e->first() - t, e->second());
                }
            } else if (e != cache.begin()) {
                --e;
                return std::make_pair(t - e->first(), e->second());
            } else {
                return std::make_pair(t, Price());
            }
        }
        
        constexpr void store(Price price, Time t) {
            cache[t] = price;
            if constexpr (MaxSize != 0) {
                if (cache.size() > MaxSize) {
                    // remove earliest
                    cache.erase(cache.begin());
                }
            }
        }
    };

    template <typename Price, typename Time, typename Set>
    class DurationBetweenLastN<Price, Time, 1, Set> {
        Time last;
        Price last_price;
    public:
        constexpr DurationBetweenLastN(Price price, Time t): last(t), last_price(price) {}
        constexpr auto duration(Time t) const { return std::make_pair(t - last, last_price); }
        constexpr auto store(Price price, Time t) { last = t; last_price = price; }
    };

    template <typename Mean>
    class FixedMean: public mkt::util::constant<Mean> {
    public:
        FixedMean(Mean mean = static_cast<Mean>(0.0)): mkt::util::constant<Mean>(mean) {}
    };
    
    template <typename Volatility>
    class FixedVolatility: public mkt::util::constant<Volatility> {
    public:
        FixedVolatility(Volatility volatility = static_cast<Volatility>(1.0)): mkt::util::constant<Volatility>(volatility) {}
    };

}
}

#endif
