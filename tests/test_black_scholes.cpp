#include <gtest/gtest.h>

#include <cmath>

#include "pricing_primitives/black_scholes/black_scholes.hpp"

namespace {
constexpr double spot = 100.0;
constexpr double strike = 100.0;
constexpr double risk_free_rate = 0.05;
constexpr double dividend = 0.02;
constexpr double volatility = 0.20;
constexpr double expiry = 1.0;
constexpr int num_steps = 500;

constexpr double parity_tolerance = 1e-3;
constexpr double model_tolerance = 1e-1;
}  // namespace

TEST(BlackScholesTest, EuropeanCallPriceIsFiniteAndWithinBounds) {
    const double call_price =
        pricing_primitives::black_scholes_price(spot, strike, risk_free_rate, dividend, volatility,
                                                expiry, pricing_primitives::OptionType::Call);
    EXPECT_TRUE(std::isfinite(call_price));
    EXPECT_GT(call_price, 0.0);
    EXPECT_LT(call_price, spot);
}

TEST(BlackScholesTest, EuropeanPutCallParityHolds) {
    const double call_price =
        pricing_primitives::black_scholes_price(spot, strike, risk_free_rate, dividend, volatility,
                                                expiry, pricing_primitives::OptionType::Call);
    const double put_price =
        pricing_primitives::black_scholes_price(spot, strike, risk_free_rate, dividend, volatility,
                                                expiry, pricing_primitives::OptionType::Put);
    const double lhs = call_price - put_price;
    const double rhs =
        spot * std::exp(-dividend * expiry) - strike * std::exp(-risk_free_rate * expiry);
    EXPECT_NEAR(lhs, rhs, parity_tolerance);
}

TEST(BlackScholesTest, MatchesKnownReferencePrice) {
    constexpr double expected_call_price = 9.227005508;
    constexpr double tolerance = 1e-9;
    const double call_price = pricing_primitives::black_scholes_price(
        100.0, 100.0, 0.05, 0.02, 0.20, 1.0, pricing_primitives::OptionType::Call);
    EXPECT_NEAR(call_price, expected_call_price, tolerance);
}

TEST(BlackScholesTest, EuropeanPutMatchesKnownReferencePrice) {
    constexpr double expected_put_price = 6.330080628;
    constexpr double tolerance = 1e-9;
    const double put_price = pricing_primitives::black_scholes_price(
        100.0, 100.0, 0.05, 0.02, 0.20, 1.0, pricing_primitives::OptionType::Put);
    EXPECT_NEAR(put_price, expected_put_price, tolerance);
}
