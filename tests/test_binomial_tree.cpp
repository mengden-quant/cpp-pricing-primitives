#include "pricing_primitives/binomial_tree/binomial_tree.hpp"

#include <cassert>
#include <cmath>

int main() {
    const double spot = 100.0;
    const double strike = 100.0;
    const double risk_free_rate = 0.05;
    const double volatility = 0.20;
    const double expiry = 1.0;
    const int num_steps = 500;

    const double parity_tolerance = 1e-3;
    const double model_tolerance = 1e-1;

    const double call_price = pricing_primitives::price_option(
        spot,
        strike,
        risk_free_rate,
        volatility,
        expiry,
        num_steps,
        pricing_primitives::OptionType::Call
    );

    const double put_price = pricing_primitives::price_option(
        spot,
        strike,
        risk_free_rate,
        volatility,
        expiry,
        num_steps,
        pricing_primitives::OptionType::Put
    );

    const double call_price_mm = pricing_primitives::price_option(
        spot,
        strike,
        risk_free_rate,
        volatility,
        expiry,
        num_steps,
        pricing_primitives::OptionType::Call,
        pricing_primitives::ExerciseType::European,
        pricing_primitives::TreeModel::MomentMatching
    );

    const double call_price_american = pricing_primitives::price_option(
        spot,
        strike,
        risk_free_rate,
        volatility,
        expiry,
        num_steps,
        pricing_primitives::OptionType::Call,
        pricing_primitives::ExerciseType::American
    );

    const double put_price_american = pricing_primitives::price_option(
        spot,
        strike,
        risk_free_rate,
        volatility,
        expiry,
        num_steps,
        pricing_primitives::OptionType::Put,
        pricing_primitives::ExerciseType::American
    );

    assert(std::isfinite(call_price));
    assert(call_price > 0.0);
    assert(call_price < spot);

    const double lhs = call_price - put_price;
    const double rhs = spot - strike * std::exp(-risk_free_rate * expiry);
    assert(std::fabs(lhs - rhs) < parity_tolerance);

    assert(put_price_american >= put_price);
    assert(std::fabs(call_price_american - call_price) < parity_tolerance);

    assert(std::isfinite(call_price_mm));
    assert(call_price_mm > 0.0);
    assert(call_price_mm < spot);
    assert(std::fabs(call_price - call_price_mm) < model_tolerance);

    return 0;
}
