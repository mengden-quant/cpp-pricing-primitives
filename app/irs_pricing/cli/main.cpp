#include <algorithm>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <vector>

#include "pricing_primitives/market/market_data_loader.hpp"
#include "pricing_primitives/rates/bootstrap.hpp"
#include "pricing_primitives/rates/interest_rate_swap.hpp"
#include "pricing_primitives/rates/interpolation.hpp"
#include "pricing_primitives/rates/irs_pricer.hpp"
#include "pricing_primitives/rates/yield_curve.hpp"
#include "pricing_primitives/risk/dv01.hpp"
#include "pricing_primitives/risk/scenario_engine.hpp"

pricing_primitives::PaymentFrequency read_frequency(const char* prompt) {
    int frequency{};
    std::cout << prompt;
    std::cin >> frequency;
    switch (frequency) {
        case 1:
            return pricing_primitives::PaymentFrequency::Annual;
        case 2:
            return pricing_primitives::PaymentFrequency::SemiAnnual;
        case 4:
            return pricing_primitives::PaymentFrequency::Quarterly;
        default:
            throw std::invalid_argument("Payment frequency must be 1. 2, or 4");
    }
}

pricing_primitives::SwapSide read_side() {
    std::string side;
    std::cout << "Side [payer/receiver]: ";
    std::cin >> side;
    if (side == "payer") {
        return pricing_primitives::SwapSide::Payer;
    }
    if (side == "receiver") {
        return pricing_primitives::SwapSide::Receiver;
    }
    throw std::invalid_argument("Swap side must be payer or receiver");
}

pricing_primitives::InterestRateSwap read_swap() {
    double notional{};
    double fixed_rate{};
    double maturity{};

    std::cout << "Notional: ";
    std::cin >> notional;

    std::cout << "Fixed rate: ";
    std::cin >> fixed_rate;

    std::cout << "Maturity: ";
    std::cin >> maturity;

    const auto fixed_frequency = read_frequency("Fixed frequency [1/2/4]: ");

    const auto floating_frequency = read_frequency("Floating frequency [1/2/4]: ");

    const auto side = read_side();

    return {.notional = notional,
            .fixed_rate = fixed_rate,
            .maturity = maturity,
            .fixed_frequency = fixed_frequency,
            .floating_frequency = floating_frequency,
            .side = side};
}

struct NamedScenario {
    std::string name;
    pricing_primitives::RateScenario scenario;
};

std::vector<NamedScenario> load_scenarios(const std::string& path) {
    std::fstream file(path);
    if (!file) {
        throw std::runtime_error("Failed to open scenario file: " + path);
    }

    nlohmann::json json;
    file >> json;

    std::vector<NamedScenario> scenarios;
    for (const auto& item : json.at("scenarios")) {
        scenarios.push_back({
            .name = item.at("name").get<std::string>(),
            .scenario = {.discount_curve_shift = item.at("discount_curve_shift").get<double>(),
                         .projection_curve_shift = item.at("projection_curve_shift").get<double>()},
        });
    }
    return scenarios;
}

