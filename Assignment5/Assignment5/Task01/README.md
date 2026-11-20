# Task 01 – Polymorphic Bill Hierarchy (A1)

**Marks:** 7

## Hierarchy
```
Bill (abstract – pure virtual total())
 ├── OutPatientBill
 ├── InPatientBill
 ├── DayCareBill
 └── EmergencyBill
```

- `total()` is pure virtual
- `type()` / `print()` / destructor are virtual
- Processing uses late binding: `bills[i]->total()`

## How to Run
```bash
g++ -std=c++17 -Wall -Wextra -o task01 task01.cpp
./task01
```
