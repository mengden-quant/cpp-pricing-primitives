#include <gtest/gtest.h>

#include "pricing_primitives/rates/interpolation.hpp"
#include "pricing_primitives/rates/yield_curve.hpp"

namespace {

using Curve = pricing_primitives::YieldCurve<pricing_primitives::LogLinearDiscountInterpolator>;
std::vector<pricing_primitives::CurveNode> valid_nodes() {
    return {
        {1.0, 0.95},
        {2.0, 0.90},
        {5.0, 0.80},
    };
}
}  // namespace

TEST(YieldCurveTest, DiscountAtZeroIsOne) {
    const Curve curve(valid_nodes());
    EXPECT_DOUBLE_EQ(curve.discount(0.0), 1.0);
}

TEST(YieldCurveTest, ReturnsExactDiscountFactorAtNode) {
    const Curve curve(valid_nodes());
    EXPECT_DOUBLE_EQ(curve.discount(1.0), 0.95);
    EXPECT_DOUBLE_EQ(curve.discount(2.0), 0.90);
    EXPECT_DOUBLE_EQ(curve.discount(5.0), 0.80);
}

TEST(YieldCurveTest, InterpolatesBetweenNodes) {
    const Curve curve({
        {1.0, 0.95},
        {3.0, 0.85},
    });
    EXPECT_NEAR(curve.discount(2.0), std::sqrt(0.95 * 0.85), 1e-12);
}

TEST(YieldCurveTest, RejectsEmptyCurve) {
    EXPECT_THROW(Curve({}), std::invalid_argument);
}

TEST(YieldCurveTest, RejectsNonPositiveMaturity) {
    EXPECT_THROW(Curve({
                     {0.0, 0.95},
                     {1.0, 0.90},
                 }),
                 std::invalid_argument);
    EXPECT_THROW(Curve({
                     {-1.0, 0.95},
                     {1.0, 0.90},
                 }),
                 std::invalid_argument);
}

TEST(YieldCurveTest, RejectsNonPositiveDiscountFactor) {
    EXPECT_THROW(Curve({
                     {1.0, 0.95},
                     {2.0, 0.0},
                 }),
                 std::invalid_argument);
    EXPECT_THROW(Curve({
                     {1.0, 0.95},
                     {2.0, -0.90},
                 }),
                 std::invalid_argument);
}

TEST(YieldCurveTest, RejectsDuplicateMaturity) {
    EXPECT_THROW(Curve({
                     {1.0, 0.95},
                     {1.0, 0.90},
                 }),
                 std::invalid_argument);
}

TEST(YieldCurveTest, RejectsUnsortedNodes) {
    EXPECT_THROW(Curve({
                     {1.0, 0.95},
                     {5.0, 0.80},
                     {2.0, 0.90},
                 }),
                 std::invalid_argument);
}

TEST(YieldCurveTest, RejectsExtrapolationBeforeFirstNode) {
    const Curve curve(valid_nodes());
    EXPECT_THROW(curve.discount(0.5), std::out_of_range);
}

TEST(YieldCurveTest, RejectsExtrapolationAfterLastNode) {
    const Curve curve(valid_nodes());
    EXPECT_THROW(curve.discount(10.0), std::out_of_range);
}

TEST(YieldCurveTest, RejectsNegativeDiscountMaturity) {
    const Curve curve(valid_nodes());
    EXPECT_THROW(curve.discount(-1.0), std::invalid_argument);
}
