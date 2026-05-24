add_executable(binomial_tree_example
    binomial_tree_example.cpp
)

target_link_libraries(binomial_tree_example
    PRIVATE
        binomial_tree
)
