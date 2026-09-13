#pragma once
#include <filesystem>

#include "pricing_primitives/market/market_data.hpp"

namespace pricing_primitives {

class MarketDataLoader {
   public:
    static MarketData load_from_json(const std::filesystem::path& path);
};
}  // namespace pricing_primitives
