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

}  // namespace pricing_primitives
