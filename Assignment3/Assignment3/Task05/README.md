# Task 05 – Overloading Increment, Comparison and Subscript

**Marks:** 6  
**Topic:** Music Playlist and Track Timer

## Prefix vs Postfix

```cpp
Time& operator++();      // prefix  – increment, return new value
Time  operator++(int);   // postfix – return old value, then increment
```

The dummy `int` parameter tells the compiler which version is intended.  
Postfix must return a **copy** of the old state, making it more expensive for class types.

Prefer `++i` for user-defined types.

## How to Run

```bash
g++ -std=c++17 -Wall -Wextra -o task05 task05.cpp
./task05
```
