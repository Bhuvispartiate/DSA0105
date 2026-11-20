# Task 05 – Write Correct Expressions with the Right Precedence

**Marks:** 4

## Objective
Write readable expressions for peak-hour checks, surcharge and concession using correct operator precedence.

## Recommended Expressions

```cpp
// Peak check – parentheses make the two intervals clear
bool isPeak = (hour >= 8 && hour < 10) || (hour >= 17 && hour < 20);

// Surcharge – pure integer arithmetic
long long fare = base + (base * PEAK_SURCHARGE_PERCENT) / 100;

// Concession
fare = fare - (fare * concessionPercent) / 100;
```

## Precedence Notes

- `* / %` bind more tightly than `+ -`
- Relational operators bind more tightly than `&&`, which binds more tightly than `||`
- Integer division truncates toward zero

## How to Run

```bash
g++ -std=c++17 -Wall -Wextra -o task05 task05.cpp
./task05
```
