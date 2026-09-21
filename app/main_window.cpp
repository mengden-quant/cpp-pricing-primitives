#include "main_window.hpp"

#include <QChart>
#include <QChartView>
#include <QFileDialog>
#include <QHeaderView>
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

#include "pricing_primitives/market/market_data_loader.hpp"
#include "pricing_primitives/rates/bootstrap.hpp"
#include "pricing_primitives/rates/interpolation.hpp"
#include "pricing_primitives/rates/yield_curve.hpp"

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
}  // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      load_button_(new QPushButton("Load Market Data")),
      bootstrap_button_(new QPushButton("Bootstrap")),
      quotes_table_(new QTableWidget()),
      interpolate_button_(new QPushButton("Interpolate")),
      market_chart_view_(new QChartView()),
      discount_chart_view_(new QChartView()) {
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
    auto* layout = new QVBoxLayout(central_widget);
    quotes_table_->setColumnCount(3);
    quotes_table_->setHorizontalHeaderLabels({"Instrument", "Maturity", "Rate"});
    quotes_table_->horizontalHeader()->setStretchLastSection(true);
    market_chart_view_->setRenderHint(QPainter::Antialiasing);
    discount_chart_view_->setRenderHint(QPainter::Antialiasing);
    bootstrap_button_->setEnabled(false);
    interpolate_button_->setEnabled(false);
    layout->addWidget(load_button_);
    layout->addWidget(bootstrap_button_);
    layout->addWidget(interpolate_button_);
    layout->addWidget(quotes_table_);
    layout->addWidget(market_chart_view_);
    layout->addWidget(discount_chart_view_);
    setCentralWidget(central_widget);
    connect(load_button_, &QPushButton::clicked, this, &MainWindow::load_market_data);
    connect(bootstrap_button_, &QPushButton::clicked, this, &MainWindow::bootstrap_discount_curve);
    connect(interpolate_button_, &QPushButton::clicked, this,
            &MainWindow::interpolate_discount_curve);
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
        interpolate_button_->setEnabled(false);
        discount_chart_view_->setChart(new QChart());
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
    const auto& quotes = market_data_->discount_curve.quotes;
    quotes_table_->clearContents();
    quotes_table_->setRowCount(static_cast<int>(quotes.size()));
    for (std::size_t i = 0; i < quotes.size(); ++i) {
        const auto& quote = quotes[i];
        quotes_table_->setItem(static_cast<int>(i), 0,
                               new QTableWidgetItem(instrument_to_string(quote.instrument)));
        quotes_table_->setItem(static_cast<int>(i), 1,
                               new QTableWidgetItem(QString::number(quote.maturity, 'f', 2)));
        quotes_table_->setItem(static_cast<int>(i), 2,
                               new QTableWidgetItem(QString::number(quote.rate, 'f', 6)));
    }
}

void MainWindow::plot_market_rates() {
    if (!market_data_) {
        return;
    }
    const auto& quotes = market_data_->discount_curve.quotes;
    if (quotes.empty()) {
        return;
    }
    auto* series = new QScatterSeries();
    series->setColor(QColor("#FF9900"));
    series->setBorderColor(QColor("#FF9900"));
    series->setName("Market Quotes");
    series->setMarkerSize(10.0);
    for (const auto& quote : quotes) {
        series->append(quote.maturity, quote.rate);
    }
    auto* chart = new QChart();
    chart->setBackgroundBrush(QBrush(Qt::black));
    chart->setTitleBrush(QBrush(QColor("#FF9900")));
    chart->legend()->setLabelColor(QColor("#E6E6E6"));
    chart->setTitle("Market Rates");
    chart->addSeries(series);
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
    series->attachAxis(maturity_axis);
    series->attachAxis(rate_axis);
    const auto maturity_minmax = std::minmax_element(
        quotes.begin(), quotes.end(),
        [](const auto& lhs, const auto& rhs) { return lhs.maturity < rhs.maturity; });
    const auto rate_minmax =
        std::minmax_element(quotes.begin(), quotes.end(),
                            [](const auto& lhs, const auto& rhs) { return lhs.rate < rhs.rate; });
    const double max_maturity = maturity_minmax.second->maturity;
    const double min_rate = rate_minmax.first->rate;
    const double max_rate = rate_minmax.second->rate;
    maturity_axis->setRange(0.0, max_maturity * 1.1);
    const double rate_padding = std::max((max_rate - min_rate) * 0.2, 0.001);
    rate_axis->setRange(min_rate - rate_padding, max_rate + rate_padding);
    market_chart_view_->setChart(chart);
}

