#pragma once

#include "pricing_primitives/common/option.hpp"

namespace pricing_primitives {

double black_scholes_price(double spot, double strike, double risk_free_rate, double dividend,
                           double volatility, double expiry, OptionType option_type,
                           PayoffType payoff_type = PayoffType::Vanilla);
}  // namespace pricing_primitives
