#pragma once

namespace pricing_primitives {

enum class SwapSide { Payer, Receiver };
enum class PaymentFrequency { Annual = 1, SemiAnnual = 2, Quarterly = 4 };
struct InterestRateSwap {
    double notional;
    double fixed_rate;
    double maturity;
    PaymentFrequency fixed_frequency;
    PaymentFrequency floating_frequency;
    SwapSide side;
};

}  // namespace pricing_primitives
