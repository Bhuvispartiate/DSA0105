# Task 01 – Q1 – Call by Value, by Reference and by Pointer

**Marks:** 6  
**Topic:** Hostel Fee Concession Updater

## Three Implementations

```cpp
void applyByValue(long long fee, int percent);      // local copy
void applyByReference(long long& fee, int percent); // modifies caller
void applyByPointer(long long* fee, int percent);   // modifies via pointer
```

## Key Observations

- Call by value leaves the original fee unchanged.
- Call by reference and call by pointer both update the caller’s variable.
- Returning a reference (`int& bigger(...)`) allows the function result to appear on the left-hand side of an assignment.
- Default arguments are supplied from the right.

## Design Decision

For successive concessions the in-place reference interface is preferred because it matches the mental model “update this student’s fee”.

## How to Run

```bash
g++ -std=c++17 -Wall -Wextra -o q1 q1.cpp
./q1
```
