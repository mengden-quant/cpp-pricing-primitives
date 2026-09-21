#pragma once

#include <stdexcept>

#include "pricing_primitives/rates/fixed_leg.hpp"
#include "pricing_primitives/rates/floating_leg.hpp"
#include "pricing_primitives/rates/interest_rate_swap.hpp"

namespace pricing_primitives {

template <typename DiscountCurve, typename ProjectionCurve>
double interest_rate_swap_npv(const InterestRateSwap& swap, const DiscountCurve& discount_curve,
                              const ProjectionCurve& projection_curve) {
    const double fixed_pv = fixed_leg_pv(swap.notional, swap.fixed_rate, swap.maturity,
                                         swap.fixed_frequency, discount_curve);
    const double floating_pv = floating_leg_pv(
        swap.notional, swap.maturity, swap.floating_frequency, discount_curve, projection_curve);
    switch (swap.side) {
        case SwapSide::Payer:
            return floating_pv - fixed_pv;
        case SwapSide::Receiver:
            return fixed_pv - floating_pv;
    }
    throw std::invalid_argument("Unsupported swap side");
}

template <typename DiscountCurve, typename ProjectionCurve>
double par_rate(const InterestRateSwap& swap, const DiscountCurve& discount_curve,
                const ProjectionCurve& projection_curve) {
    const auto schedule = generate_schedule(swap.maturity, swap.fixed_frequency);
    const double accrual = 1.0 / static_cast<int>(swap.fixed_frequency);
    double annuity = 0.0;
    for (const double payment_time : schedule) {
        annuity += accrual * discount_curve.discount(payment_time);
    }
    const double floating_pv = floating_leg_pv(
        swap.notional, swap.maturity, swap.floating_frequency, discount_curve, projection_curve);
    return floating_pv / (swap.notional * annuity);
}

}  // namespace pricing_primitives
