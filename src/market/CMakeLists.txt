add_library(market
    market_data_loader.cpp
)

add_library(pricing_primitives::market ALIAS market)

target_include_directories(market
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(market
    PUBLIC
        cxx_std_20
)

target_link_libraries(market
    PRIVATE
        nlohmann_json::nlohmann_json
        project_warnings
)

enable_clang_tidy(market)
