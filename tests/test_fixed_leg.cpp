#include <gtest/gtest.h>

#include <vector>

#include "pricing_primitives/rates/fixed_leg.hpp"
#include "pricing_primitives/rates/interpolation.hpp"
#include "pricing_primitives/rates/yield_curve.hpp"

class FlatDiscountCurve {
   public:
    explicit FlatDiscountCurve(double discount_factor) : discount_factor_(discount_factor) {}
    double discount(double /* maturity */) const {
        return discount_factor_;
    }

   private:
    double discount_factor_;
};

TEST(FixedLegTest, CalculatesAnnualFixedLegPv) {
    const FlatDiscountCurve curve(0.95);
    const double pv = pricing_primitives::fixed_leg_pv(
        1'000'000.0, 0.05, 2.0, pricing_primitives::PaymentFrequency::Annual, curve);
    // Two annual coupons:
    // 1,000,000 * 5% * 0.95 * 2 = 95,000
    EXPECT_NEAR(pv, 95'000.0, 1e-10);
}

TEST(FixedLegTest, CalculatesSemiAnnualFixedLegPv) {
    const FlatDiscountCurve curve(0.95);
    const double pv = pricing_primitives::fixed_leg_pv(
        1'000'000.0, 0.05, 2.0, pricing_primitives::PaymentFrequency::SemiAnnual, curve);
    // Four coupons of 25,000 discounted by 0.95.
    EXPECT_NEAR(pv, 95'000.0, 1e-10);
}

TEST(FixedLegTest, ReturnsZeroForZeroFixedRate) {
    const FlatDiscountCurve curve(0.95);
    const double pv = pricing_primitives::fixed_leg_pv(
        1'000'000.0, 0.0, 5.0, pricing_primitives::PaymentFrequency::Annual, curve);
    EXPECT_DOUBLE_EQ(pv, 0.0);
}

TEST(FixedLegTest, ScalesLinearlyWithNotional) {
    const FlatDiscountCurve curve(0.95);
    const double pv = pricing_primitives::fixed_leg_pv(
        1'000'000.0, 0.05, 5.0, pricing_primitives::PaymentFrequency::Annual, curve);
    const double doubled_pv =
        fixed_leg_pv(2'000'000.0, 0.05, 5.0, pricing_primitives::PaymentFrequency::Annual, curve);
    EXPECT_NEAR(doubled_pv, 2.0 * pv, 1e-10);
}

TEST(FixedLegTest, ScalesLinearlyWithFixedRate) {
    const FlatDiscountCurve curve(0.95);
    const double pv = pricing_primitives::fixed_leg_pv(
        1'000'000.0, 0.03, 5.0, pricing_primitives::PaymentFrequency::Annual, curve);
    const double doubled_rate_pv = pricing_primitives::fixed_leg_pv(
        1'000'000.0, 0.06, 5.0, pricing_primitives::PaymentFrequency::Annual, curve);

    EXPECT_NEAR(doubled_rate_pv, 2.0 * pv, 1e-10);
}

TEST(FixedLegTest, WorksWithYieldCurve) {
    const std::vector<pricing_primitives::CurveNode> nodes{{1.0, 0.97}, {2.0, 0.94}};
    const pricing_primitives::YieldCurve<pricing_primitives::LogLinearDiscountInterpolator> curve(
        nodes);
    const double pv = pricing_primitives::fixed_leg_pv(
        1'000'000.0, 0.05, 2.0, pricing_primitives::PaymentFrequency::Annual, curve);
    const double expected = 1'000'000.0 * 0.05 * (0.97 + 0.94);
    EXPECT_NEAR(pv, expected, 1e-10);
}
