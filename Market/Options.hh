#ifndef OPTIONS_H
#define OPTIONS_H

#include <cmath>
#include <utility>
#include <memory>

template <typename T>
inline T gaussian_cdf(T x) {
    return std::erfc(-x/std::sqrt(2)) / 2;
}

/*
 * Interest rates
 */
template <typename Time, typename FP = long double>
class InterestRateModel {
public:
    virtual ~InterestRateModel() = 0; // force abstract
    virtual FP getInterestRate(Time t) = 0;
};

template <typename Time, typename FP>
InterestRateModel<Time, FP>::~InterestRateModel() {}

template <typename Time, typename FP = long double>
class ConstantInterestRateModel: public InterestRateModel<Time, FP> {
    FP interestRate;
public:
    ConstantInterestRateModel(FP interestRate): interestRate(interestRate) {}

    virtual FP getInterestRate(Time t) {
        return interestRate;
    }

    ~ConstantInterestRateModel() {}
};

/*
 * Volatility
 */
template <typename Time, typename FP = long double>
class VolatilityModel {
public:
    virtual ~VolatilityModel() = 0; // force abstract
    virtual FP getVolatility(Time t) = 0;
};

template <typename Time, typename FP>
VolatilityModel<Time, FP>::~VolatilityModel() {}

template <typename Time, typename FP = long double>
class ConstantVolatilityModel: public VolatilityModel<Time, FP> {
    FP volatility;

public:
    ConstantVolatilityModel(FP volatility): volatility(volatility) {}

    virtual FP getVolatility(Time t) {
        return volatility;
    }

    ~ConstantVolatilityModel() {}
};

/*
 * Option
 */
template <typename Payout, typename Quantity, typename Price, typename Time>
class Option {
    Price strike;
    Time maturity;

    std::shared_ptr<InterestRateModel<Time>> interestRateModel;
    std::shared_ptr<VolatilityModel<Time>> volatilityModel;

public:
    Option(Price strike, Time maturity, 
        std::shared_ptr<VolatilityModel<Time>> volatilityModel,
        std::shared_ptr<InterestRateModel<Time>> interestRateModel)
        : strike(strike), maturity(maturity), interestRateModel(interestRateModel), volatilityModel(volatilityModel) {}
    
    Price getStrike() const {
        return strike;
    }

    Time getMaturity() const {
        return maturity;
    }
};

template <template <typename, typename, typename, typename> typename OptionModel, template <typename, typename> typename Eval, typename Quantity, typename Price, typename Time>
class Call: public OptionModel<Call<OptionModel, Eval, Quantity, Price, Time>, Quantity, Price, Time>, public Eval<Time, Quantity> {

public:
    Call(Price strike, Time maturity,
         std::shared_ptr<VolatilityModel<Time>> volatilityModel,
         std::shared_ptr<InterestRateModel<Time>> interestRateModel): OptionModel<Call, Quantity, Price, Time>(strike, maturity, volatilityModel, interestRateModel) {}
protected:
    auto getFinalPayout(Quantity q) const {
        auto strike = this->getStrike();
        return q > strike ? q - strike : 0;
    }
};

template <template <typename, typename, typename, typename> typename OptionModel, template <typename, typename> typename Eval, typename Quantity, typename Price, typename Time>
class Put: public OptionModel<Put<OptionModel, Eval, Quantity, Price, Time>, Quantity, Price, Time>, public Eval<Time, Quantity>  {
protected:
    auto getFinalPayout(Quantity q) const {
        auto strike = this->getStrike();
        return q < strike ? strike - q : 0;
    }
};

template <typename Time, typename Quantity, bool isCall>
class BlackScholesEvaluator {
    auto operator() (Time t, Quantity q) const {
        static_assert(isCall, "FIXME Puts not implemented");

        if constexpr (isCall) {
            /* 
            * Black-Scholes formula
            * C = N(d1) S(t) - N(d2) K exp(-rt)
            * with d1, d2 as defined in code below
            */
            const long double rate = this->interestRateModel->getInterestRate(t);
            const long double vol = this->volatilityModel->getVolatility(t);
            const long double vol_sq = (vol * vol) / 2.0;
            const long double a = vol * std::sqrt(t);
            const long double d1 = (std::log(q / this->strike) + (rate + vol_sq) * t) / a;
            const long double d2 = d1 - a;

            return gaussian_cdf(d1) * q - gaussian_cdf(d2) * this->strike * std::exp(-rate * t);
        } else {
            
        }
    }
};

template <typename Time, typename Quantity>
using BlackScholesCallEvaluator = BlackScholesEvaluator<Time, Quantity, true>;

template <typename Time, typename Quantity>
using BlackScholesPutEvaluator = BlackScholesEvaluator<Time, Quantity, false>;

// TODO: Implement for Puts

#endif
