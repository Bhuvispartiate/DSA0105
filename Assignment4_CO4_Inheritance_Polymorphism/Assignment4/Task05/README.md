# Task 05 – Pointers to Objects and the this Pointer

**Marks:** 7

## Four Ways to Reach an Object
| Way            | Example                     | Notes                          |
|----------------|-----------------------------|--------------------------------|
| Direct name    | `Cart c; c.add(...);`       | Safest for locals              |
| Pointer        | `Cart* p = &c; p->add(...);`| May be null; needed for poly   |
| Reference      | `Cart& r = c; r.add(...);`  | Cannot be null, cannot reseat  |
| Dynamic        | `new Cart; ...; delete`     | Prefer smart pointers          |

## Method Chaining
Returning `*this` (by reference) allows:
```cpp
c.add("A", 10).add("B", 20).setDiscount(0.1);
```

## How to Run
```bash
g++ -std=c++17 -Wall -Wextra -o task05 task05.cpp
./task05
```
