# Task 09 – Final Engineering Decision (B4)

**Marks:** 5

## Production Decision
Given 200 000 bills/day and error rate well below 1 %:
- Polymorphic design with exceptions is suitable
- Measured overhead is negligible compared with I/O
- RAII + virtual destructor keep the system leak-free

## How to Run
```bash
g++ -std=c++17 -Wall -Wextra -o task09 task09.cpp
./task09
```
