# Task 05 – Recovery and Rethrowing (A5)

**Marks:** 4

## Catch Strategy
- Low-level validation throws
- Mid-level `settle()` catches, audits, then `throw;` (rethrow)
- Top-level UI catches `std::exception`, shows message, continues

## RAII
When an exception leaves a block, automatic objects are destroyed in reverse order of construction.  
No explicit `delete` is needed in catch clauses.

## How to Run
```bash
g++ -std=c++17 -Wall -Wextra -o task05 task05.cpp
./task05
```
