#include "pricing_primitives/market/market_data_loader.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>

namespace pricing_primitives {
namespace {

InstrumentType parse_instrument_type(const std::string& value) {
    if (value == "OIS") {
        return InstrumentType::OIS;
    }
    if (value == "IRS") {
        return InstrumentType::IRS;
    }
    throw std::invalid_argument("unsupported instrument type: " + value);
}

MarketQuote parse_market_quote(const nlohmann::json& json) {
    const auto instrument = parse_instrument_type(json.at("instrument").get<std::string>());
    const double maturity = json.at("maturity").get<double>();
    const double rate = json.at("rate").get<double>();
    if (maturity <= 0.0) {
        throw std::invalid_argument("market quote maturity must be positive");
    }
    if (!std::isfinite(rate)) {
        throw std::invalid_argument("market quote rate must be finite");
    }
    return {
        instrument,
        maturity,
        rate,
    };
}

CurveMarketData parse_curve_market_data(const nlohmann::json& json) {
    CurveMarketData result;
    for (const auto& quote_json : json.at("quotes")) {
        result.quotes.push_back(parse_market_quote(quote_json));
    }

    if (result.quotes.empty()) {
        throw std::invalid_argument("curve must contain at least one market quote");
    }
    std::sort(
        result.quotes.begin(), result.quotes.end(),
        [](const MarketQuote& lhs, const MarketQuote& rhs) { return lhs.maturity < rhs.maturity; });
    for (std::size_t i = 1; i < result.quotes.size(); ++i) {
        if (result.quotes[i - 1].maturity == result.quotes[i].maturity) {
            throw std::invalid_argument("duplicate market quote maturity");
        }
    }
    return result;
}
}  // namespace

MarketData MarketDataLoader::load_from_json(const std::filesystem::path& path) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("failed to open market data file: " + path.string());
    }
    nlohmann::json json;
    input >> json;
    return {
        parse_curve_market_data(json.at("discount_curve")),
        parse_curve_market_data(json.at("projection_curve")),
    };
}
}  // namespace pricing_primitives
