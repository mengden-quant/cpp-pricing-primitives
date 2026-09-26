#pragma once

#include <algorithm>
#include <cstddef>
#include <thread>
#include <vector>

#include "pricing_primitives/market/market_data.hpp"
#include "pricing_primitives/rates/interest_rate_swap.hpp"
#include "pricing_primitives/risk/scenario.hpp"

namespace pricing_primitives {

template <typename DiscountInterpolator, typename ProjectionInterpolator>
std::vector<ScenarioResult> calculate_scenarios_sequential(
    const InterestRateSwap& swap, const MarketData& market_data,
    const std::vector<RateScenario>& scenarios) {
    std::vector<ScenarioResult> results;
    results.reserve(scenarios.size());

    for (const auto& scenario : scenarios) {
        results.push_back(calculate_scenario<DiscountInterpolator, ProjectionInterpolator>(
            swap, market_data, scenario));
    }

    return results;
}

template <typename DiscountInterpolator, typename ProjectionInterpolator>
std::vector<ScenarioResult> calculate_scenarios_parallel(
    const InterestRateSwap& swap, const MarketData& market_data,
    const std::vector<RateScenario>& scenarios) {
    if (scenarios.empty()) {
        return {};
    }

    std::vector<ScenarioResult> results(scenarios.size());
    const auto hardware_threads = std::thread::hardware_concurrency();
    const std::size_t number_of_threads =
        std::min<std::size_t>(hardware_threads == 0 ? 1 : hardware_threads, scenarios.size());
    std::vector<std::thread> threads;
    threads.reserve(number_of_threads);

    auto process_range = [&](std::size_t begin, std::size_t end) {
        for (std::size_t i = begin; i < end; ++i) {
            results[i] = calculate_scenario<DiscountInterpolator, ProjectionInterpolator>(
                swap, market_data, scenarios[i]);
        }
    };

    const std::size_t chunk_size = (scenarios.size() + number_of_threads - 1) / number_of_threads;
    for (std::size_t thread_index = 0; thread_index < number_of_threads; ++thread_index) {
        const std::size_t begin = thread_index * chunk_size;
        const std::size_t end = std::min(begin + chunk_size, scenarios.size());
        threads.emplace_back(process_range, begin, end);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    return results;
}
}  // namespace pricing_primitives
