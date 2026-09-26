#pragma once

#include "pricing_primitives/market/market_data.hpp"
#include "pricing_primitives/rates/bootstrap.hpp"
#include "pricing_primitives/rates/interest_rate_swap.hpp"
#include "pricing_primitives/rates/irs_pricer.hpp"
#include "pricing_primitives/rates/yield_curve.hpp"

namespace pricing_primitives {

struct RateScenario {
    double discount_curve_shift;
    double projection_curve_shift;
};
struct ScenarioResult {
    double scenario_npv;
};

template <typename DiscountInterpolator, typename ProjectionInterpolator>
ScenarioResult calculate_scenario(const InterestRateSwap& swap, const MarketData& market_data,
                                  const RateScenario& scenario) {
    auto shocked_discount_data = market_data.discount_curve;
    for (auto& quote : shocked_discount_data.quotes) {
        quote.rate += scenario.discount_curve_shift;
    }
    const auto shocked_discount_nodes =
        pricing_primitives::CurveBootstrapper::bootstrap(shocked_discount_data);
    const pricing_primitives::YieldCurve<DiscountInterpolator> shocked_discount_curve(
        shocked_discount_nodes);

    auto shocked_projection_data = market_data.projection_curve;
    for (auto& quote : shocked_projection_data.quotes) {
        quote.rate += scenario.projection_curve_shift;
    }
    const auto shocked_projection_nodes =
        pricing_primitives::CurveBootstrapper::bootstrap_projection_curve(shocked_projection_data,
                                                                          shocked_discount_curve);
    const pricing_primitives::YieldCurve<ProjectionInterpolator> shocked_projection_curve(
        shocked_projection_nodes);

    const double scenario_npv = pricing_primitives::interest_rate_swap_npv(
        swap, shocked_discount_curve, shocked_projection_curve);

    return {scenario_npv};
}

}  // namespace pricing_primitives
