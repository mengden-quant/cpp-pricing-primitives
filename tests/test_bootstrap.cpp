#include <gtest/gtest.h>

#include <cmath>

#include "pricing_primitives/rates/bootstrap.hpp"
#include "pricing_primitives/rates/interest_rate_swap.hpp"
#include "pricing_primitives/rates/interpolation.hpp"
#include "pricing_primitives/rates/irs_pricer.hpp"
#include "pricing_primitives/rates/yield_curve.hpp"

TEST(DiscountCurveBootstrapperTest, BootstrapsOneYearDiscountFactor) {
    const pricing_primitives::CurveMarketData market_data{
        {{pricing_primitives::InstrumentType::OIS, 1.0, 0.05}}};
    const auto nodes = pricing_primitives::CurveBootstrapper::bootstrap(market_data);
    ASSERT_EQ(nodes.size(), 1);
    EXPECT_DOUBLE_EQ(nodes[0].maturity, 1.0);
    EXPECT_NEAR(nodes[0].discount_factor, 1.0 / 1.05, 1e-12);
}

TEST(DiscountCurveBootstrapperTest, BootstrapsTwoYearDiscountFactor) {
    const pricing_primitives::CurveMarketData market_data{
        {{pricing_primitives::InstrumentType::OIS, 1.0, 0.05},
         {pricing_primitives::InstrumentType::OIS, 2.0, 0.06}}};
    const auto nodes = pricing_primitives::CurveBootstrapper::bootstrap(market_data);
    ASSERT_EQ(nodes.size(), 2);
    const double expected_p1 = 1.0 / 1.05;
    const double expected_p2 = (1.0 - 0.06 * expected_p1) / 1.06;
    EXPECT_NEAR(nodes[0].discount_factor, expected_p1, 1e-12);
    EXPECT_NEAR(nodes[1].discount_factor, expected_p2, 1e-12);
}

TEST(DiscountCurveBootstrapperTest, RepricesCalibrationQuotes) {
    const pricing_primitives::CurveMarketData market_data{
        {{pricing_primitives::InstrumentType::OIS, 1.0, 0.031},
         {pricing_primitives::InstrumentType::OIS, 2.0, 0.032},
         {pricing_primitives::InstrumentType::OIS, 3.0, 0.033},
         {pricing_primitives::InstrumentType::OIS, 4.0, 0.034},
         {pricing_primitives::InstrumentType::OIS, 5.0, 0.035}}};
    const auto nodes = pricing_primitives::CurveBootstrapper::bootstrap(market_data);
    ASSERT_EQ(nodes.size(), market_data.quotes.size());
    double annuity = 0.0;
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        annuity += nodes[i].discount_factor;
        const double implied_par_rate = (1.0 - nodes[i].discount_factor) / annuity;
        EXPECT_NEAR(implied_par_rate, market_data.quotes[i].rate, 1e-12);
    }
}

TEST(DiscountCurveBootstrapperTest, RejectsEmptyMarketData) {
    EXPECT_THROW(pricing_primitives::CurveBootstrapper::bootstrap({}), std::invalid_argument);
}

TEST(DiscountCurveBootstrapperTest, RejectsNonOisInstrument) {
    const pricing_primitives::CurveMarketData market_data{
        {{pricing_primitives::InstrumentType::IRS, 1.0, 0.03}}};
    EXPECT_THROW(pricing_primitives::CurveBootstrapper::bootstrap(market_data),
                 std::invalid_argument);
}

TEST(DiscountCurveBootstrapperTest, RejectsNonConsecutiveMaturities) {
    const pricing_primitives::CurveMarketData market_data{
        {{pricing_primitives::InstrumentType::OIS, 1.0, 0.03},
         {pricing_primitives::InstrumentType::OIS, 3.0, 0.04}}};
    EXPECT_THROW(pricing_primitives::CurveBootstrapper::bootstrap(market_data),
                 std::invalid_argument);
}

TEST(DiscountCurveBootstrapperTest, RejectsMaturityNotStartingAtOneYear) {
    const pricing_primitives::CurveMarketData market_data{
        {{pricing_primitives::InstrumentType::OIS, 2.0, 0.03}}};
    EXPECT_THROW(pricing_primitives::CurveBootstrapper::bootstrap(market_data),
                 std::invalid_argument);
}

