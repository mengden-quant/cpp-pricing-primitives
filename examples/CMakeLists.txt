add_executable(binomial_tree_example
    binomial_tree_example.cpp
)

target_link_libraries(binomial_tree_example
    PRIVATE
        pricing_primitives::binomial_tree
)

add_executable(black_scholes_example
    black_scholes_example.cpp

)

target_link_libraries(black_scholes_example
    PRIVATE
        pricing_primitives::black_scholes
)
