#pragma once
#include <vector>

#include "pricing_primitives/market/market_data.hpp"
#include "pricing_primitives/rates/yield_curve.hpp"

namespace pricing_primitives {

class DiscountCurveBootstrapper {
   public:
    static std::vector<CurveNode> bootstrap(const CurveMarketData& market_data);

   private:
    static void validate(const CurveMarketData& market_data);
};
}  // namespace pricing_primitives
