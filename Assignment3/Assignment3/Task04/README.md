# Task 04 – Overloading Arithmetic and Stream Operators

**Marks:** 7  
**Topic:** Money Arithmetic for a Billing Counter

## Design Decisions

- `+` returns a **new** object (the sum is a distinct value).
- `<<` must be a **non-member** (usually a friend) because its left operand is an `ostream`.
- Only operators that preserve the money abstraction are overloaded (no Money × Money).

## How to Run

```bash
g++ -std=c++17 -Wall -Wextra -o task04 task04.cpp
./task04
```
