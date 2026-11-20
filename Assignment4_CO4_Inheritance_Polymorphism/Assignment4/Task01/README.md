# Task 01 – Class Design Review (Diagnose & Repair)

**Marks:** 6

## Problems in the Original Design
- Repeated fields and pay formula across classes
- Public data members
- Magic number 0.42 scattered
- No common base → cannot process polymorphically

## Repaired Hierarchy
```
Staff (abstract)
 ├── Professor
 ├── LabAssistant
 └── GuestFaculty
```

- `pay()` is pure virtual
- DA rate is a single `static constexpr`
- New staff types are added by writing one new derived class

## How to Run
```bash
g++ -std=c++17 -Wall -Wextra -o task01 task01.cpp
./task01
```
