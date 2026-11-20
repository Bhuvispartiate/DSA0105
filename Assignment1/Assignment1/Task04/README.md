# Task 04 – Control Implicit Conversions

**Marks:** 4

## Objective
Understand common implicit conversions and control them with explicit casts.

## Common Conversion Points in the Fare System

- Integer division truncates toward zero
- Integer-to-double needed for percentage calculations
- Signed / unsigned comparison can reverse the result
- Narrowing conversions lose information
- Non-zero integer becomes `true` when converted to `bool`

## Demonstration Output

```
7 / 2  (integer)          = 3
7 / 2  (with cast)        = 3.5
'A' + 2                   = C
ch + 1 (promoted to int)  = 68
bool(5)                   = true
(-1 < 1u)                 = false
```

The signed/unsigned comparison produces a surprising result because `-1` is converted to a large unsigned value.

## Recommended Practice

- Prefer brace initialisation to catch narrowing at compile time
- Write every intentional conversion with `static_cast`
- Compile with `-Wall -Wextra`

## How to Run

```bash
g++ -std=c++17 -Wall -Wextra -o task04 task04.cpp
./task04
```
