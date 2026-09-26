#include <gtest/gtest.h>

#include <vector>

#include "pricing_primitives/market/market_data.hpp"
#include "pricing_primitives/rates/interest_rate_swap.hpp"
#include "pricing_primitives/rates/interpolation.hpp"
#include "pricing_primitives/risk/scenario.hpp"
#include "pricing_primitives/risk/scenario_engine.hpp"

TEST(ScenarioEngineTest, SequentialAndParallelResultsMatch) {
    constexpr double notional = 1'000'000.0;
    constexpr double fixed_rate = 0.035;

    const pricing_primitives::MarketData market_data{
        .discount_curve = {{
            {pricing_primitives::InstrumentType::OIS, 1.0, 0.031},
            {pricing_primitives::InstrumentType::OIS, 2.0, 0.032},
            {pricing_primitives::InstrumentType::OIS, 3.0, 0.033},
            {pricing_primitives::InstrumentType::OIS, 4.0, 0.034},
            {pricing_primitives::InstrumentType::OIS, 5.0, 0.035},
        }},
        .projection_curve = {{
            {pricing_primitives::InstrumentType::IRS, 1.0, 0.034},
            {pricing_primitives::InstrumentType::IRS, 2.0, 0.035},
            {pricing_primitives::InstrumentType::IRS, 3.0, 0.036},
            {pricing_primitives::InstrumentType::IRS, 4.0, 0.037},
            {pricing_primitives::InstrumentType::IRS, 5.0, 0.038},
        }},
    };

    const pricing_primitives::InterestRateSwap swap{notional,
                                                    fixed_rate,
                                                    5.0,
                                                    pricing_primitives::PaymentFrequency::Annual,
                                                    pricing_primitives::PaymentFrequency::Annual,
                                                    pricing_primitives::SwapSide::Payer};

    const std::vector<pricing_primitives::RateScenario> scenarios{
        {0.0, 0.0},     {0.001, 0.001},  {-0.001, -0.001}, {0.01, 0.01},
        {-0.01, -0.01}, {0.001, 0.0},    {0.0, 0.001},     {-0.001, 0.0},
        {0.0, -0.001},  {0.005, -0.005}, {-0.005, 0.005},
    };

    const auto sequential = pricing_primitives::calculate_scenarios_sequential<
        pricing_primitives::LogLinearDiscountInterpolator,
        pricing_primitives::LogLinearDiscountInterpolator>(swap, market_data, scenarios);
    const auto parallel = pricing_primitives::calculate_scenarios_parallel<
        pricing_primitives::LogLinearDiscountInterpolator,
        pricing_primitives::LogLinearDiscountInterpolator>(swap, market_data, scenarios);

    ASSERT_EQ(sequential.size(), scenarios.size());
    ASSERT_EQ(parallel.size(), scenarios.size());

    for (std::size_t i = 0; i < scenarios.size(); ++i) {
        EXPECT_DOUBLE_EQ(sequential[i].scenario_npv, parallel[i].scenario_npv);
    }
}

TEST(ScenarioEngineTest, EmptyScenarioListReturnsEmptyResults) {
    constexpr double notional = 1'000'000.0;
    constexpr double fixed_rate = 0.035;

    const pricing_primitives::MarketData market_data{
        .discount_curve = {{
            {pricing_primitives::InstrumentType::OIS, 1.0, 0.031},
        }},
        .projection_curve = {{
            {pricing_primitives::InstrumentType::IRS, 1.0, 0.034},
        }},
    };

    const pricing_primitives::InterestRateSwap swap{notional,
                                                    fixed_rate,
                                                    1.0,
                                                    pricing_primitives::PaymentFrequency::Annual,
                                                    pricing_primitives::PaymentFrequency::Annual,
                                                    pricing_primitives::SwapSide::Payer};

    const std::vector<pricing_primitives::RateScenario> scenarios;

    const auto sequential = pricing_primitives::calculate_scenarios_sequential<
        pricing_primitives::LogLinearDiscountInterpolator,
        pricing_primitives::LogLinearDiscountInterpolator>(swap, market_data, scenarios);
    const auto parallel = pricing_primitives::calculate_scenarios_parallel<
        pricing_primitives::LogLinearDiscountInterpolator,
        pricing_primitives::LogLinearDiscountInterpolator>(swap, market_data, scenarios);

    EXPECT_TRUE(sequential.empty());
    EXPECT_TRUE(parallel.empty());
}
