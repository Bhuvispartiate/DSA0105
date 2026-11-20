# Task 12 – Day-4 Design Challenge

**Marks:** 8

## Changed Requirements

- Fourth zone: Airport (base fare ₹60)
- 10 % weekend discount that does **not** combine with peak surcharge
- Monthly pass allowing unlimited journeys in two chosen zones

## Design Decision

**RETAIN** the original SmartCard for ordinary stored-value cards.  
**MODIFY** the zone enumeration and fare function for the Airport zone and weekend rule.  
**EXTEND** the system with a separate `MonthlyPass` class.

## New Fare Function

```cpp
long long computeFare(Zone z, bool peak, bool weekend) {
    long long base = BASE_PAISE[static_cast<int>(z)];
    if (weekend)
        base -= (base * 10) / 100;
    else if (peak)
        base += (base * 20) / 100;
    return base;
}
```

## How to Run

```bash
g++ -std=c++17 -Wall -Wextra -o task12 task12.cpp
./task12
```
