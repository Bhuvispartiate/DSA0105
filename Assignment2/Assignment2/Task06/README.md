# Task 06 – Q6 – Friend Functions and Inline Functions

**Marks:** 8  
**Topic:** Train Journey Planner

## Why a Friend Function Is Needed

`minutesBetween` and `earlier` operate symmetrically on two `Time` objects.  
A member function would force an asymmetric call style (`a.minutesTo(b)`).  
A free friend function can access the private data of both arguments while preserving a natural binary-operator appearance.

## Working Friend Functions

```cpp
inline int  minutesBetween(const Time& a, const Time& b);
inline bool earlier(const Time& a, const Time& b);
```

## Inline Decision

These functions perform only a few arithmetic operations. Declaring them `inline` (or defining them in the header) asks the compiler to substitute the body at each call site, removing call overhead.

## How to Run

```bash
g++ -std=c++17 -Wall -Wextra -o q6 q6.cpp
./q6
```
