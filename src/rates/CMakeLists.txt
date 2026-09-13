add_library(rates
    interpolation.cpp
)

add_library(pricing_primitives::rates ALIAS rates)

target_include_directories(rates
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(rates
    PUBLIC
        cxx_std_20
)
