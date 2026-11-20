# Task 05 – Q5 – Static Members and Arrays of Objects

**Marks:** 7  
**Topic:** College Bus Seat Tickets

## Class Design

```cpp
class BusTicket {
    static int issued;
    static const int CAPACITY = 3;
    int seat;
    std::string name;
public:
    explicit BusTicket(const std::string& n);
    static int count() { return issued; }
    void print() const;
};
int BusTicket::issued = 0;   // definition required
```

## Why a Static Member Function Cannot Use `this`

A static member function is not invoked on a particular object, therefore it has no `this` pointer and cannot access non-static members.

## Key Point

Because `issued` is static its value persists across function calls and is shared by every `BusTicket` object.

## How to Run

```bash
g++ -std=c++17 -Wall -Wextra -o q5 q5.cpp
./q5
```
