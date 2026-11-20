# Task 06 – The Rules of Operator Overloading

**Marks:** 7  
**Topic:** Simple Text Class for an SMS Gateway

## Operators That Cannot Be Overloaded

`::`  `.`  `.*`  `?:`  `sizeof`  `typeid`  `alignof`

## Core Rules

- Precedence, associativity and arity are fixed.
- At least one operand must be of a user-defined type.
- `operator=`, `operator[]`, `operator()`, `operator->` must be non-static members.

## How to Run

```bash
g++ -std=c++17 -Wall -Wextra -o task06 task06.cpp
./task06
```
