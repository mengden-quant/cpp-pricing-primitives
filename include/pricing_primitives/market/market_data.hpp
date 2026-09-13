#pragma once
#include <vector>

namespace pricing_primitives {

enum class InstrumentType { OIS, IRS };

struct MarketQuote {
    InstrumentType instrument;
    double maturity;
    double rate;
};

struct CurveMarketData {
    std::vector<MarketQuote> quotes;
};

struct MarketData {
    CurveMarketData discount_curve;
    CurveMarketData projection_curve;
};
}  // namespace pricing_primitives
