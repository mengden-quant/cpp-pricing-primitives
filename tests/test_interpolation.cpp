#include <gtest/gtest.h>

#include <cmath>

#include "pricing_primitives/rates/interpolation.hpp"
#include "pricing_primitives/rates/yield_curve.hpp"

TEST(LogLinearDiscountInterpolatorTest, InterpolatesBetweenNodes) {
    const pricing_primitives::CurveNode left{1.0, 0.95};
    const pricing_primitives::CurveNode right{3.0, 0.85};
    const pricing_primitives::LogLinearDiscountInterpolator interpolator;
    const double actual = interpolator(2.0, left, right);
    const double expected = std::sqrt(0.95 * 0.85);
    EXPECT_NEAR(actual, expected, 1e-12);
}
