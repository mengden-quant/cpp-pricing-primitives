#include "pricing_primitives/black_scholes/black_scholes.hpp"

#include <cmath>
#include <stdexcept>

namespace pricing_primitives {
namespace {

void validate_inputs(double spot, double strike, double volatility, double expiry) {
    if (spot <= 0.0) {
        throw std::invalid_argument("'spot' must be positive!");
    }
    if (strike <= 0.0) {
        throw std::invalid_argument("'strike' must be positive!");
    }
    if (volatility <= 0.0) {
        throw std::invalid_argument("'volatility' must be positive!");
    }
    if (expiry <= 0.0) {
        throw std::invalid_argument("'expiry' must be positive!");
    }
}

double normal_cdf(double x) {
    return 0.5 * std::erfc(-x / std::sqrt(2.0));
}

double calc_d1(double spot, double strike, double risk_free_rate, double dividend,
               double volatility, double expiry) {
    return (std::log(spot / strike) +
            (risk_free_rate - dividend + 0.5 * volatility * volatility) * expiry) /
           (volatility * std::sqrt(expiry));
}

double calc_d2(double spot, double strike, double risk_free_rate, double dividend,
               double volatility, double expiry) {
    return calc_d1(spot, strike, risk_free_rate, dividend, volatility, expiry) -
           volatility * std::sqrt(expiry);
}

double vanilla_price(double spot, double strike, double risk_free_rate, double dividend,
                     double volatility, double expiry, OptionType option_type) {
    const double d1 = calc_d1(spot, strike, risk_free_rate, dividend, volatility, expiry);
    const double d2 = calc_d2(spot, strike, risk_free_rate, dividend, volatility, expiry);
    const double discounted_spot = spot * std::exp(-dividend * expiry);
    const double discounted_strike = strike * std::exp(-risk_free_rate * expiry);
    switch (option_type) {
        case OptionType::Call:
            return discounted_spot * normal_cdf(d1) - discounted_strike * normal_cdf(d2);
        case OptionType::Put:
            return discounted_strike * normal_cdf(-d2) - discounted_spot * normal_cdf(-d1);
    }
    throw std::invalid_argument("unknown option type!");
}

double digital_price(double spot, double strike, double risk_free_rate, double dividend,
                     double volatility, double expiry, OptionType option_type) {
    const double d2 = calc_d2(spot, strike, risk_free_rate, dividend, volatility, expiry);
    const double discount_factor = std::exp(-risk_free_rate * expiry);
    switch (option_type) {
        case OptionType::Call:
            return discount_factor * normal_cdf(d2);
        case OptionType::Put:
            return discount_factor * normal_cdf(-d2);
    }
    throw std::invalid_argument("unknown option type!");
}
}  // namespace

double black_scholes_price(double spot, double strike, double risk_free_rate, double dividend,
                           double volatility, double expiry, OptionType option_type,
                           PayoffType payoff_type) {
    validate_inputs(spot, strike, volatility, expiry);
    switch (payoff_type) {
        case PayoffType::Vanilla:
            return vanilla_price(spot, strike, risk_free_rate, dividend, volatility, expiry,
                                 option_type);
        case PayoffType::Digital:
            return digital_price(spot, strike, risk_free_rate, dividend, volatility, expiry,
                                 option_type);
    }
    throw std::invalid_argument("unknown payoff type!");
}
}  // namespace pricing_primitives
