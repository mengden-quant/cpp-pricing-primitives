#pragma once

namespace pricing_primitives {

enum class OptionType {
    Call,
    Put
};

enum class ExerciseType {
    European,
    American,
};

enum class TreeModel {
    CRR,
    MomentMatching
};

double price_option(
    double spot,
    double strike,
    double risk_free_rate,
    double volatility,
    double expiry,
    int num_steps,
    OptionType option_type,
    ExerciseType exercise_type = ExerciseType::European,
    TreeModel tree_model = TreeModel::CRR
);

} // namespace pricing_primitives
