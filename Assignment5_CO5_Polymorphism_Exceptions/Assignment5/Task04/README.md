# Task 04 – Exception Strategy (A4)

**Marks:** 4

## Exception Hierarchy
```
std::runtime_error
 └── BillingError
      ├── ClaimError
      ├── PackageError
      └── RecordError
```

- Destructors are `noexcept` and must never throw
- Accessors that only read validated state can be marked `noexcept`

## How to Run
```bash
g++ -std=c++17 -Wall -Wextra -o task04 task04.cpp
./task04
```
