#include <gtest/gtest.h>

#include <cmath>
#include <limits>
#include <stdexcept>
#include <vector>

#include "pricing_primitives/rates/schedule.hpp"

TEST(ScheduleTest, GeneratesAnnualSchedule) {
    const auto schedule =
        pricing_primitives::generate_schedule(5.0, pricing_primitives::PaymentFrequency::Annual);
    const std::vector<double> expected{1.0, 2.0, 3.0, 4.0, 5.0};
    EXPECT_EQ(schedule, expected);
}

TEST(ScheduleTest, GeneratesSemiAnnualSchedule) {
    const auto schedule = pricing_primitives::generate_schedule(
        2.0, pricing_primitives::PaymentFrequency::SemiAnnual);
    const std::vector<double> expected{0.5, 1.0, 1.5, 2.0};
    EXPECT_EQ(schedule, expected);
}

TEST(ScheduleTest, GeneratesQuarterlySchedule) {
    const auto schedule =
        pricing_primitives::generate_schedule(1.0, pricing_primitives::PaymentFrequency::Quarterly);
    const std::vector<double> expected{0.25, 0.5, 0.75, 1.0};
    EXPECT_EQ(schedule, expected);
}

TEST(ScheduleTest, RejectsZeroMaturity) {
    EXPECT_THROW(
        pricing_primitives::generate_schedule(0.0, pricing_primitives::PaymentFrequency::Annual),
        std::invalid_argument);
}

TEST(ScheduleTest, RejectsNegativeMaturity) {
    EXPECT_THROW(
        pricing_primitives::generate_schedule(-1.0, pricing_primitives::PaymentFrequency::Annual),
        std::invalid_argument);
}

TEST(ScheduleTest, RejectsNonFiniteMaturity) {
    EXPECT_THROW(
        pricing_primitives::generate_schedule(std::numeric_limits<double>::infinity(),
                                              pricing_primitives::PaymentFrequency::Annual),
        std::invalid_argument);
    EXPECT_THROW(
        pricing_primitives::generate_schedule(std::numeric_limits<double>::quiet_NaN(),
                                              pricing_primitives::PaymentFrequency::Annual),
        std::invalid_argument);
}

TEST(ScheduleTest, RejectsIncompatibleMaturityAndFrequency) {
    EXPECT_THROW(pricing_primitives::generate_schedule(
                     1.25, pricing_primitives::PaymentFrequency::SemiAnnual),
                 std::invalid_argument);
}

TEST(ScheduleTest, RejectsInvalidPaymentFrequency) {
    const auto invalid_frequency = static_cast<pricing_primitives::PaymentFrequency>(0);
    EXPECT_THROW(pricing_primitives::generate_schedule(1.0, invalid_frequency),
                 std::invalid_argument);
}
