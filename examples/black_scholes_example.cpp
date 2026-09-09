#include <iostream>

#include "pricing_primitives/black_scholes/black_scholes.hpp"

int main() {
    const double spot = 100.0;
    const double strike = 100.0;
    const double risk_free_rate = 0.05;
    const double dividend = 0.0;
    const double volatility = 0.20;
    const double expiry = 1.0;

    const double call_price =
        pricing_primitives::black_scholes_price(spot, strike, risk_free_rate, dividend, volatility,
                                                expiry, pricing_primitives::OptionType::Call);
    std::cout << "European call price: " << call_price << std::endl;

    return 0;
}
