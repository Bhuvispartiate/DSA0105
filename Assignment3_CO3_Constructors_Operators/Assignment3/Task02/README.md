# Task 02 – Copy Constructors: Shallow and Deep Copies

**Marks:** 7  
**Topic:** Hospital Prescription Records

## The Problem

Without a user-defined copy constructor the compiler generates a shallow (member-wise) copy.  
Both objects then share the same pointer → double-free when both are destroyed.

## Rule of Three

If a class manages a resource it needs:
1. Destructor
2. Copy constructor
3. Copy-assignment operator

## How to Run

```bash
g++ -std=c++17 -Wall -Wextra -o task02 task02.cpp
./task02
```
