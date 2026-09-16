# C++ Pricing Primitives

[![CI](https://github.com/mengden-quant/cpp-pricing-primitives/actions/workflows/ci.yml/badge.svg)](https://github.com/mengden-quant/cpp-pricing-primitives/actions/workflows/ci.yml)

Modern C++ implementations of derivatives pricing models, numerical methods, and validation tests used in quantitative finance.

## Purpose

This repository contains compact and production-style implementations of core pricing models frequently used in quantitative finance.

The focus is on:

- derivatives pricing;
- numerical methods;
- model validation;
- convergence analysis;
- arbitrage checks;
- risk sensitivities.

---

## Implemented Models and Components

### Equity Options

#### Binomial Tree

European and American option pricing using a recombining binomial tree.

Features:
- European call and put options;
- American call and put options;
- CRR calibration;
- moment-matching calibration.

Validation:
- input validation;
- arbitrage bounds;
- planned: convergence against Black-Scholes;
- planned: put-call parity tests.

#### Black-Scholes

Analytical pricing of European options under the Black-Scholes model.

Features:
- European call and put options;
- analytical option pricing.

Validation:
- input validation.

### Interest Rates

#### Market Data

Loading and validation of interest-rate market data from JSON files.

Features:
- OIS and IRS market quotes;
- separate discount and projection curve market data;
- maturity sorting;
- duplicate maturity detection;
- input validation.

#### Yield Curve

Discount curve representation with log-linear interpolation of discount factors.

Features:
- discount factor curve nodes;
- log-linear interpolation;
- exact recovery of curve nodes;
- explicit rejection of extrapolation;
- input validation.

#### Discount Curve Bootstrap

Bootstrap of discount factors from par OIS market quotes.

Current assumptions:
- annual payment frequency;
- consecutive annual maturities;
- OIS quotes interpreted as par rates.

Validation:
- repricing of calibration instruments.

#### Interest Rate Swap

Vanilla fixed-floating interest rate swap pricing and risk analytics.

Currently implemented:
- swap domain model;
- annual, semi-annual, and quarterly payment schedules;
- fixed leg valuation.

Planned:
- floating leg valuation;
- swap NPV and par rate;
- DV01/PV01;
- scenario risk analysis.

---

## Repository Structure

```text
cpp-pricing-primitives/
├── app/                 # Qt GUI
├── data/                # Market data scenarios
├── docs/
├── examples/
├── include/
│   └── pricing_primitives/
├── src/
│   ├── binomial_tree/
│   ├── black_scholes/
│   ├── market/
│   └── rates/
├── tests/
└── .github/
```

---

## Dependencies

Required:
- C++20-compatible compiler;
- CMake 3.20 or newer.

Fetched automatically by CMake:
- nlohmann/json 3.12.0 (JSON market data parsing);
- GoogleTest 1.17.0 (unit testing when `BUILD_TESTING=ON`).

Optional:
- Qt 6 with Widgets and Charts components (required only when
`BUILD_GUI=ON`).

---

## Build

### Core library and tests

Qt is not required when the GUI is disabled:
```bash
cmake -S . -B build -DBUILD_GUI=OFF
cmake --build build
```

The default build type is `Release`.

To build in Debug mode:

```bash
cmake -S . -B build -DBUILD_GUI=OFF -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

### GUI

To build the GUI, Qt 6 with the Widgets and Charts components must be
installed and discoverable by CMake:

```bash
cmake -S . -B build -DBUILD_GUI=ON
cmake --build build
```

---

## Run Example

Linux/macOS:
```bash
./build/examples/binomial_tree_example
```

Windows (Visual Studio):
```bash
cmake --build build --config Release
./build/examples/Release/binomial_tree_example.exe
```

---

## Run Tests

Linux/macOS:

```bash
ctest --test-dir build --output-on-failure
```

Windows (Visual Studio):

```bash
ctest --test-dir build -C Release --output-on-failure
```

---

## License

MIT License.
