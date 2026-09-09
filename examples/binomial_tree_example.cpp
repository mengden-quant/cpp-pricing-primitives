#include <iostream>

#include "pricing_primitives/binomial_tree/binomial_tree.hpp"

int main() {
    const double spot = 100.0;
    const double strike = 100.0;
    const double risk_free_rate = 0.05;
    const double volatility = 0.20;
    const double expiry = 1.0;
    const int num_steps = 100;

    const double call_price =
        pricing_primitives::price_option(spot, strike, risk_free_rate, volatility, expiry,
                                         num_steps, pricing_primitives::OptionType::Call);

    std::cout << "European call price: " << call_price << std::endl;

    return 0;
}
