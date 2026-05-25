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

## Implemented Models

### Binomial Tree

European and American option pricing using a recombining binomial tree.

Features:

- European call and put options;
- American call and put options;
- CRR calibration;
- moment-matching calibration.

Validation:

- input validation;
- arbitrage bounds;
- planned: convergence against BSM;
- planned: put-call parity tests.

---

## Repository Structure

```text
cpp-pricing-primitives
├── include/
├── src/
│   └── binomial_tree/
├── examples/
├── tests/
├── docs/
└── .github/
```

---

## Build

```bash
cmake -S . -B build
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
./build/examples/Debug/binomial_tree_example.exe
```

---

## Run Tests

Linux/macOS:

```bash
ctest --test-dir build --output-on-failure
```

Windows (Visual Studio):

```bash
ctest --test-dir build -C Debug --output-on-failure
```

---

## License

MIT License.
