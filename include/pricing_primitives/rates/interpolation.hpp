#pragma once
#include "pricing_primitives/rates/yield_curve.hpp"

namespace pricing_primitives {

class LogLinearDiscountInterpolator {
   public:
    double operator()(double t, const CurveNode& left, const CurveNode& right) const;
};

}  // namespace pricing_primitives
