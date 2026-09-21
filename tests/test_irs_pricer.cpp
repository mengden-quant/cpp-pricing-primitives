#include <gtest/gtest.h>

#include <vector>

#include "pricing_primitives/rates/interest_rate_swap.hpp"
#include "pricing_primitives/rates/interpolation.hpp"
#include "pricing_primitives/rates/irs_pricer.hpp"
#include "pricing_primitives/rates/yield_curve.hpp"

TEST(IrsPricerTest, CalculatesPayerSwapNpv) {
    const std::vector<pricing_primitives::CurveNode> nodes{
        {1.0, 0.97},
        {2.0, 0.94},
    };
    const pricing_primitives::YieldCurve<pricing_primitives::LogLinearDiscountInterpolator> curve(
        nodes);
    const pricing_primitives::InterestRateSwap swap{
        1'000'000.0,
        0.03,
        2.0,
        pricing_primitives::PaymentFrequency::Annual,
        pricing_primitives::PaymentFrequency::Annual,
        pricing_primitives::SwapSide::Payer,
    };
    const double npv = pricing_primitives::interest_rate_swap_npv(swap, curve, curve);
    // Fixed leg:
    // 1,000,000 * 3% * (0.97 + 0.94) = 57,300
    const double fixed_pv = 57'300.0;
    // With the same projection and discount curve:
    // floating leg PV = N * (1 - P(T))
    //                 = 1,000,000 * (1 - 0.94)
    //                 = 60,000
    const double floating_pv = 60'000.0;
    // Payer swap: receive floating, pay fixed.
    const double expected = floating_pv - fixed_pv;
    EXPECT_NEAR(npv, expected, 1e-10);
}

TEST(IrsPricerTest, CalculatesReceiverSwapNpv) {
    const std::vector<pricing_primitives::CurveNode> nodes{
        {1.0, 0.97},
        {2.0, 0.94},
    };
    const pricing_primitives::YieldCurve<pricing_primitives::LogLinearDiscountInterpolator> curve(
        nodes);
    const pricing_primitives::InterestRateSwap swap{
        1'000'000.0,
        0.03,
        2.0,
        pricing_primitives::PaymentFrequency::Annual,
        pricing_primitives::PaymentFrequency::Annual,
        pricing_primitives::SwapSide::Receiver,
    };
    const double npv = pricing_primitives::interest_rate_swap_npv(swap, curve, curve);
    const double fixed_pv = 57'300.0;
    const double floating_pv = 60'000.0;
    // Receiver swap: receive fixed, pay floating.
    const double expected = fixed_pv - floating_pv;
    EXPECT_NEAR(npv, expected, 1e-10);
}

TEST(IrsPricerTest, PayerAndReceiverNpvsHaveOppositeSigns) {
    const std::vector<pricing_primitives::CurveNode> discount_nodes{
        {1.0, 0.97},
        {2.0, 0.94},
    };
    const std::vector<pricing_primitives::CurveNode> projection_nodes{
        {1.0, 0.96},
        {2.0, 0.92},
    };
    const pricing_primitives::YieldCurve<pricing_primitives::LogLinearDiscountInterpolator>
        discount_curve(discount_nodes);
    const pricing_primitives::YieldCurve<pricing_primitives::LogLinearDiscountInterpolator>
        projection_curve(projection_nodes);
    const pricing_primitives::InterestRateSwap payer_swap{
        1'000'000.0,
        0.04,
        2.0,
        pricing_primitives::PaymentFrequency::Annual,
        pricing_primitives::PaymentFrequency::Annual,
        pricing_primitives::SwapSide::Payer,
    };
    const pricing_primitives::InterestRateSwap receiver_swap{
        1'000'000.0,
        0.04,
        2.0,
        pricing_primitives::PaymentFrequency::Annual,
        pricing_primitives::PaymentFrequency::Annual,
        pricing_primitives::SwapSide::Receiver,
    };
    const double payer_npv =
        pricing_primitives::interest_rate_swap_npv(payer_swap, discount_curve, projection_curve);
    const double receiver_npv =
        pricing_primitives::interest_rate_swap_npv(receiver_swap, discount_curve, projection_curve);
    EXPECT_NEAR(payer_npv, -receiver_npv, 1e-10);
}
