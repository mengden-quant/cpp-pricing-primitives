#pragma once

#include "pricing_primitives/rates/interest_rate_swap.hpp"
#include "pricing_primitives/rates/schedule.hpp"

namespace pricing_primitives {

template <typename DiscountCurve, typename ProjectionCurve>
double floating_leg_pv(double notional, double maturity, PaymentFrequency frequency,
                       const DiscountCurve& discount_curve,
                       const ProjectionCurve& projection_curve) {
    const auto schedule = generate_schedule(maturity, frequency);
    const double accrual = 1.0 / static_cast<int>(frequency);
    double pv = 0.0;
    double previous_time = 0.0;
    for (const double payment_time : schedule) {
        const double projection_df_start = projection_curve.discount(previous_time);
        const double projection_df_end = projection_curve.discount(payment_time);
        const double forward_rate = (projection_df_start / projection_df_end - 1.0) / accrual;
        pv += notional * forward_rate * accrual * discount_curve.discount(payment_time);
        previous_time = payment_time;
    }
    return pv;
}

}  // namespace pricing_primitives
