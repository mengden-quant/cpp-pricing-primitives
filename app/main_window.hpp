#pragma once
#include <QMainWindow>
#include <optional>
#include <vector>

#include "pricing_primitives/market/market_data.hpp"
#include "pricing_primitives/rates/yield_curve.hpp"

class QPushButton;
class QTableWidget;
class QChartView;

class MainWindow : public QMainWindow {
   public:
    explicit MainWindow(QWidget* parent = nullptr);

   private:
    void load_market_data();
    void update_quotes_table();
    void plot_market_rates();

    void bootstrap_discount_curve();
    void plot_discount_curve_nodes();
    void interpolate_discount_curve();

    QPushButton* load_button_;
    QPushButton* bootstrap_button_;
    QTableWidget* quotes_table_;
    QPushButton* interpolate_button_;
    QChartView* market_chart_view_;
    QChartView* discount_chart_view_;

    std::optional<pricing_primitives::MarketData> market_data_;
    std::vector<pricing_primitives::CurveNode> discount_nodes_;
};
