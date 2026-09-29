#include "main_window.hpp"

#include <QAbstractItemView>
#include <QBrush>
#include <QChart>
#include <QChartView>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLegend>
#include <QLegendMarker>
#include <QLineSeries>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QScatterSeries>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QValueAxis>
#include <QWidget>
#include <algorithm>
#include <exception>
#include <limits>

#include "pricing_primitives/market/market_data_loader.hpp"
#include "pricing_primitives/rates/bootstrap.hpp"
#include "pricing_primitives/rates/interpolation.hpp"
#include "pricing_primitives/rates/irs_pricer.hpp"
#include "pricing_primitives/rates/yield_curve.hpp"
#include "pricing_primitives/risk/dv01.hpp"

namespace {

QString instrument_to_string(pricing_primitives::InstrumentType instrument) {
    switch (instrument) {
        case pricing_primitives::InstrumentType::OIS:
            return "OIS";
        case pricing_primitives::InstrumentType::IRS:
            return "IRS";
    }
    return "Unknown";
}

void set_axis_range(QValueAxis* axis, double min_value, double max_value) {
    constexpr double relative_padding = 0.2;
    constexpr double minimum_padding = 1e-6;
    const double padding = std::max((max_value - min_value) * relative_padding, minimum_padding);
    axis->setRange(min_value - padding, max_value + padding);
}

}  // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      load_button_(new QPushButton("Load Market Data")),
      bootstrap_button_(new QPushButton("Bootstrap")),
      interpolate_button_(new QPushButton("Interpolate")),
      price_risk_button_(new QPushButton("Price && Risk")),
      notional_input_(new QDoubleSpinBox()),
      fixed_rate_input_(new QDoubleSpinBox()),
      maturity_input_(new QDoubleSpinBox()),
      fixed_frequency_input_(new QComboBox()),
      floating_frequency_input_(new QComboBox()),
      side_input_(new QComboBox()),
      quotes_table_(new QTableWidget()),
      market_chart_view_(new QChartView()),
      curve_chart_view_(new QChartView()),
      npv_value_(new QLabel("-")),
      par_rate_value_(new QLabel("-")),
      discount_dv01_value_(new QLabel("-")),
      projection_dv01_value_(new QLabel("-")),
      bucketed_dv01_table_(new QTableWidget()) {
    setWindowTitle("Pricing Primitives");
    resize(1000, 800);
    setStyleSheet(R"(
    QMainWindow {
        background-color: #000000;
    }
    QWidget {
        background-color: #000000;
        color: #E6E6E6;
    }
    QPushButton {
        background-color: #1A1A1A;
        color: #FF9900;
        border: 1px solid #FF9900;
        padding: 6px 12px;
        font-weight: bold;
    }
    QPushButton:hover {
        background-color: #332200;
    }
    QPushButton:disabled {
        color: #666666;
        border-color: #444444;
    }
    QTableWidget {
        background-color: #000000;
        color: #E6E6E6;
        gridline-color: #444444;
        border: 1px solid #444444;
    }
    QHeaderView::section {
        background-color: #1A1A1A;
        color: #FF9900;
        border: 1px solid #444444;
        padding: 4px;
    }
)");
    auto* central_widget = new QWidget(this);
    auto* main_layout = new QVBoxLayout(central_widget);
    quotes_table_->setColumnCount(3);
    quotes_table_->setHorizontalHeaderLabels({"Instrument", "Maturity", "Rate"});
    quotes_table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    quotes_table_->setEditTriggers(QAbstractItemView::NoEditTriggers);

    notional_input_->setRange(1.0, 1'000'000'000.0);
    notional_input_->setValue(1'000'000.0);
    notional_input_->setDecimals(0);

    fixed_rate_input_->setRange(-1.0, 1.0);
    fixed_rate_input_->setValue(0.035);
    fixed_rate_input_->setDecimals(4);
    fixed_rate_input_->setSingleStep(0.001);

    maturity_input_->setRange(1.0, 50.0);
    maturity_input_->setValue(5.0);
    maturity_input_->setDecimals(1);
    maturity_input_->setSingleStep(0.5);

    fixed_frequency_input_->addItem("Annual",
                                    static_cast<int>(pricing_primitives::PaymentFrequency::Annual));
    fixed_frequency_input_->addItem(
        "Semi-Annual", static_cast<int>(pricing_primitives::PaymentFrequency::SemiAnnual));
    fixed_frequency_input_->addItem(
        "Quarterly", static_cast<int>(pricing_primitives::PaymentFrequency::Quarterly));

    floating_frequency_input_->addItem(
        "Annual", static_cast<int>(pricing_primitives::PaymentFrequency::Annual));
    floating_frequency_input_->addItem(
        "Semi-Annual", static_cast<int>(pricing_primitives::PaymentFrequency::SemiAnnual));
    floating_frequency_input_->addItem(
        "Quarterly", static_cast<int>(pricing_primitives::PaymentFrequency::Quarterly));
    side_input_->addItem("Payer", static_cast<int>(pricing_primitives::SwapSide::Payer));
    side_input_->addItem("Receiver", static_cast<int>(pricing_primitives::SwapSide::Receiver));

    bucketed_dv01_table_->setColumnCount(3);
    bucketed_dv01_table_->setHorizontalHeaderLabels(
        {"Maturity", "Discount DV01", "Projection DV01"});
    bucketed_dv01_table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    bucketed_dv01_table_->setEditTriggers(QAbstractItemView::NoEditTriggers);

    auto* swap_group = new QGroupBox("Swap Parameters");
    auto* swap_layout = new QFormLayout(swap_group);

    swap_layout->addRow("Notional:", notional_input_);
    swap_layout->addRow("Contract Fixed Rate:", fixed_rate_input_);
    swap_layout->addRow("Maturity:", maturity_input_);
    swap_layout->addRow("Fixed Frequency:", fixed_frequency_input_);
    swap_layout->addRow("Floating Frequency:", floating_frequency_input_);
    swap_layout->addRow("Side", side_input_);

    swap_layout->addRow("NPV:", npv_value_);
    swap_layout->addRow("Par Rate:", par_rate_value_);
    swap_layout->addRow("Discount DV01:", discount_dv01_value_);
    swap_layout->addRow("Projection DV01:", projection_dv01_value_);

    market_chart_view_->setRenderHint(QPainter::Antialiasing);
    market_chart_view_->hide();
    curve_chart_view_->setRenderHint(QPainter::Antialiasing);
    curve_chart_view_->hide();
    bootstrap_button_->setEnabled(false);
    interpolate_button_->setEnabled(false);
    price_risk_button_->setEnabled(false);
    auto* button_layout = new QHBoxLayout();
    button_layout->addWidget(load_button_);
    button_layout->addWidget(bootstrap_button_);
    button_layout->addWidget(interpolate_button_);
    button_layout->addWidget(price_risk_button_);
    button_layout->addStretch();
    main_layout->addLayout(button_layout);
    quotes_table_->setFixedWidth(500);
    auto* input_layout = new QHBoxLayout();
    input_layout->addWidget(swap_group);
    input_layout->addWidget(quotes_table_);
    input_layout->addStretch();
    main_layout->addLayout(input_layout);
    auto* chart_layout = new QHBoxLayout();
    chart_layout->addWidget(market_chart_view_, 1);
    chart_layout->addWidget(curve_chart_view_, 1);
    main_layout->addLayout(chart_layout, 1);
    main_layout->addWidget(bucketed_dv01_table_);
    setCentralWidget(central_widget);
    connect(load_button_, &QPushButton::clicked, this, &MainWindow::load_market_data);
    connect(bootstrap_button_, &QPushButton::clicked, this, &MainWindow::bootstrap_curves);
    connect(interpolate_button_, &QPushButton::clicked, this, &MainWindow::interpolate_curves);
    connect(price_risk_button_, &QPushButton::clicked, this, &MainWindow::calculate_price_and_risk);
}

void MainWindow::load_market_data() {
    const QString file_path =
        QFileDialog::getOpenFileName(this, "Open Market Data", {}, "JSON Files (*.json)");
    if (file_path.isEmpty()) {
        return;
    }
    try {
        market_data_ =
            pricing_primitives::MarketDataLoader::load_from_json(file_path.toStdString());
        discount_nodes_.clear();
        projection_nodes_.clear();
        interpolate_button_->setEnabled(false);
        price_risk_button_->setEnabled(false);
        curve_chart_view_->setChart(new QChart());
        update_quotes_table();
        plot_market_rates();
        bootstrap_button_->setEnabled(true);
    } catch (const std::exception& exception) {
        QMessageBox::critical(this, "Failed to load market data", exception.what());
    }
}

void MainWindow::update_quotes_table() {
    if (!market_data_) {
        return;
    }
    const auto& discount_quotes = market_data_->discount_curve.quotes;
    const auto& projection_quotes = market_data_->projection_curve.quotes;
    quotes_table_->clearContents();
    quotes_table_->setRowCount(static_cast<int>(discount_quotes.size() + projection_quotes.size()));
    std::size_t row = 0;
    const auto add_quotes = [&](const auto& quotes) {
        for (const auto& quote : quotes) {
            quotes_table_->setItem(static_cast<int>(row), 0,
                                   new QTableWidgetItem(instrument_to_string(quote.instrument)));
            quotes_table_->setItem(static_cast<int>(row), 1,
                                   new QTableWidgetItem(QString::number(quote.maturity, 'f', 2)));
            quotes_table_->setItem(static_cast<int>(row), 2,
                                   new QTableWidgetItem(QString::number(quote.rate, 'f', 6)));
            ++row;
        }
    };
    add_quotes(discount_quotes);
    add_quotes(projection_quotes);
}

pricing_primitives::InterestRateSwap MainWindow::build_swap() const {
    pricing_primitives::InterestRateSwap swap{
        .notional = notional_input_->value(),
        .fixed_rate = fixed_rate_input_->value(),
        .maturity = maturity_input_->value(),
        .fixed_frequency = static_cast<pricing_primitives::PaymentFrequency>(
            fixed_frequency_input_->currentData().toInt()),
        .floating_frequency = static_cast<pricing_primitives::PaymentFrequency>(
            floating_frequency_input_->currentData().toInt()),
        .side = static_cast<pricing_primitives::SwapSide>(side_input_->currentData().toInt())};
    return swap;
}

void MainWindow::calculate_price_and_risk() {
    try {
        constexpr double bump_size = 1e-4;
        using Interpolator = pricing_primitives::LogLinearDiscountInterpolator;
        const pricing_primitives::YieldCurve<Interpolator> discount_curve(discount_nodes_);
        const pricing_primitives::YieldCurve<Interpolator> projection_curve(projection_nodes_);

        const auto swap = build_swap();
        const double npv =
            pricing_primitives::interest_rate_swap_npv(swap, discount_curve, projection_curve);
        const double market_par_rate =
            pricing_primitives::par_rate(swap, discount_curve, projection_curve);
        const auto dv01_result = pricing_primitives::dv01(swap, *market_data_, discount_curve,
                                                          projection_curve, bump_size);

        npv_value_->setText(QString::number(npv, 'f', 2));
        npv_value_->setStyleSheet("font-weight: bold; color: #FF9900;");
        par_rate_value_->setText(QString::number(market_par_rate, 'f', 4));
        par_rate_value_->setStyleSheet("font-weight: bold; color: #FF9900;");
        discount_dv01_value_->setText(QString::number(dv01_result.discount_curve, 'f', 2));
        discount_dv01_value_->setStyleSheet("font-weight: bold; color: #FF9900;");
        projection_dv01_value_->setText(QString::number(dv01_result.projection_curve, 'f', 2));
        projection_dv01_value_->setStyleSheet("font-weight: bold; color: #FF9900;");

        update_bucketed_dv01_table(swap);

    } catch (const std::exception& exception) {
        QMessageBox::critical(this, "Pricing failed", exception.what());
    }
}

void MainWindow::update_bucketed_dv01_table(const pricing_primitives::InterestRateSwap& swap) {
    std::vector<double> maturities;
    for (const auto& quote : market_data_->discount_curve.quotes) {
        maturities.push_back(quote.maturity);
    }
    for (const auto& quote : market_data_->projection_curve.quotes) {
        maturities.push_back(quote.maturity);
    }
    std::sort(maturities.begin(), maturities.end());
    maturities.erase(std::unique(maturities.begin(), maturities.end()), maturities.end());
    bucketed_dv01_table_->setRowCount(static_cast<int>(maturities.size()));
    using Interpolator = pricing_primitives::LogLinearDiscountInterpolator;
    const pricing_primitives::YieldCurve<Interpolator> discount_curve(discount_nodes_);
    const pricing_primitives::YieldCurve<Interpolator> projection_curve(projection_nodes_);

    constexpr double bump_size = 1e-4;
    for (std::size_t i = 0; i < maturities.size(); ++i) {
        const double maturity = maturities[i];
        const auto result = pricing_primitives::bucketed_dv01(
            swap, *market_data_, discount_curve, projection_curve, maturity, bump_size);
        const int row = static_cast<int>(i);
        bucketed_dv01_table_->setItem(row, 0,
                                      new QTableWidgetItem(QString::number(maturity, 'f', 1)));
        bucketed_dv01_table_->setItem(
            row, 1,
            new QTableWidgetItem(
                result.discount_curve ? QString::number(*result.discount_curve, 'f', 2) : "-"));
        bucketed_dv01_table_->setItem(
            row, 2,
            new QTableWidgetItem(
                result.projection_curve ? QString::number(*result.projection_curve, 'f', 2) : "-"));
    }
}

void MainWindow::plot_market_rates() {
    constexpr auto discount_color = "#FF9900";
    constexpr auto projection_color = "#6E6259";

    const auto& discount_quotes = market_data_->discount_curve.quotes;
    const auto& projection_quotes = market_data_->projection_curve.quotes;

    auto* discount_series = new QScatterSeries();
    discount_series->setName("OIS Quotes");
    discount_series->setMarkerSize(10.0);
    discount_series->setColor(QColor(discount_color));
    discount_series->setBorderColor(QColor(discount_color));

    auto* projection_series = new QScatterSeries();
    projection_series->setName("IRS Quotes");
    projection_series->setMarkerSize(10.0);
    projection_series->setColor(QColor(projection_color));
    projection_series->setBorderColor(QColor(projection_color));

    double max_maturity = 0.0;
    double min_rate = std::numeric_limits<double>::max();
    double max_rate = std::numeric_limits<double>::lowest();

    for (const auto& quote : discount_quotes) {
        discount_series->append(quote.maturity, quote.rate);
        max_maturity = std::max(max_maturity, quote.maturity);
        min_rate = std::min(min_rate, quote.rate);
        max_rate = std::max(max_rate, quote.rate);
    }

    for (const auto& quote : projection_quotes) {
        projection_series->append(quote.maturity, quote.rate);
        max_maturity = std::max(max_maturity, quote.maturity);
        min_rate = std::min(min_rate, quote.rate);
        max_rate = std::max(max_rate, quote.rate);
    }

    auto* chart = new QChart();
    chart->setBackgroundBrush(QBrush(Qt::black));
    chart->setTitleBrush(QBrush(QColor("#FF9900")));
    chart->legend()->setLabelColor(QColor("#E6E6E6"));
    chart->setTitle("Market Rates");

    chart->addSeries(discount_series);
    chart->addSeries(projection_series);

    auto* maturity_axis = new QValueAxis();
    maturity_axis->setLabelsColor(QColor("#E6E6E6"));
    maturity_axis->setTitleBrush(QBrush(QColor("#FF9900")));
    maturity_axis->setGridLineColor(QColor("#333333"));
    maturity_axis->setTitleText("Maturity");

    auto* rate_axis = new QValueAxis();
    rate_axis->setLabelsColor(QColor("#E6E6E6"));
    rate_axis->setTitleBrush(QBrush(QColor("#FF9900")));
    rate_axis->setGridLineColor(QColor("#333333"));
    rate_axis->setTitleText("Rate");

    chart->addAxis(maturity_axis, Qt::AlignBottom);
    chart->addAxis(rate_axis, Qt::AlignLeft);

    discount_series->attachAxis(maturity_axis);
    discount_series->attachAxis(rate_axis);

    projection_series->attachAxis(maturity_axis);
    projection_series->attachAxis(rate_axis);

    maturity_axis->setRange(0.0, max_maturity * 1.1);
    set_axis_range(rate_axis, min_rate, max_rate);

    market_chart_view_->setChart(chart);
    market_chart_view_->show();
}

void MainWindow::bootstrap_curves() {
    if (!market_data_) {
        return;
    }
    try {
        discount_nodes_ =
            pricing_primitives::CurveBootstrapper::bootstrap(market_data_->discount_curve);
        const pricing_primitives::YieldCurve<pricing_primitives::LogLinearDiscountInterpolator>
            discount_curve(discount_nodes_);
        projection_nodes_ = pricing_primitives::CurveBootstrapper::bootstrap_projection_curve(
            market_data_->projection_curve, discount_curve);
        plot_bootstrapped_nodes();
        interpolate_button_->setEnabled(true);
        price_risk_button_->setEnabled(true);
    } catch (const std::exception& exception) {
        discount_nodes_.clear();
        projection_nodes_.clear();
        interpolate_button_->setEnabled(false);
        QMessageBox::critical(this, "Bootstrap failed", exception.what());
    }
}

void MainWindow::plot_bootstrapped_nodes() {
    plot_curves(false);
}

void MainWindow::interpolate_curves() {
    try {
        plot_curves(true);
        interpolate_button_->setEnabled(false);
    } catch (const std::exception& exception) {
        QMessageBox::critical(this, "Interpolation failed", exception.what());
    }
}

void MainWindow::plot_curves(bool interpolate) {
    constexpr auto discount_color = "#FF9900";
    constexpr auto projection_color = "#6E6259";

    auto* discount_node_series = new QScatterSeries();
    discount_node_series->setName("Discount Curve");
    discount_node_series->setMarkerSize(10.0);
    discount_node_series->setColor(QColor(discount_color));
    discount_node_series->setBorderColor(QColor(discount_color));

    auto* projection_node_series = new QScatterSeries();
    projection_node_series->setName("Projection Curve");
    projection_node_series->setMarkerSize(10.0);
    projection_node_series->setColor(QColor(projection_color));
    projection_node_series->setBorderColor(QColor(projection_color));

    discount_node_series->append(0.0, 1.0);
    projection_node_series->append(0.0, 1.0);

    double max_maturity = 0.0;
    double min_factor = std::numeric_limits<double>::max();
    double max_factor = std::numeric_limits<double>::lowest();

    const auto add_nodes = [&](const auto& nodes, QScatterSeries* series) {
        for (const auto& node : nodes) {
            series->append(node.maturity, node.discount_factor);
            max_maturity = std::max(max_maturity, node.maturity);
            min_factor = std::min(min_factor, node.discount_factor);
            max_factor = std::max(max_factor, node.discount_factor);
        }
    };

    add_nodes(discount_nodes_, discount_node_series);
    add_nodes(projection_nodes_, projection_node_series);
    max_factor = std::max(max_factor, 1.0);

    auto* chart = new QChart();
    chart->setBackgroundBrush(QBrush(Qt::black));
    chart->setTitleBrush(QBrush(QColor("#FF9900")));
    chart->legend()->setLabelColor(QColor("#E6E6E6"));
    chart->setTitle("Bootstrapped Curves");

    chart->addSeries(discount_node_series);
    chart->addSeries(projection_node_series);

    QLineSeries* discount_interpolation_series = nullptr;
    QLineSeries* projection_interpolation_series = nullptr;

    if (interpolate) {
        using Interpolator = pricing_primitives::LogLinearDiscountInterpolator;
        const pricing_primitives::YieldCurve<Interpolator> discount_curve(discount_nodes_);
        const pricing_primitives::YieldCurve<Interpolator> projection_curve(projection_nodes_);

        discount_interpolation_series = new QLineSeries;
        discount_interpolation_series->setName("Discount Interpolation");
        discount_interpolation_series->setColor(QColor(discount_color));

        projection_interpolation_series = new QLineSeries;
        projection_interpolation_series->setName("Projection Interpolation");
        projection_interpolation_series->setColor(QColor(projection_color));

        const double step = 0.05;
        discount_interpolation_series->append(0.0, discount_curve.discount(0.0));
        projection_interpolation_series->append(0.0, projection_curve.discount(0.0));
        for (double maturity = discount_nodes_.front().maturity;
             maturity < discount_nodes_.back().maturity; maturity += step) {
            discount_interpolation_series->append(maturity, discount_curve.discount(maturity));
        }
        for (double maturity = projection_nodes_.front().maturity;
             maturity < projection_nodes_.back().maturity; maturity += step) {
            projection_interpolation_series->append(maturity, projection_curve.discount(maturity));
        }
        discount_interpolation_series->append(
            discount_nodes_.back().maturity,
            discount_curve.discount(discount_nodes_.back().maturity));
        projection_interpolation_series->append(
            projection_nodes_.back().maturity,
            projection_curve.discount(projection_nodes_.back().maturity));
        chart->addSeries(discount_interpolation_series);
        chart->addSeries(projection_interpolation_series);
        chart->legend()->markers(discount_interpolation_series).front()->setVisible(false);
        chart->legend()->markers(projection_interpolation_series).front()->setVisible(false);
    }

    auto* maturity_axis = new QValueAxis();
    maturity_axis->setLabelsColor(QColor("#E6E6E6"));
    maturity_axis->setTitleBrush(QBrush(QColor("#FF9900")));
    maturity_axis->setGridLineColor(QColor("#333333"));
    maturity_axis->setTitleText("Maturity");

    auto* factor_axis = new QValueAxis();
    factor_axis->setLabelsColor(QColor("#E6E6E6"));
    factor_axis->setTitleBrush(QBrush(QColor("#FF9900")));
    factor_axis->setGridLineColor(QColor("#333333"));
    factor_axis->setTitleText("Factor");

    chart->addAxis(maturity_axis, Qt::AlignBottom);
    chart->addAxis(factor_axis, Qt::AlignLeft);
    discount_node_series->attachAxis(maturity_axis);
    discount_node_series->attachAxis(factor_axis);
    projection_node_series->attachAxis(maturity_axis);
    projection_node_series->attachAxis(factor_axis);

    if (interpolate) {
        discount_interpolation_series->attachAxis(maturity_axis);
        discount_interpolation_series->attachAxis(factor_axis);
        projection_interpolation_series->attachAxis(maturity_axis);
        projection_interpolation_series->attachAxis(factor_axis);
    }

    maturity_axis->setRange(0.0, max_maturity * 1.1);
    set_axis_range(factor_axis, min_factor, max_factor);

    curve_chart_view_->setChart(chart);
    curve_chart_view_->show();
}
