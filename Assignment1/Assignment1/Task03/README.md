# Task 03 – Choose Data Types and Check Type Compatibility

**Marks:** 4

## Objective
Select appropriate data types for the metro fare system and understand type compatibility issues.

## Data-Type Decisions

| Value            | Chosen Type   | Reason                                      |
|------------------|---------------|---------------------------------------------|
| Fare / balance   | `long long`   | Exact arithmetic, safe for network totals   |
| Zone index       | `enum class`  | Small discrete set, type-safe               |
| Hour (0–23)      | `int`         | Simple arithmetic, no overflow risk         |
| Peak flag        | `bool`        | True / false only                           |
| Journey count    | `long long`   | Millions of journeys need > 32-bit range    |
| Concession %     | `int`         | Small integer percentage                    |

## Why Money Must Be Stored in Integer Paise

Floating-point types cannot represent most decimal fractions exactly.  
Adding ₹0.10 many times produces rounding error.  
Storing every amount as an integer number of paise guarantees exact arithmetic.

## How to Run

```bash
g++ -std=c++17 -Wall -Wextra -o task03 task03.cpp
./task03
```
