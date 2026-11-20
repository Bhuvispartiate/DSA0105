# Task 04 – Pointers to Derived Classes: Forensics

**Marks:** 7

## Defects Fixed
1. `rate()` must be **virtual** (otherwise static binding)
2. Base destructor must be **virtual** (otherwise only `~Account` runs → leak)
3. Never store derived objects **by value** in a base variable (slicing)

## Prevention Rules
- Any base intended for polymorphic deletion needs a virtual destructor
- Functions meant to be overridden should be `virtual` + `override`

## How to Run
```bash
g++ -std=c++17 -Wall -Wextra -o task04 task04.cpp
./task04
```
