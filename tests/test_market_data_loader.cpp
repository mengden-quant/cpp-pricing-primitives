#include <gtest/gtest.h>

#include <filesystem>

#include "pricing_primitives/market/market_data_loader.hpp"

namespace {

std::filesystem::path valid_market_data_path() {
    return std::filesystem::path(TEST_DATA_DIR) / "valid_market_data.json";
}
}  // namespace

TEST(MarketDataLoaderTest, LoadsValidMarketData) {
    const auto market_data =
        pricing_primitives::MarketDataLoader::load_from_json(valid_market_data_path());
    ASSERT_EQ(market_data.discount_curve.quotes.size(), 4);
    ASSERT_EQ(market_data.projection_curve.quotes.size(), 4);
    EXPECT_EQ(market_data.discount_curve.quotes[0].instrument,
              pricing_primitives::InstrumentType::OIS);
    EXPECT_DOUBLE_EQ(market_data.discount_curve.quotes[0].maturity, 0.5);
}

TEST(MarketDataLoaderTest, QuotesAreSortedByMaturity) {
    const auto market_data =
        pricing_primitives::MarketDataLoader::load_from_json(valid_market_data_path());
    const auto& discount_quotes = market_data.discount_curve.quotes;
    ASSERT_EQ(discount_quotes.size(), 4);
    EXPECT_DOUBLE_EQ(discount_quotes[0].maturity, 0.5);
    EXPECT_DOUBLE_EQ(discount_quotes[1].maturity, 1.0);
    EXPECT_DOUBLE_EQ(discount_quotes[2].maturity, 2.0);
    EXPECT_DOUBLE_EQ(discount_quotes[3].maturity, 5.0);
}
