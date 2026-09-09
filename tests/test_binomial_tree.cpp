#include <gtest/gtest.h>

#include <cmath>

#include "pricing_primitives/binomial_tree/binomial_tree.hpp"

namespace {
constexpr double spot = 100.0;
constexpr double strike = 100.0;
constexpr double risk_free_rate = 0.05;
constexpr double volatility = 0.20;
constexpr double expiry = 1.0;
constexpr int num_steps = 500;

constexpr double parity_tolerance = 1e-3;
constexpr double model_tolerance = 1e-1;
}  // namespace

TEST(BinomialTreeTest, EuropeanCallPriceIsFiniteAndWithinBounds) {
    const double call_price =
        pricing_primitives::binomial_tree_price(spot, strike, risk_free_rate, volatility, expiry,
                                                num_steps, pricing_primitives::OptionType::Call);
    EXPECT_TRUE(std::isfinite(call_price));
    EXPECT_GT(call_price, 0.0);
    EXPECT_LT(call_price, spot);
}

TEST(BinomialTreeTest, EuropeanPutCallParityHolds) {
    const double call_price =
        pricing_primitives::binomial_tree_price(spot, strike, risk_free_rate, volatility, expiry,
                                                num_steps, pricing_primitives::OptionType::Call);
    const double put_price =
        pricing_primitives::binomial_tree_price(spot, strike, risk_free_rate, volatility, expiry,
                                                num_steps, pricing_primitives::OptionType::Put);
    const double lhs = call_price - put_price;
    const double rhs = spot - strike * std::exp(-risk_free_rate * expiry);
    EXPECT_NEAR(lhs, rhs, parity_tolerance);
}

TEST(BinomialTreeTest, AmericanPutIsNotCheaperThanEuropeanPut) {
    const double european_put =
        pricing_primitives::binomial_tree_price(spot, strike, risk_free_rate, volatility, expiry,
                                                num_steps, pricing_primitives::OptionType::Put);
    const double american_put = pricing_primitives::binomial_tree_price(
        spot, strike, risk_free_rate, volatility, expiry, num_steps,
        pricing_primitives::OptionType::Put, pricing_primitives::ExerciseType::American);
    EXPECT_GE(american_put, european_put);
}

TEST(BinomialTreeTest, AmericanCallMatchesEuropeanCallWithoutDividends) {
    const double european_call =
        pricing_primitives::binomial_tree_price(spot, strike, risk_free_rate, volatility, expiry,
                                                num_steps, pricing_primitives::OptionType::Call);
    const double american_call = pricing_primitives::binomial_tree_price(
        spot, strike, risk_free_rate, volatility, expiry, num_steps,
        pricing_primitives::OptionType::Call, pricing_primitives::ExerciseType::American);
    EXPECT_NEAR(american_call, european_call, parity_tolerance);
}

TEST(BinomialTreeTest, MomentMatchingProducesReasonableCallPrice) {
    const double crr_call =
        pricing_primitives::binomial_tree_price(spot, strike, risk_free_rate, volatility, expiry,
                                                num_steps, pricing_primitives::OptionType::Call);
    const double mm_call = pricing_primitives::binomial_tree_price(
        spot, strike, risk_free_rate, volatility, expiry, num_steps,
        pricing_primitives::OptionType::Call, pricing_primitives::ExerciseType::European,
        pricing_primitives::TreeModel::MomentMatching);
    EXPECT_TRUE(std::isfinite(mm_call));
    EXPECT_GT(mm_call, 0.0);
    EXPECT_LT(mm_call, spot);
    EXPECT_NEAR(crr_call, mm_call, model_tolerance);
}
