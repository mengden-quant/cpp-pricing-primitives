#include <gtest/gtest.h>

#include "pricing_primitives/market/market_data.hpp"
#include "pricing_primitives/rates/bootstrap.hpp"
#include "pricing_primitives/rates/interest_rate_swap.hpp"
#include "pricing_primitives/rates/interpolation.hpp"
#include "pricing_primitives/rates/irs_pricer.hpp"
#include "pricing_primitives/rates/yield_curve.hpp"
#include "pricing_primitives/risk/scenario.hpp"

TEST(ScenarioTest, ZeroShockPreservesNpv) {
    constexpr double notional = 1'000'000.0;
    constexpr double fixed_rate = 0.035;

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

    const pricing_primitives::RateScenario scenario{.discount_curve_shift = 0.0,
                                                    .projection_curve_shift = 0.0};

    const auto result =
        pricing_primitives::calculate_scenario<pricing_primitives::LogLinearDiscountInterpolator,
                                               pricing_primitives::LogLinearDiscountInterpolator>(
            swap, market_data, scenario);
    const double expected_npv =
        pricing_primitives::interest_rate_swap_npv(swap, discount_curve, projection_curve);

    EXPECT_NEAR(result.scenario_npv, expected_npv, 1e-8);
}

TEST(ScenarioTest, CalculatesOneYearScenarioNpv) {
    constexpr double notional = 1'000'000;
    constexpr double fixed_rate = 0.035;
    constexpr double discount_rate = 0.03;
    constexpr double projection_rate = 0.04;

    const pricing_primitives::MarketData market_data{
        .discount_curve = {{
            {pricing_primitives::InstrumentType::OIS, 1.0, discount_rate},
        }},
        .projection_curve = {{
            {pricing_primitives::InstrumentType::IRS, 1.0, projection_rate},
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

    const pricing_primitives::RateScenario scenario{.discount_curve_shift = 0.01,
                                                    .projection_curve_shift = 0.005};

    const auto result =
        pricing_primitives::calculate_scenario<pricing_primitives::LogLinearDiscountInterpolator,
                                               pricing_primitives::LogLinearDiscountInterpolator>(
            swap, market_data, scenario);
    const double expected_npv = notional *
                                (projection_rate + scenario.projection_curve_shift - fixed_rate) /
                                (1.0 + discount_rate + scenario.discount_curve_shift);

    EXPECT_NEAR(result.scenario_npv, expected_npv, 1e-8);
}
