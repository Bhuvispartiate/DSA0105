# Task 07 – Q7 – Day-4 Design Challenge

**Marks:** 13  
**Topic:** Diagnose, Redesign and Validate

## A. Diagnose the Limitation

The original design (Q4) shifted elements on every insertion → O(k) cost.  
When k grew to 50 and 10 000 transactions arrived, the cumulative work became noticeable.

## B. Design Objective

Achieve **O(1)** time per recorded transaction while keeping a fixed upper bound on memory and an unchanged public interface.

## C. Revised Design – Circular Buffer

```cpp
long long trans[50];
int head = 0;    // next write position
int count = 0;   // how many valid entries

void record(long long amount) {
    trans[head] = amount;
    head = (head + 1) % 50;
    if (count < 50) ++count;
}
```

No element shifting occurs. Extra memory is only two integers.

## D. Validation

| Operation              | Original (shift) | Redesign (circular) |
|------------------------|------------------|---------------------|
| 1 insert (k=50)        | O(49) moves      | O(1)                |
| 10 000 inserts (k=50)  | noticeable delay | instant             |

Functional results (deposit, withdraw, mini-statement order) remain identical.

## How to Run

```bash
g++ -std=c++17 -Wall -Wextra -o q7 q7.cpp
./q7
```
