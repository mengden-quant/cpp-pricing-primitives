#pragma once

#include "pricing_primitives/rates/interest_rate_swap.hpp"
#include "pricing_primitives/rates/schedule.hpp"

namespace pricing_primitives {

template <typename Curve>
double fixed_leg_pv(double notional, double fixed_rate, double maturity, PaymentFrequency frequency,
                    const Curve& discount_curve) {
    const auto schedule = generate_schedule(maturity, frequency);
    const double accrual = 1.0 / static_cast<int>(frequency);
    double pv = 0.0;
    for (const double payment_time : schedule) {
        pv += notional * fixed_rate * accrual * discount_curve.discount(payment_time);
    }
    return pv;
}

}  // namespace pricing_primitives
