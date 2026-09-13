#include "pricing_primitives/rates/interpolation.hpp"

#include <cmath>

namespace pricing_primitives {

double LogLinearDiscountInterpolator::operator()(double t, const CurveNode& left,
                                                 const CurveNode& right) const {
    const double weight = (t - left.maturity) / (right.maturity - left.maturity);
    return std::exp(std::log(left.discount_factor) +
                    weight * (std::log(right.discount_factor) - std::log(left.discount_factor)));
}
}  // namespace pricing_primitives
