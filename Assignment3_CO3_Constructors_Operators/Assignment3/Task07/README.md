# Task 07 – Design a Complex-Number Class from First Principles

**Marks:** 10  
**Topic:** AC Circuit Impedance Calculator

## Class Summary

```cpp
class Complex {
    double re, im;
public:
    Complex(double r = 0, double i = 0);
    Complex operator+(const Complex&) const;
    Complex operator-(const Complex&) const;
    Complex operator*(const Complex&) const;
    Complex operator/(const Complex&) const;   // throws on zero
    bool    operator==(const Complex&) const;
    double  mag() const;
    friend std::ostream& operator<<(std::ostream&, const Complex&);
};
```

## Verification

- Series R–L: `30 + j40` → magnitude 50
- Parallel combination matches algebraic evaluation
- Division by zero throws a defined error

## How to Run

```bash
g++ -std=c++17 -Wall -Wextra -o task07 task07.cpp
./task07
```
