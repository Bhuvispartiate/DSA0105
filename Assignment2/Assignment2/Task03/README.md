# Task 03 – Q3 – Default Arguments and Function Overloading

**Marks:** 6  
**Topic:** Courier Charge Calculator

## Ambiguity Problem

Mixing overloads that differ only by default arguments can make a call ambiguous.  
Example: `charge(double)` and `charge(double, bool = false)` both match `charge(2.5)`.

## Clean Redesign

```cpp
double charge(double kg);
double charge(double kg, bool express);
double charge(double kg, bool express, bool insured);
double charge(int grams);
double charge(const Parcel& p);
```

Each distinct combination has its own signature, so every call has exactly one best match.

## Overload Resolution Ranking

exact match > promotion > standard conversion > user-defined conversion

## How to Run

```bash
g++ -std=c++17 -Wall -Wextra -o q3 q3.cpp
./q3
```
