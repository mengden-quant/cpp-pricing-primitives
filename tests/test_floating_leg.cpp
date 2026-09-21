#include <gtest/gtest.h>

#include <vector>

#include "pricing_primitives/rates/floating_leg.hpp"
#include "pricing_primitives/rates/interpolation.hpp"
#include "pricing_primitives/rates/yield_curve.hpp"

TEST(FloatingLegTest, CalculatesAnnualFloatingLegPv) {
    const std::vector<pricing_primitives::CurveNode> discount_nodes{
        {1.0, 0.96},
        {2.0, 0.92},
    };
    const std::vector<pricing_primitives::CurveNode> projection_nodes{
        {1.0, 0.95},
        {2.0, 0.89},
    };
    const pricing_primitives::YieldCurve<pricing_primitives::LogLinearDiscountInterpolator>
        discount_curve(discount_nodes);
    const pricing_primitives::YieldCurve<pricing_primitives::LogLinearDiscountInterpolator>
        projection_curve(projection_nodes);
    const double pv = pricing_primitives::floating_leg_pv(
        1'000'000.0, 2.0, pricing_primitives::PaymentFrequency::Annual, discount_curve,
        projection_curve);
    const double first_forward = 1.0 / 0.95 - 1.0;
    const double second_forward = 0.95 / 0.89 - 1.0;
    const double expected = 1'000'000.0 * (first_forward * 0.96 + second_forward * 0.92);
    EXPECT_NEAR(pv, expected, 1e-10);
}

TEST(FloatingLegTest, CalculatesSemiAnnualFloatingLegPv) {
    const std::vector<pricing_primitives::CurveNode> nodes{
        {0.5, 0.98},
        {1.0, 0.95},
    };
    const pricing_primitives::YieldCurve<pricing_primitives::LogLinearDiscountInterpolator> curve(
        nodes);
    const double pv = pricing_primitives::floating_leg_pv(
        1'000'000.0, 1.0, pricing_primitives::PaymentFrequency::SemiAnnual, curve, curve);
    const double expected = 1'000'000.0 * (1.0 - 0.95);
    EXPECT_DOUBLE_EQ(curve.discount(0.0), 1.0);

    EXPECT_DOUBLE_EQ(curve.discount(0.5), 0.98);

    EXPECT_DOUBLE_EQ(curve.discount(1.0), 0.95);
    EXPECT_NEAR(pv, expected, 1e-10);
}

TEST(FloatingLegTest, MatchesSingleCurveIdentity) {
    const std::vector<pricing_primitives::CurveNode> nodes{
        {1.0, 0.97}, {2.0, 0.94}, {3.0, 0.90}, {4.0, 0.86}, {5.0, 0.82},
    };
    const pricing_primitives::YieldCurve<pricing_primitives::LogLinearDiscountInterpolator> curve(
        nodes);
    const double pv = pricing_primitives::floating_leg_pv(
        1'000'000.0, 5.0, pricing_primitives::PaymentFrequency::Annual, curve, curve);
    const double expected = 1'000'000.0 * (1.0 - curve.discount(5.0));
    EXPECT_NEAR(pv, expected, 1e-10);
}

TEST(FloatingLegTest, ScalesLinearlyWithNotional) {
    const std::vector<pricing_primitives::CurveNode> nodes{
        {1.0, 0.97},
        {2.0, 0.94},
    };
    const pricing_primitives::YieldCurve<pricing_primitives::LogLinearDiscountInterpolator> curve(
        nodes);
    const double pv = pricing_primitives::floating_leg_pv(
        1'000'000.0, 2.0, pricing_primitives::PaymentFrequency::Annual, curve, curve);
    const double doubled_pv = pricing_primitives::floating_leg_pv(
        2'000'000.0, 2.0, pricing_primitives::PaymentFrequency::Annual, curve, curve);
    EXPECT_NEAR(doubled_pv, 2.0 * pv, 1e-10);
}
