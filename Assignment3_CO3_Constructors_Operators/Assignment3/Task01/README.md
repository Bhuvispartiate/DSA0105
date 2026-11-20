# Task 01 – Constructors and Constructor Overloading

**Marks:** 6  
**Topic:** Library Book Records

## Key Points

- All constructors use member initializer lists.
- Simpler constructors **delegate** to the most general one so validation lives in one place.
- A single-argument constructor is marked `explicit` to prevent unwanted implicit conversions.
- Members are always initialised in **declaration order**, not the order written in the initializer list.

## How to Run

```bash
g++ -std=c++17 -Wall -Wextra -o task01 task01.cpp
./task01
```
