#include "pricing_primitives/rates/schedule.hpp"

#include <cmath>
#include <stdexcept>

namespace pricing_primitives {
std::vector<double> generate_schedule(double maturity, PaymentFrequency frequency) {
    if (!std::isfinite(maturity) || maturity <= 0.0) {
        throw std::invalid_argument("Maturity must be finite and positive");
    }
    const int payments_per_year = static_cast<int>(frequency);
    if (payments_per_year <= 0) {
        throw std::invalid_argument("Payment frequency must be positive");
    }
    const double periods = maturity * payments_per_year;
    const int number_of_payments = static_cast<int>(std::round(periods));
    constexpr double tolerance = 1e-10;
    if (std::abs(periods - number_of_payments) > tolerance) {
        throw std::invalid_argument("Maturity is incompatible with payment frequency");
    }
    std::vector<double> schedule;
    schedule.reserve(number_of_payments);
    for (int i = 1; i <= number_of_payments; ++i) {
        schedule.push_back(static_cast<double>(i) / payments_per_year);
    }
    return schedule;
}

}  // namespace pricing_primitives
