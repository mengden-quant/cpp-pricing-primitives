add_executable(
    pricing_primitives_tests
    test_binomial_tree.cpp
    test_black_scholes.cpp
    test_market_data_loader.cpp
    test_interpolation.cpp
    test_yield_curve.cpp
    test_bootstrap.cpp
    test_schedule.cpp
    test_fixed_leg.cpp
    test_floating_leg.cpp
)

target_link_libraries(
    pricing_primitives_tests
    PRIVATE
        pricing_primitives::binomial_tree
        pricing_primitives::black_scholes
        pricing_primitives::market
        pricing_primitives::rates
        GTest::gtest_main
        project_warnings
)

target_compile_definitions(
    pricing_primitives_tests
    PRIVATE
        TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
)

include(GoogleTest)

gtest_discover_tests(pricing_primitives_tests)
