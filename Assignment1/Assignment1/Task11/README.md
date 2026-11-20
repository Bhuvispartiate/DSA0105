# Task 11 – Recommend and Defend the Final Design

**Marks:** 4

## Recommended Design

- Money stored as `long long` paise
- Zones modelled with `enum class` + parallel `constexpr` array
- All monetary constants declared `constexpr`
- `SmartCard` class with private balance and public `canTravel` / `deduct` / `recharge`
- Peak check written with clear parentheses
- Base fare obtained by `switch` or array lookup

## How a Fare Revision Is Performed

Change the single corresponding entry in the `BASE_PAISE` array, recompile and redeploy. No other source file needs to be edited.

## Limitations

- Assumes a single farthest-zone fare model
- Concession percentages are compile-time constants
- Monthly-pass support is added in Task 12

## How to Run

```bash
g++ -std=c++17 -Wall -Wextra -o task11 task11.cpp
./task11
```
