#include "pricing_primitives/binomial_tree/binomial_tree.hpp"

#include <cassert>
#include <cmath>

int main() {
    const double price = pricing_primitives::price_option(
        100.0,
        100.0,
        0.05,
        0.20,
        1.0,
        100,
        pricing_primitives::OptionType::Call
    );

    assert(std::isfinite(price));
    assert(price > 0.0);
    assert(price < 100.0);

    return 0;
}
