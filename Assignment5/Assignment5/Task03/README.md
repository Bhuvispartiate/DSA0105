# Task 03 – Abstract Classes – Payer (A3)

**Marks:** 5

## Payer Interface
```cpp
class Payer {
public:
    virtual double amountCovered(const Bill& b) const = 0;
    virtual ~Payer() = default;
};
```

Derived: `SelfPay`, `Insurance`, `GovernmentScheme`.  
A new `CorporateTieUp` can be added without modifying existing code.

## How to Run
```bash
g++ -std=c++17 -Wall -Wextra -o task03 task03.cpp
./task03
```
