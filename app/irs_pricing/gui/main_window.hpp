#pragma once
#include <QMainWindow>
#include <optional>
#include <vector>

#include "pricing_primitives/market/market_data.hpp"
#include "pricing_primitives/rates/interest_rate_swap.hpp"
#include "pricing_primitives/rates/yield_curve.hpp"

class QChartView;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QPushButton;
class QTableWidget;

class MainWindow : public QMainWindow {
   public:
    explicit MainWindow(QWidget* parent = nullptr);

   private:
    void load_market_data();
    void update_quotes_table();
    void plot_market_rates();

    void bootstrap_curves();
    void plot_bootstrapped_nodes();

    void interpolate_curves();
    void plot_curves(bool interpolate);

    pricing_primitives::InterestRateSwap build_swap() const;

    void calculate_price_and_risk();
    void update_bucketed_dv01_table(const pricing_primitives::InterestRateSwap& swap);

    QPushButton* load_button_;
    QPushButton* bootstrap_button_;
    QPushButton* interpolate_button_;
    QPushButton* price_risk_button_;

    QDoubleSpinBox* notional_input_;
    QDoubleSpinBox* fixed_rate_input_;
    QDoubleSpinBox* maturity_input_;

    QComboBox* fixed_frequency_input_;
    QComboBox* floating_frequency_input_;
    QComboBox* side_input_;

    QTableWidget* quotes_table_;

    QChartView* market_chart_view_;
    QChartView* curve_chart_view_;

    QLabel* npv_value_;
    QLabel* par_rate_value_;
    QLabel* discount_dv01_value_;
    QLabel* projection_dv01_value_;

    QTableWidget* bucketed_dv01_table_;

    std::optional<pricing_primitives::MarketData> market_data_;

    std::vector<pricing_primitives::CurveNode> discount_nodes_;
    std::vector<pricing_primitives::CurveNode> projection_nodes_;
};
