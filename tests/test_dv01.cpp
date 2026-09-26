#include <gtest/gtest.h>

#include "pricing_primitives/market/market_data.hpp"
#include "pricing_primitives/rates/bootstrap.hpp"
#include "pricing_primitives/rates/interest_rate_swap.hpp"
#include "pricing_primitives/rates/interpolation.hpp"
#include "pricing_primitives/rates/yield_curve.hpp"
#include "pricing_primitives/risk/dv01.hpp"

TEST(Dv01Test, CalculatesOneYearDv01) {
    constexpr double notional = 1'000'000.0;
    constexpr double fixed_rate = 0.035;
    constexpr double discount_rate = 0.03;
    constexpr double projection_rate = 0.04;
    constexpr double bump = 1e-4;

    const pricing_primitives::MarketData market_data{
        .discount_curve = {{
            {pricing_primitives::InstrumentType::OIS, 1.0, discount_rate},
        }},
        .projection_curve = {{{pricing_primitives::InstrumentType::IRS, 1.0, projection_rate}}},
    };

    const auto discount_nodes =
        pricing_primitives::CurveBootstrapper::bootstrap(market_data.discount_curve);
    const pricing_primitives::YieldCurve<pricing_primitives::LogLinearDiscountInterpolator>
        discount_curve(discount_nodes);
    const auto projection_nodes = pricing_primitives::CurveBootstrapper::bootstrap_projection_curve(
        market_data.projection_curve, discount_curve);
    const pricing_primitives::YieldCurve<pricing_primitives::LogLinearDiscountInterpolator>
        projection_curve(projection_nodes);

    const pricing_primitives::InterestRateSwap swap{notional,
                                                    fixed_rate,
                                                    1.0,
                                                    pricing_primitives::PaymentFrequency::Annual,
                                                    pricing_primitives::PaymentFrequency::Annual,
                                                    pricing_primitives::SwapSide::Payer};
    const auto result = dv01(swap, market_data, discount_curve, projection_curve, bump);

    const double expected_discount_dv01 =
        notional * (projection_rate - fixed_rate) / 2.0 *
        (1.0 / (1.0 + discount_rate + bump) - 1.0 / (1.0 + discount_rate - bump));
    const double expected_projection_dv01 = notional / (1.0 + discount_rate) * 1e-4;

    EXPECT_NEAR(result.discount_curve, expected_discount_dv01, 1e-8);
    EXPECT_NEAR(result.projection_curve, expected_projection_dv01, 1e-8);
}

TEST(Dv01Test, BucketedDv01SumsToParallelDv01) {
    constexpr double notional = 1'000'000.0;
    constexpr double fixed_rate = 0.035;
    constexpr double bump = 1e-4;

    const pricing_primitives::MarketData market_data{
        .discount_curve = {{
            {pricing_primitives::InstrumentType::OIS, 1.0, 0.03},
            {pricing_primitives::InstrumentType::OIS, 2.0, 0.035},
        }},
        .projection_curve = {{
            {pricing_primitives::InstrumentType::IRS, 1.0, 0.04},
            {pricing_primitives::InstrumentType::IRS, 2.0, 0.045},
        }},
    };

    const auto discount_nodes =
        pricing_primitives::CurveBootstrapper::bootstrap(market_data.discount_curve);
    const pricing_primitives::YieldCurve<pricing_primitives::LogLinearDiscountInterpolator>
        discount_curve(discount_nodes);
    const auto projection_nodes = pricing_primitives::CurveBootstrapper::bootstrap_projection_curve(
        market_data.projection_curve, discount_curve);
    const pricing_primitives::YieldCurve<pricing_primitives::LogLinearDiscountInterpolator>
        projection_curve(projection_nodes);

    const pricing_primitives::InterestRateSwap swap{notional,
                                                    fixed_rate,
                                                    1.0,
                                                    pricing_primitives::PaymentFrequency::Annual,
                                                    pricing_primitives::PaymentFrequency::Annual,
                                                    pricing_primitives::SwapSide::Payer};

    const auto parallel = dv01(swap, market_data, discount_curve, projection_curve, bump);
    const auto bucket_1y = pricing_primitives::bucketed_dv01(swap, market_data, discount_curve,
                                                             projection_curve, 1.0, bump);
    const auto bucket_2y = pricing_primitives::bucketed_dv01(swap, market_data, discount_curve,
                                                             projection_curve, 2.0, bump);

    ASSERT_TRUE(bucket_1y.discount_curve.has_value());
    ASSERT_TRUE(bucket_2y.discount_curve.has_value());
    ASSERT_TRUE(bucket_1y.projection_curve.has_value());
    ASSERT_TRUE(bucket_2y.projection_curve.has_value());

    EXPECT_NEAR(bucket_1y.discount_curve.value() + bucket_2y.discount_curve.value(),
                parallel.discount_curve, 1e-4);
    EXPECT_NEAR(bucket_1y.projection_curve.value() + bucket_2y.projection_curve.value(),
                parallel.projection_curve, 1e-4);
}

TEST(Dv01Test, BucketedDV01HandlesMissingProjectedQuote) {
    constexpr double notional = 1'000'000.0;
    constexpr double fixed_rate = 0.035;
    constexpr double bump = 1e-4;

    const pricing_primitives::MarketData market_data{
        .discount_curve = {{
            {pricing_primitives::InstrumentType::OIS, 1.0, 0.03},
            {pricing_primitives::InstrumentType::OIS, 2.0, 0.035},
            {pricing_primitives::InstrumentType::OIS, 3.0, 0.037},
        }},
        .projection_curve = {{{pricing_primitives::InstrumentType::IRS, 1.0, 0.04},
                              {pricing_primitives::InstrumentType::IRS, 2.0, 0.045}}},
    };

    const auto discount_nodes =
        pricing_primitives::CurveBootstrapper::bootstrap(market_data.discount_curve);
    const pricing_primitives::YieldCurve<pricing_primitives::LogLinearDiscountInterpolator>
        discount_curve(discount_nodes);
    const auto projection_nodes = pricing_primitives::CurveBootstrapper::bootstrap_projection_curve(
        market_data.projection_curve, discount_curve);
    const pricing_primitives::YieldCurve<pricing_primitives::LogLinearDiscountInterpolator>
        projection_curve(projection_nodes);

    const pricing_primitives::InterestRateSwap swap{notional,
                                                    fixed_rate,
                                                    2.0,
                                                    pricing_primitives::PaymentFrequency::Annual,
                                                    pricing_primitives::PaymentFrequency::Annual,
                                                    pricing_primitives::SwapSide::Payer};
    const auto result =
        bucketed_dv01(swap, market_data, discount_curve, projection_curve, 3.0, bump);

    EXPECT_TRUE(result.discount_curve.has_value());
    EXPECT_FALSE(result.projection_curve.has_value());
}
