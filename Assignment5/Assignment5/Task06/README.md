# Task 06 – Implement and Validate Polymorphic Billing (B1)

**Marks:** 7

## Validation
| Bill | Hand calculation      | Program |
|------|-----------------------|---------|
| OP   | 500 + 1200 = 1700     | 1700    |
| IP   | 3×2500 + 40000 = 47500| 47500   |
| Day  | 1700 + 47500 = 49200  | 49200   |

Removing `virtual` from `total()` breaks polymorphic behaviour through a base pointer.

## How to Run
```bash
g++ -std=c++17 -Wall -Wextra -o task06 task06.cpp
./task06
```
