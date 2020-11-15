//
//  OHLC.hpp
//  Market
//
//  Created by Blagovest on 08/11/20.
//

#ifndef Util_OHLC_hpp
#define Util_OHLC_hpp

#include <iostream>

namespace mkt {
namespace util {
    class LowPriceEvaluationStrategy;
    class HighPriceEvaluationStrategy;
    class OpenPriceEvaluationStrategy;
    class ClosePriceEvaluationStrategy;
    class OpenCloseAveragePriceEvaluationStrategy;
    class LowHighPriceEvaluationStrategy;
    
    template <typename Price, typename PriceEvaluationPolicy = ClosePriceEvaluationStrategy>
    class CandleStick;

    template <typename Price, typename PriceEvaluationPolicy>
    class CandleStick {
        bool is_open;
        Price _open, _high, _low, _close;
    public:
        constexpr CandleStick(): is_open(false), _open(), _high(), _low(), _close() {}
        
        constexpr void update(Price price) noexcept {
            if (!is_open) {
                _open = _low = _high = _close = price;
                is_open = true;
                return;
            }
            
            _close = price;
            
            if (price < _low) {
                _low = price;
            } else if (price > _high) {
                _high = price;
            }
        }
        
        constexpr void reset() noexcept { is_open = false; }
        constexpr operator bool () const { return is_open; }
        constexpr Price open() const noexcept { return _open; }
        constexpr Price high() const noexcept { return _high; }
        constexpr Price low() const noexcept { return _low; }
        constexpr Price close() const noexcept { return _close; }
        constexpr auto range() const noexcept { return _high - _low; }
        constexpr auto price() const noexcept { return PriceEvaluationPolicy()(*this); }
        constexpr bool overlaps(const CandleStick & other) const {
            return other.is_open && is_open && !(_high < other._low || other._high < _low);
        }
        
        friend std::ostream & operator << (std::ostream & out, const CandleStick & cs) {
            return out << "CandleStick(open=" << cs.open() << ", low=" << cs.low() << ", high=" << cs.high() << ", close=" << cs.close() << ")";
        }
        
        friend PriceEvaluationPolicy;
    };

    class LowPriceEvaluationStrategy {
    public:
        template <typename CandleStick>
        auto operator() (const CandleStick & candle_stick) const {
            return candle_stick.low();
        }
    };

    class HighPriceEvaluationStrategy {
    public:
        template <typename CandleStick>
        auto operator() (const CandleStick & candle_stick) const {
            return candle_stick.high();
        }
    };

    class OpenPriceEvaluationStrategy {
    public:
        template <typename CandleStick>
        auto operator() (const CandleStick & candle_stick) const {
            return candle_stick.open();
        }
    };

    class ClosePriceEvaluationStrategy {
    public:
        template <typename CandleStick>
        auto operator() (const CandleStick & candle_stick) const {
            return candle_stick.close();
        }
    };

    class OpenCloseAveragePriceEvaluationStrategy {
    public:
        template <typename CandleStick>
        auto operator() (const CandleStick & candle_stick) const {
            return 0.5 * candle_stick.open() + 0.5 * candle_stick.close();
        }
    };

    class LowHighPriceEvaluationStrategy {
    public:
        template <typename CandleStick>
        auto operator() (const CandleStick & candle_stick) const {
            return 0.5 * candle_stick.low() + 0.5 * candle_stick.high();
        }
    };

}
}


#endif /* OHLC_hpp */