int main(int argc, char* argv[]) {
    if (argc < 2 || argc > 3) {
        std::cerr << "Usage: " << argv[0] << " <marget_data.json> [scenarios.json]" << std::endl;
        return 1;
    }
    try {
        const auto market_data = pricing_primitives::MarketDataLoader::load_from_json(argv[1]);
        std::cout << "Market data loaded successfully" << std::endl;
        std::cout << "Discount curve quotes: " << market_data.discount_curve.quotes.size()
                  << std::endl;
        std::cout << "Projection curve quotes: " << market_data.projection_curve.quotes.size()
                  << std::endl;

        const auto swap = read_swap();

        pricing_primitives::CurveBootstrapper bootstrapper;

        const auto discount_nodes = bootstrapper.bootstrap(market_data.discount_curve);
        const auto discount_curve =
            pricing_primitives::YieldCurve<pricing_primitives::LogLinearDiscountInterpolator>(
                discount_nodes);

        const auto projection_nodes =
            bootstrapper.bootstrap_projection_curve(market_data.projection_curve, discount_curve);
        const auto projection_curve =
            pricing_primitives::YieldCurve<pricing_primitives::LogLinearDiscountInterpolator>(
                projection_nodes);

        const double npv =
            pricing_primitives::interest_rate_swap_npv(swap, discount_curve, projection_curve);
        const double market_par_rate =
            pricing_primitives::par_rate(swap, discount_curve, projection_curve);

        constexpr double bump_size = 1e-4;
        const auto dv01 = pricing_primitives::dv01(swap, market_data, discount_curve,
                                                   projection_curve, bump_size);

        std::vector<double> maturities;
        for (const auto& quote : market_data.discount_curve.quotes) {
            if (quote.maturity <= swap.maturity) {
                maturities.push_back(quote.maturity);
            }
        }
        for (const auto& quote : market_data.projection_curve.quotes) {
            if (quote.maturity <= swap.maturity) {
                maturities.push_back(quote.maturity);
            }
        }
        std::sort(maturities.begin(), maturities.end());
        maturities.erase(std::unique(maturities.begin(), maturities.end()), maturities.end());

        std::cout << "IRS created succesfully" << std::endl;

        std::cout << "Pricing & Risk" << std::endl;
        std::cout << "----------------------------------------" << std::endl;
        std::cout << "NPV:             " << npv << std::endl;
        std::cout << "Par rate:        " << market_par_rate << std::endl;
        std::cout << "Discount DV01:   " << dv01.discount_curve << std::endl;
        std::cout << "Projection DV01: " << dv01.projection_curve << std::endl;

        std::cout << "Bucketed DV01" << std::endl;
        std::cout << std::left << std::setw(12) << "Maturity" << std::right << std::setw(18)
                  << "Discount DV01" << std::setw(18) << "Projection DV01" << std::endl;
        for (const double maturity : maturities) {
            const auto result = bucketed_dv01(swap, market_data, discount_curve, projection_curve,
                                              maturity, bump_size);
            std::cout << std::left << std::setw(12) << maturity << std::right;
            if (result.discount_curve.has_value()) {
                std::cout << std::setw(18) << std::fixed << std::setprecision(2)
                          << result.discount_curve.value();
            } else {
                std::cout << std::setw(18) << "-";
            }
            if (result.projection_curve.has_value()) {
                std::cout << std::setw(18) << std::fixed << std::setprecision(2)
                          << result.projection_curve.value();
            } else {
                std::cout << std::setw(18) << "-";
            }
            std::cout << std::endl;
        }

        if (argc == 3) {
            const auto named_scenarios = load_scenarios(argv[2]);
            std::vector<pricing_primitives::RateScenario> scenarios;
            scenarios.reserve(named_scenarios.size());
            for (const auto& named_scenario : named_scenarios) {
                scenarios.push_back(named_scenario.scenario);
            }
            const auto results = pricing_primitives::calculate_scenarios_parallel<
                pricing_primitives::LogLinearDiscountInterpolator,
                pricing_primitives::LogLinearDiscountInterpolator>(swap, market_data, scenarios);

            std::cout << "Scenario Analysis" << std::endl;
            std::cout << "----------------------------------------" << std::endl;
            std::cout << std::left << std::setw(20) << "Scenario" << std::right << std::setw(10)
                      << "NPV" << std::setw(10) << "P&L" << std::endl;
            for (std::size_t i = 0; i < results.size(); ++i) {
                const double scenario_npv = results[i].scenario_npv;
                const double pnl = scenario_npv - npv;

                std::cout << std::left << std::setw(20) << named_scenarios[i].name << std::right
                          << std::setw(10) << std::fixed << std::setprecision(2) << scenario_npv
                          << std::setw(10) << pnl << std::endl;
            }
        }
    } catch (const std::exception& exception) {
        std::cerr << "Error: " << exception.what() << std::endl;
        return 1;
    }
    return 0;
}
