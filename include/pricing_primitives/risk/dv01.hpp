#pragma once
#include <cmath>
#include <cstddef>
#include <optional>

#include "pricing_primitives/market/market_data.hpp"
#include "pricing_primitives/rates/interest_rate_swap.hpp"
#include "pricing_primitives/rates/irs_pricer.hpp"

namespace pricing_primitives {

namespace detail {

inline CurveMarketData bump_market_data(const CurveMarketData& market_data, double bump_size) {
    auto bumped = market_data;
    for (auto& quote : bumped.quotes) {
        quote.rate += bump_size;
    }
    return bumped;
}

inline CurveMarketData bump_market_data(const CurveMarketData& market_data, std::size_t quote_index,
                                        double bump_size) {
    auto bumped = market_data;
    bumped.quotes.at(quote_index).rate += bump_size;
    return bumped;
}

inline std::optional<std::size_t> find_quote_index(const CurveMarketData& market_data,
                                                   double maturity) {
    constexpr double tolerance = 1e-10;
    for (std::size_t i = 0; i < market_data.quotes.size(); ++i) {
        if (std::abs(market_data.quotes[i].maturity - maturity) < tolerance) {
            return i;
        }
    }
    return std::nullopt;
}

}  // namespace detail

struct Dv01Result {
    double discount_curve;
    double projection_curve;
};

struct BucketedDv01Result {
    std::optional<double> discount_curve;
    std::optional<double> projection_curve;
};

template <typename DiscountCurve, typename ProjectionCurve>
Dv01Result dv01(const InterestRateSwap& swap, const MarketData& market_data,
                const DiscountCurve& discount_curve, const ProjectionCurve& projection_curve,
                double bump_size = 1e-4) {
    using DiscountInterpolator = typename DiscountCurve::interpolator_type;
    using ProjectionInterpolator = typename ProjectionCurve::interpolator_type;

    // ----- Discount curve DV01 -----
    auto discount_up_data = detail::bump_market_data(market_data.discount_curve, bump_size);
    auto discount_down_data = detail::bump_market_data(market_data.discount_curve, -bump_size);

    const auto discount_up_nodes = CurveBootstrapper::bootstrap(discount_up_data);
    const auto discount_down_nodes = CurveBootstrapper::bootstrap(discount_down_data);

    const YieldCurve<DiscountInterpolator> discount_up_curve(discount_up_nodes);
    const YieldCurve<DiscountInterpolator> discount_down_curve(discount_down_nodes);

    const double discount_up_npv =
        interest_rate_swap_npv(swap, discount_up_curve, projection_curve);
    const double discount_down_npv =
        interest_rate_swap_npv(swap, discount_down_curve, projection_curve);

    const double discount_dv01 = (discount_up_npv - discount_down_npv) / (2.0 * bump_size) * 1e-4;

    // ----- Projection curve DV01 -----
    auto projection_up_data = detail::bump_market_data(market_data.projection_curve, bump_size);
    auto projection_down_data = detail::bump_market_data(market_data.projection_curve, -bump_size);

    const auto projection_up_nodes =
        CurveBootstrapper::bootstrap_projection_curve(projection_up_data, discount_curve);
    const auto projection_down_nodes =
        CurveBootstrapper::bootstrap_projection_curve(projection_down_data, discount_curve);

    const YieldCurve<ProjectionInterpolator> projection_up_curve(projection_up_nodes);
    const YieldCurve<ProjectionInterpolator> projection_down_curve(projection_down_nodes);

    const double projection_up_npv =
        interest_rate_swap_npv(swap, discount_curve, projection_up_curve);
    const double projection_down_npv =
        interest_rate_swap_npv(swap, discount_curve, projection_down_curve);

    const double projection_dv01 =
        (projection_up_npv - projection_down_npv) / (2.0 * bump_size) * 1e-4;

    return {discount_dv01, projection_dv01};
}

template <typename DiscountCurve, typename ProjectionCurve>
BucketedDv01Result bucketed_dv01(const InterestRateSwap& swap, const MarketData& market_data,
                                 const DiscountCurve& discount_curve,
                                 const ProjectionCurve& projection_curve, double maturity,
                                 double bump_size) {
    std::optional<double> discount_dv01;
    std::optional<double> projection_dv01;
    using DiscountInterpolator = typename DiscountCurve::interpolator_type;
    using ProjectionInterpolator = typename ProjectionCurve::interpolator_type;

    // ----- Discount curve bucket DV01 -----
    if (const auto index = detail::find_quote_index(market_data.discount_curve, maturity)) {
        const auto discount_up_data =
            detail::bump_market_data(market_data.discount_curve, *index, bump_size);
        const auto discount_down_data =
            detail::bump_market_data(market_data.discount_curve, *index, -bump_size);

        const auto discount_up_nodes = CurveBootstrapper::bootstrap(discount_up_data);
        const auto discount_down_nodes = CurveBootstrapper::bootstrap(discount_down_data);

        const YieldCurve<DiscountInterpolator> discount_up_curve(discount_up_nodes);
        const YieldCurve<DiscountInterpolator> discount_down_curve(discount_down_nodes);

        const double discount_up_npv =
            interest_rate_swap_npv(swap, discount_up_curve, projection_curve);
        const double discount_down_npv =
            interest_rate_swap_npv(swap, discount_down_curve, projection_curve);

        discount_dv01 = (discount_up_npv - discount_down_npv) / (2.0 * bump_size) * 1e-4;
    }

    // ----- Projection curve DV01 -----
    if (const auto index = detail::find_quote_index(market_data.projection_curve, maturity)) {
        const auto projection_up_data =
            detail::bump_market_data(market_data.projection_curve, *index, bump_size);
        const auto projection_down_data =
            detail::bump_market_data(market_data.projection_curve, *index, -bump_size);

        const auto projection_up_nodes =
            CurveBootstrapper::bootstrap_projection_curve(projection_up_data, discount_curve);
        const auto projection_down_nodes =
            CurveBootstrapper::bootstrap_projection_curve(projection_down_data, discount_curve);

        const YieldCurve<ProjectionInterpolator> projection_up_curve(projection_up_nodes);
        const YieldCurve<ProjectionInterpolator> projection_down_curve(projection_down_nodes);

        const double projection_up_npv =
            interest_rate_swap_npv(swap, discount_curve, projection_up_curve);
        const double projection_down_npv =
            interest_rate_swap_npv(swap, discount_curve, projection_down_curve);

        projection_dv01 = (projection_up_npv - projection_down_npv) / (2.0 * bump_size) * 1e-4;
    }

    return {discount_dv01, projection_dv01};
}

}  // namespace pricing_primitives
