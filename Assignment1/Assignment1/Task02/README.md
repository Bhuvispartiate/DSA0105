# Task 02 – Tokens, Keywords, Identifiers and Constants

**Marks:** 3

## Objective
Use tokens, keywords, identifiers and constants correctly. Prefer modern C++ constructs.

## Key Decisions

| Construct     | Choice                          | Reason |
|---------------|---------------------------------|--------|
| Constants     | `constexpr`                     | Typed, scoped, compile-time |
| Zones         | `enum class`                    | No scope pollution, no implicit conversion to int |
| Avoid         | `#define` macros                | No type safety, no scope |

## Recommended Constants

```cpp
enum class Zone { Inner = 0, Middle = 1, Outer = 2 };

constexpr int BASE_PAISE[] = {1000, 2000, 3000};
constexpr int PEAK_SURCHARGE_PERCENT = 20;
constexpr long long MIN_BALANCE_PAISE = 5000;
constexpr int STUDENT_CONCESSION_PERCENT = 25;
constexpr int SENIOR_CONCESSION_PERCENT  = 30;
```

## How to Run

```bash
g++ -std=c++17 -Wall -Wextra -o task02 task02.cpp
./task02
```
