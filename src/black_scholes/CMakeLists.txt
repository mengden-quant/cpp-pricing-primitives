add_library(black_scholes
    black_scholes.cpp
)

add_library(pricing_primitives::black_scholes ALIAS black_scholes)

target_include_directories(black_scholes
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(black_scholes
    PUBLIC
        cxx_std_20
)
