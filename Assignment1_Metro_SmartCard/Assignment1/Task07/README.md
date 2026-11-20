# Task 07 – From Procedural to Object-Oriented

**Marks:** 4

## Objective
Convert a procedural balance-management approach into a proper class that enforces the minimum-balance rule.

## Procedural Problems
Any function can read or write a global balance variable, making it easy to violate the ₹50 rule.

## Object-Oriented Solution
```cpp
class SmartCard {
private:
    long long balancePaise;
public:
    bool canTravel(long long fare) const;
    bool deduct(long long fare);   // calls canTravel internally
    void recharge(long long amount);
};
```

The private balance can be changed only through the member functions, guaranteeing the rule is never broken from outside the class.

## How to Run

```bash
g++ -std=c++17 -Wall -Wextra -o task07 task07.cpp
./task07
```
