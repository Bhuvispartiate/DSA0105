# Task 07 – Engineer Error Conditions (B2)

**Marks:** 5

## Deliberate Faults Tested
- Negative charges → `invalid_argument`
- Claim above cover → `ClaimError` (audited then rethrown)
- Use `throw;` (not `throw e;`) to avoid slicing

## How to Run
```bash
g++ -std=c++17 -Wall -Wextra -o task07 task07.cpp
./task07
```
