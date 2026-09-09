add_executable(
    pricing_primitives_tests
    test_binomial_tree.cpp
)

target_link_libraries(
    pricing_primitives_tests
    PRIVATE
        pricing_primitives::binomial_tree
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(pricing_primitives_tests)