TEST(ProjectionCurveBootstrapTest, BootstrapsOneYearProjectionDiscountFactor) {
    const pricing_primitives::CurveMarketData discount_market_data{{{
        pricing_primitives::InstrumentType::OIS,
        1.0,
        0.03,
    }}};
    const auto discount_nodes =
        pricing_primitives::CurveBootstrapper::bootstrap(discount_market_data);
    const pricing_primitives::YieldCurve<pricing_primitives::LogLinearDiscountInterpolator>
        discount_curve(discount_nodes);
    const pricing_primitives::CurveMarketData projection_market_data{{{
        pricing_primitives::InstrumentType::IRS,
        1.0,
        0.04,
    }}};
    const auto projection_nodes = pricing_primitives::CurveBootstrapper::bootstrap_projection_curve(
        projection_market_data, discount_curve);
    ASSERT_EQ(projection_nodes.size(), 1);
    const double expected = 1.0 / 1.04;
    EXPECT_NEAR(projection_nodes[0].discount_factor, expected, 1e-10);
}

TEST(ProjectionCurveBootstrapTest, BootstrapsTwoYearProjectionCurve) {
    const pricing_primitives::CurveMarketData discount_market_data{{
        {pricing_primitives::InstrumentType::OIS, 1.0, 0.03},
        {pricing_primitives::InstrumentType::OIS, 2.0, 0.035},
    }};
    const auto discount_nodes =
        pricing_primitives::CurveBootstrapper::bootstrap(discount_market_data);
    const pricing_primitives::YieldCurve<pricing_primitives::LogLinearDiscountInterpolator>
        discount_curve(discount_nodes);
    const pricing_primitives::CurveMarketData projection_market_data{{
        {pricing_primitives::InstrumentType::IRS, 1.0, 0.04},
        {pricing_primitives::InstrumentType::IRS, 2.0, 0.045},
    }};
    const auto projection_nodes = pricing_primitives::CurveBootstrapper::bootstrap_projection_curve(
        projection_market_data, discount_curve);
    ASSERT_EQ(projection_nodes.size(), 2);
    const double p1 = 1.0 / 1.04;
    const double pd1 = discount_curve.discount(1.0);
    const double pd2 = discount_curve.discount(2.0);
    const double known_floating_pv = (1.0 / p1 - 1.0) * pd1;
    const double fixed_annuity = pd1 + pd2;
    const double last_floating_component = (0.045 * fixed_annuity - known_floating_pv) / pd2;
    const double p2 = p1 / (1.0 + last_floating_component);
    EXPECT_NEAR(projection_nodes[0].discount_factor, p1, 1e-10);
    EXPECT_NEAR(projection_nodes[1].discount_factor, p2, 1e-10);
}

TEST(ProjectionCurveBootstrapTest, RepricesMarketIrsQuotes) {
    const pricing_primitives::CurveMarketData discount_market_data{{
        {pricing_primitives::InstrumentType::OIS, 1.0, 0.031},
        {pricing_primitives::InstrumentType::OIS, 2.0, 0.032},
        {pricing_primitives::InstrumentType::OIS, 3.0, 0.033},
        {pricing_primitives::InstrumentType::OIS, 4.0, 0.034},
        {pricing_primitives::InstrumentType::OIS, 5.0, 0.035},
    }};
    const pricing_primitives::CurveMarketData projection_market_data{{
        {pricing_primitives::InstrumentType::IRS, 1.0, 0.034},
        {pricing_primitives::InstrumentType::IRS, 2.0, 0.035},
        {pricing_primitives::InstrumentType::IRS, 3.0, 0.036},
        {pricing_primitives::InstrumentType::IRS, 4.0, 0.037},
        {pricing_primitives::InstrumentType::IRS, 5.0, 0.038},
    }};
    const auto discount_nodes =
        pricing_primitives::CurveBootstrapper::bootstrap(discount_market_data);
    const pricing_primitives::YieldCurve<pricing_primitives::LogLinearDiscountInterpolator>
        discount_curve(discount_nodes);
    const auto projection_nodes = pricing_primitives::CurveBootstrapper::bootstrap_projection_curve(
        projection_market_data, discount_curve);
    const pricing_primitives::YieldCurve<pricing_primitives::LogLinearDiscountInterpolator>
        projection_curve(projection_nodes);
    for (const auto& quote : projection_market_data.quotes) {
        const pricing_primitives::InterestRateSwap swap{
            1'000'000.0,
            quote.rate,
            quote.maturity,
            pricing_primitives::PaymentFrequency::Annual,
            pricing_primitives::PaymentFrequency::Annual,
            pricing_primitives::SwapSide::Payer,
        };
        const double npv =
            pricing_primitives::interest_rate_swap_npv(swap, discount_curve, projection_curve);
        EXPECT_NEAR(npv, 0.0, 1e-8);
    }
}
