# Task 03 – Constructors in Derived Classes & Member Classes

**Marks:** 9

## Construction Order
1. Virtual bases
2. Non-virtual bases (declaration order)
3. Member objects (declaration order)
4. Derived-class body

Destruction is the exact reverse.

## Example Output
```
Vehicle
Engine
Car
~Car
~Engine
~Vehicle
```

## How to Run
```bash
g++ -std=c++17 -Wall -Wextra -o task03 task03.cpp
./task03
```
