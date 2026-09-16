qt_add_executable(pricing_primitives_gui
    main.cpp
    main_window.cpp
    main_window.hpp
)

target_link_libraries(
    pricing_primitives_gui
    PRIVATE
        Qt6::Widgets
        Qt6::Charts
        pricing_primitives::market
        pricing_primitives::rates
        project_warnings
)
