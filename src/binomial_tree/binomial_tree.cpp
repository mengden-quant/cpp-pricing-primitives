#include "pricing_primitives/binomial_tree/binomial_tree.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

namespace pricing_primitives {
namespace {

struct TreeParameters {
    double up;
    double down;
    double probability;
    double discount_factor;
};

void validate_inputs(double spot, double strike, double volatility, double expiry, int num_steps) {
    if (spot <= 0.0) {
        throw std::invalid_argument("'asset' must be positive!");
    }
    if (strike <= 0.0) {
        throw std::invalid_argument("'strike' must be positive!");
    }
    if (volatility <= 0.0) {
        throw std::invalid_argument("'volatility must be positive!'");
    }
    if (expiry <= 0.0) {
        throw std::invalid_argument("'expiry' must be positive!");
    }
    if (num_steps <= 0) {
        throw std::invalid_argument("'num_steps' must be positive!");
    }
}

double payoff(double spot, double strike, OptionType option_type) {
    switch (option_type) {
        case OptionType::Call:
            return std::max(spot - strike, 0.0);

        case OptionType::Put:
            return std::max(strike - spot, 0.0);
    }

    throw std::invalid_argument("unknown option type!");
}

TreeParameters make_crr_parameters(double risk_free_rate, double volatility, double dt) {
    const double up = std::exp(volatility * std::sqrt(dt));
    const double down = 1.0 / up;
    const double growth = std::exp(risk_free_rate * dt);
    const double probability = (growth - down) / (up - down);
    const double discount_factor = std::exp(-risk_free_rate * dt);

    if (probability < 0.0 || probability > 1.0) {
        throw std::invalid_argument("CRR probability is outside [0, 1]!");
    }
    return {up, down, probability, discount_factor};
}

TreeParameters make_moment_matching_parameters(double risk_free_rate, double volatility,
                                               double dt) {
    const double a =
        std::exp(-risk_free_rate * dt) + std::exp((risk_free_rate + volatility * volatility) * dt);

    const double discriminant = a * a - 4.0;

    if (discriminant < 0.0) {
        throw std::invalid_argument("moment matching discriminant is negative!");
    }

    const double up = 0.5 * (a + std::sqrt(discriminant));
    const double down = 1.0 / up;
    const double growth = std::exp(risk_free_rate * dt);
    const double probability = (growth - down) / (up - down);
    const double discount_factor = std::exp(-risk_free_rate * dt);

    if (probability < 0.0 || probability > 1.0) {
        throw std::invalid_argument("moment matching probability is outside [0, 1]!");
    }

    return {up, down, probability, discount_factor};
}

TreeParameters make_tree_parameters(double risk_free_rate, double volatility, double dt,
                                    TreeModel tree_model) {
    switch (tree_model) {
        case TreeModel::CRR:
            return make_crr_parameters(risk_free_rate, volatility, dt);

        case TreeModel::MomentMatching:
            return make_moment_matching_parameters(risk_free_rate, volatility, dt);
    }

    throw std::invalid_argument("unknown tree model!");
}

}  // namespace

double binomial_tree_price(double spot, double strike, double risk_free_rate, double volatility,
                           double expiry, int num_steps, OptionType option_type,
                           ExerciseType exercise_type, TreeModel tree_model) {
    validate_inputs(spot, strike, volatility, expiry, num_steps);

    const double dt = expiry / static_cast<double>(num_steps);

    const TreeParameters params = make_tree_parameters(risk_free_rate, volatility, dt, tree_model);

    std::vector<double> option_values(num_steps + 1);

    for (int j = 0; j <= num_steps; ++j) {
        const double terminal_spot =
            spot * std::pow(params.up, j) * std::pow(params.down, num_steps - j);

        option_values[j] = payoff(terminal_spot, strike, option_type);
    }

    for (int step = num_steps; step >= 1; --step) {
        for (int j = 0; j < step; ++j) {
            const double continuation_value =
                params.discount_factor * (params.probability * option_values[j + 1] +
                                          (1.0 - params.probability) * option_values[j]);

            if (exercise_type == ExerciseType::American) {
                const double current_spot =
                    spot * std::pow(params.up, j) * std::pow(params.down, (step - 1) - j);

                const double exercise_value = payoff(current_spot, strike, option_type);

                option_values[j] = std::max(continuation_value, exercise_value);
            } else {
                option_values[j] = continuation_value;
            }
        }
    }

    return option_values[0];
}

}  // namespace pricing_primitives