void MainWindow::bootstrap_discount_curve() {
    if (!market_data_) {
        return;
    }
    try {
        discount_nodes_ =
            pricing_primitives::CurveBootstrapper::bootstrap(market_data_->discount_curve);
        plot_discount_curve_nodes();
        interpolate_button_->setEnabled(true);
    } catch (const std::exception& exception) {
        discount_nodes_.clear();
        interpolate_button_->setEnabled(false);
        QMessageBox::critical(this, "Bootstrap failed", exception.what());
    }
}

void MainWindow::plot_discount_curve_nodes() {
    if (discount_nodes_.empty()) {
        return;
    }
    auto* series = new QScatterSeries();
    series->setName("Discount Factors");
    series->setMarkerSize(10.0);
    for (const auto& node : discount_nodes_) {
        series->append(node.maturity, node.discount_factor);
    }
    auto* chart = new QChart();
    chart->setBackgroundBrush(QBrush(Qt::black));
    chart->setTitleBrush(QBrush(QColor("#FF9900")));
    chart->legend()->setLabelColor(QColor("#E6E6E6"));
    chart->setTitle("Discount Curve");
    chart->addSeries(series);
    auto* maturity_axis = new QValueAxis();
    maturity_axis->setLabelsColor(QColor("#E6E6E6"));
    maturity_axis->setTitleBrush(QBrush(QColor("#FF9900")));
    maturity_axis->setGridLineColor(QColor("#333333"));
    maturity_axis->setTitleText("Maturity");
    auto* discount_axis = new QValueAxis();
    discount_axis->setLabelsColor(QColor("#E6E6E6"));
    discount_axis->setTitleBrush(QBrush(QColor("#FF9900")));
    discount_axis->setGridLineColor(QColor("#333333"));
    discount_axis->setTitleText("Discount Factor");
    chart->addAxis(maturity_axis, Qt::AlignBottom);
    chart->addAxis(discount_axis, Qt::AlignLeft);
    series->attachAxis(maturity_axis);
    series->attachAxis(discount_axis);
    series->setColor(QColor("#FF9900"));
    series->setBorderColor(QColor("#FF9900"));
    const auto maturity_minmax = std::minmax_element(
        discount_nodes_.begin(), discount_nodes_.end(),
        [](const auto& lhs, const auto& rhs) { return lhs.maturity < rhs.maturity; });
    const auto discount_minmax = std::minmax_element(
        discount_nodes_.begin(), discount_nodes_.end(),
        [](const auto& lhs, const auto& rhs) { return lhs.discount_factor < rhs.discount_factor; });
    const double max_maturity = maturity_minmax.second->maturity;
    const double min_discount = discount_minmax.first->discount_factor;
    const double max_discount = discount_minmax.second->discount_factor;
    maturity_axis->setRange(0.0, max_maturity * 1.1);
    const double discount_padding = std::max((max_discount - min_discount) * 0.2, 0.01);
    discount_axis->setRange(min_discount - discount_padding, max_discount + discount_padding);
    discount_chart_view_->setChart(chart);
}

void MainWindow::interpolate_discount_curve() {
    if (discount_nodes_.empty()) {
        return;
    }
    try {
        pricing_primitives::YieldCurve<pricing_primitives::LogLinearDiscountInterpolator> curve(
            discount_nodes_);
        auto* series = new QLineSeries();
        series->setName("Interpolated Curve");
        series->setColor(QColor("#FF9900"));
        const double first_maturity = discount_nodes_.front().maturity;
        const double last_maturity = discount_nodes_.back().maturity;
        constexpr double step = 0.05;
        series->append(0.0, curve.discount(0.0));
        for (double maturity = first_maturity; maturity < last_maturity; maturity += step) {
            series->append(maturity, curve.discount(maturity));
        }
        series->append(last_maturity, curve.discount(last_maturity));
        auto* chart = discount_chart_view_->chart();
        chart->addSeries(series);
        const auto axes_x = chart->axes(Qt::Horizontal);
        const auto axes_y = chart->axes(Qt::Vertical);
        if (!axes_x.empty()) {
            series->attachAxis(axes_x.front());
        }
        if (!axes_y.empty()) {
            series->attachAxis(axes_y.front());
        }
        interpolate_button_->setEnabled(false);
    } catch (const std::exception& exception) {
        QMessageBox::critical(this, "Interpolation failed", exception.what());
    }
}
