# Task 03 – Dynamic Constructors and Destructors

**Marks:** 7  
**Topic:** Auditorium Seat Booking

## Key Points

- Constructor allocates a 2-D seat map with `new`.
- Destructor releases every row and the row-pointer array.
- Local automatic objects are destroyed in **reverse order of construction**.
- RAII guarantees that every exit path (normal return, exception, early return) still frees memory.

## How to Run

```bash
g++ -std=c++17 -Wall -Wextra -o task03 task03.cpp
./task03
```
