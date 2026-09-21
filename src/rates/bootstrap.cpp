#include "pricing_primitives/rates/bootstrap.hpp"

#include <cmath>
#include <stdexcept>

namespace pricing_primitives {

void CurveBootstrapper::validate(const CurveMarketData& market_data,
                                 InstrumentType expected_instrument) {
    if (market_data.quotes.empty()) {
        throw std::invalid_argument("cannot bootstrap curve from empty market data");
    }
    double expected_maturity = 1.0;
    for (const auto& quote : market_data.quotes) {
        if (quote.instrument != expected_instrument) {
            throw std::invalid_argument("curve contains unsupported instrument type");
        }
        if (!std::isfinite(quote.maturity) || quote.maturity <= 0.0) {
            throw std::invalid_argument("quote maturity must be positive and finite");
        }
        if (!std::isfinite(quote.rate)) {
            throw std::invalid_argument("quote rate must be finite");
        }
        constexpr double maturity_tolerance = 1e-10;
        if (std::abs(quote.maturity - expected_maturity) > maturity_tolerance) {
            throw std::invalid_argument(
                "discount curve bootstrap requires consecutive annual maturities");
        }
        expected_maturity += 1.0;
    }
}

std::vector<CurveNode> CurveBootstrapper::bootstrap(const CurveMarketData& market_data) {
    validate(market_data, InstrumentType::OIS);
    std::vector<CurveNode> nodes;
    nodes.reserve(market_data.quotes.size());
    for (const auto& quote : market_data.quotes) {
        double previous_discount_factors = 0.0;
        for (const auto& node : nodes) {
            previous_discount_factors += node.discount_factor;
        }
        const double discount_factor =
            (1.0 - quote.rate * previous_discount_factors) / (1.0 + quote.rate);
        if (!std::isfinite(discount_factor) || discount_factor <= 0.0) {
            throw std::runtime_error("bootstrap produced invalid discount factor");
        }
        nodes.push_back({quote.maturity, discount_factor});
    }
    return nodes;
}
}  // namespace pricing_primitives
