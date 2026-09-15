#pragma once
#include <vector>

#include "pricing_primitives/rates/interest_rate_swap.hpp"

namespace pricing_primitives {

std::vector<double> generate_schedule(double maturity, PaymentFrequency frequency);

}  // namespace pricing_primitives
