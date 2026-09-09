#pragma once
#include "pricing_primitives/common/option.hpp"

namespace pricing_primitives {

enum class TreeModel { CRR, MomentMatching };

double binomial_tree_price(double spot, double strike, double risk_free_rate, double volatility,
                           double expiry, int num_steps, OptionType option_type,
                           ExerciseType exercise_type = ExerciseType::European,
                           TreeModel tree_model = TreeModel::CRR);

}  // namespace pricing_primitives
