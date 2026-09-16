add_library(rates
    interpolation.cpp
    bootstrap.cpp
    schedule.cpp
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

target_link_libraries(
    rates
    PRIVATE
        project_warnings
)
