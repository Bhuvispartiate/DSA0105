# Task 08 – Virtual Functions vs switch vs Exceptions (B3)

**Marks:** 8

## Engineering Trade-Off
- Virtual calls cost a few nanoseconds but keep the design **open**
- A switch is marginally faster yet **closed**
- Exceptions are ideal for rare errors (<1 %); return codes are cheaper when errors are common

## How to Run
```bash
g++ -std=c++17 -Wall -Wextra -o task08 task08.cpp
./task08
```
