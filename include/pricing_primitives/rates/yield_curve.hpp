#pragma once
#include <algorithm>
#include <cmath>
#include <iterator>
#include <stdexcept>
#include <utility>
#include <vector>

namespace pricing_primitives {

struct CurveNode {
    double maturity;
    double discount_factor;
};

template <typename Interpolator>
class YieldCurve {
   public:
    using interpolator_type = Interpolator;
    explicit YieldCurve(std::vector<CurveNode> nodes) : nodes_(std::move(nodes)) {
        validate();
    }

    double discount(double maturity) const {
        if (!std::isfinite(maturity) || maturity < 0.0) {
            throw std::invalid_argument("discount maturity must be non-negative and finite");
        }
        if (maturity == 0.0) {
            return 1.0;
        }
        if (maturity < nodes_.front().maturity || maturity > nodes_.back().maturity) {
            throw std::out_of_range("discount maturity is outside the curve range");
        }
        const auto right = std::lower_bound(
            nodes_.begin(), nodes_.end(), maturity,
            [](const CurveNode& node, double value) { return node.maturity < value; });
        static constexpr double maturity_tolerance = 1e-12;
        if (std::abs(right->maturity - maturity) < maturity_tolerance) {
            return right->discount_factor;
        }
        const auto left = std::prev(right);
        return interpolator_(maturity, *left, *right);
    }

   private:
    void validate() const {
        if (nodes_.empty()) {
            throw std::invalid_argument("yield curve must contain at least one node");
        }
        for (const auto& node : nodes_) {
            if (!std::isfinite(node.maturity) || node.maturity <= 0.0) {
                throw std::invalid_argument("curve node maturity must be positive and finite");
            }
            if (!std::isfinite(node.discount_factor) || node.discount_factor <= 0.0) {
                throw std::invalid_argument(
                    "curve node discount factor must be positive and finite");
            }
        }
        for (std::size_t i = 1; i < nodes_.size(); ++i) {
            if (nodes_[i].maturity <= nodes_[i - 1].maturity) {
                throw std::invalid_argument("curve nodes must be strictly increasing by maturity");
            }
        }
    }
    std::vector<CurveNode> nodes_;
    Interpolator interpolator_{};
};
}  // namespace pricing_primitives
