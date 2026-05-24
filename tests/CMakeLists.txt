add_executable(test_binomial_tree
    test_binomial_tree.cpp
)

target_link_libraries(test_binomial_tree
    PRIVATE
        binomial_tree
)

add_test(
    NAME test_binomial_tree
    COMMAND test_binomial_tree
)
