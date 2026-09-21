#pragma once
#include <vector>

#include "pricing_primitives/market/market_data.hpp"
#include "pricing_primitives/rates/yield_curve.hpp"

namespace pricing_primitives {

class CurveBootstrapper {
   public:
    static std::vector<CurveNode> bootstrap(const CurveMarketData& market_data);

    template <typename DiscountCurve>
    static std::vector<CurveNode> bootstrap_projection_curve(const CurveMarketData& market_data,
                                                             const DiscountCurve& discount_curve) {
        validate(market_data, InstrumentType::IRS);
        std::vector<CurveNode> nodes;
        nodes.reserve(market_data.quotes.size());
        for (const auto& quote : market_data.quotes) {
            double fixed_annuity = 0.0;
            double known_floating_pv = 0.0;
            double projection_df_start = 1.0;
            for (const auto& node : nodes) {
                const double payment_time = node.maturity;
                const double projection_df_end = node.discount_factor;
                const double discount_df = discount_curve.discount(payment_time);
                fixed_annuity += discount_df;
                known_floating_pv += (projection_df_start / projection_df_end - 1.0) * discount_df;
                projection_df_start = projection_df_end;
            }
            const double discount_df_end = discount_curve.discount(quote.maturity);
            fixed_annuity += discount_df_end;
            const double required_last_floating_pv = quote.rate * fixed_annuity - known_floating_pv;
            const double last_forward_component = required_last_floating_pv / discount_df_end;
            const double projection_df = projection_df_start / (1.0 + last_forward_component);
            nodes.push_back({quote.maturity, projection_df});
        }
        return nodes;
    }

   private:
    static void validate(const CurveMarketData& market_data, InstrumentType expected_instrument);
};

}  // namespace pricing_primitives
