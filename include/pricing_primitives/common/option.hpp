#pragma once

namespace pricing_primitives {

enum class OptionType { Call, Put };

enum class ExerciseType { European, American };

enum class PayoffType { Vanilla, Digital };

struct EquityOption {
    double strike;
    double expiry;
    OptionType option_type;
    ExerciseType exercise_type;
    PayoffType payoff_type;
};

}  // namespace pricing_primitives
