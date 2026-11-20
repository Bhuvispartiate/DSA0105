# Task 04 – Q4 – Specifying a Class with an Array Inside It

**Marks:** 6  
**Topic:** Savings Account Mini-Statement

## Class Design

```cpp
class SavingsAccount {
private:
    long long accNo;
    long long balance;       // paise
    long long trans[10];     // last 10 transactions
    int count;
public:
    void deposit(long long amount);
    bool withdraw(long long amount);
    void miniStatement() const;
};
```

## Handling the 11th Transaction

Two strategies:
- **Shift left** – O(k) cost
- **Circular buffer** – O(1) cost (used in Q7)

The array stays private so clients cannot corrupt the count or ordering invariants.

## How to Run

```bash
g++ -std=c++17 -Wall -Wextra -o q4 q4.cpp
./q4
```
