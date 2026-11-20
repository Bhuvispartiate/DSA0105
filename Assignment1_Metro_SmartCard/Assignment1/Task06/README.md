# Task 06 – Design the Control Structures

**Marks:** 4

## Objective
Choose appropriate decision and looping structures for the fare calculation.

## Decisions

- `switch` on zone (small discrete set)
- `if` for peak surcharge and canTravel test
- Range-based `for` preferred when processing a list of journeys

## Core Functions

```cpp
long long computeFare(Zone z, bool peak);
bool canTravel(long long balance, long long fare);
```

Boundary test: hour == 10 and hour == 20 correctly evaluate to off-peak.

## How to Run

```bash
g++ -std=c++17 -Wall -Wextra -o task06 task06.cpp
./task06
```
