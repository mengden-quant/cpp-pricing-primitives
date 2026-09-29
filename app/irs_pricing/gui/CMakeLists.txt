qt_add_executable(irs_pricing_gui
    main.cpp
    main_window.cpp
    main_window.hpp
)

target_link_libraries(
    irs_pricing_gui
    PRIVATE
        Qt6::Widgets
        Qt6::Charts
        pricing_primitives::market
        pricing_primitives::rates
        project_warnings
)

enable_clang_tidy(irs_pricing_gui)
