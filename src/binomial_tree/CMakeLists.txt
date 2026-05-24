add_library(binomial_tree
    binomial_tree.cpp
)

target_include_directories(binomial_tree
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(binomial_tree
    PUBLIC
        cxx_std_20
)
