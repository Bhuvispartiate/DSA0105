# Task 02 – Runtime Binding (A2)

**Marks:** 5

## Early vs Late Binding
- Non-virtual call → bound at **compile time** (static type)
- Virtual call through base pointer → bound at **run time** (dynamic type)

## Virtual Destructor
`delete b` where `b` is `Bill*` must invoke the most-derived destructor.  
Without a virtual destructor only `~Bill` runs → resource leak.

## How to Run
```bash
g++ -std=c++17 -Wall -Wextra -o task02 task02.cpp
./task02
```
