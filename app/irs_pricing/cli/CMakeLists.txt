add_executable(irs_pricing_cli
    main.cpp
)

target_link_libraries(
    irs_pricing_cli
    PRIVATE
        pricing_primitives::market
        pricing_primitives::rates
        nlohmann_json::nlohmann_json
        project_warnings
)

enable_clang_tidy(irs_pricing_cli)
