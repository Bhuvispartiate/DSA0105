# Task 02 – From Requirement to Hierarchy (University People)

**Marks:** 8

## Relationships
- Student is-a Person
- Staff is-a Person
- TeachingAssistant is-a Student **and** is-a Staff → diamond
- Solved with **virtual inheritance**

## Diamond Problem
Without `virtual`, TeachingAssistant contains two Person sub-objects → ambiguous access.  
With `virtual public Person`, only one shared Person sub-object exists.

## How to Run
```bash
g++ -std=c++17 -Wall -Wextra -o task02 task02.cpp
./task02
```
