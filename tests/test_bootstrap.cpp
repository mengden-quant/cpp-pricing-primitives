#include <gtest/gtest.h>

#include <cmath>

#include "pricing_primitives/rates/bootstrap.hpp"

TEST(DiscountCurveBootstrapperTest, BootstrapsOneYearDiscountFactor) {
    const pricing_primitives::CurveMarketData market_data{
        {{pricing_primitives::InstrumentType::OIS, 1.0, 0.05}}};
    const auto nodes = pricing_primitives::DiscountCurveBootstrapper::bootstrap(market_data);
    ASSERT_EQ(nodes.size(), 1);
    EXPECT_DOUBLE_EQ(nodes[0].maturity, 1.0);
    EXPECT_NEAR(nodes[0].discount_factor, 1.0 / 1.05, 1e-12);
}

TEST(DiscountCurveBootstrapperTest, BootstrapsTwoYearDiscountFactor) {
    const pricing_primitives::CurveMarketData market_data{
        {{pricing_primitives::InstrumentType::OIS, 1.0, 0.05},
         {pricing_primitives::InstrumentType::OIS, 2.0, 0.06}}};
    const auto nodes = pricing_primitives::DiscountCurveBootstrapper::bootstrap(market_data);
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
    const auto nodes = pricing_primitives::DiscountCurveBootstrapper::bootstrap(market_data);
    ASSERT_EQ(nodes.size(), market_data.quotes.size());
    double annuity = 0.0;
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        annuity += nodes[i].discount_factor;
        const double implied_par_rate = (1.0 - nodes[i].discount_factor) / annuity;
        EXPECT_NEAR(implied_par_rate, market_data.quotes[i].rate, 1e-12);
    }
}

TEST(DiscountCurveBootstrapperTest, RejectsEmptyMarketData) {
    EXPECT_THROW(pricing_primitives::DiscountCurveBootstrapper::bootstrap({}),
                 std::invalid_argument);
}

TEST(DiscountCurveBootstrapperTest, RejectsNonOisInstrument) {
    const pricing_primitives::CurveMarketData market_data{
        {{pricing_primitives::InstrumentType::IRS, 1.0, 0.03}}};
    EXPECT_THROW(pricing_primitives::DiscountCurveBootstrapper::bootstrap(market_data),
                 std::invalid_argument);
}

TEST(DiscountCurveBootstrapperTest, RejectsNonConsecutiveMaturities) {
    const pricing_primitives::CurveMarketData market_data{
        {{pricing_primitives::InstrumentType::OIS, 1.0, 0.03},
         {pricing_primitives::InstrumentType::OIS, 3.0, 0.04}}};
    EXPECT_THROW(pricing_primitives::DiscountCurveBootstrapper::bootstrap(market_data),
                 std::invalid_argument);
}

TEST(DiscountCurveBootstrapperTest, RejectsMaturityNotStartingAtOneYear) {
    const pricing_primitives::CurveMarketData market_data{
        {{pricing_primitives::InstrumentType::OIS, 2.0, 0.03}}};
    EXPECT_THROW(pricing_primitives::DiscountCurveBootstrapper::bootstrap(market_data),
                 std::invalid_argument);
}
