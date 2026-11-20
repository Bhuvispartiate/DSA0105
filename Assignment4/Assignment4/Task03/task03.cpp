// Task 03 – Constructors in Derived Classes & Member Classes
// OOP with C++ | Assignment 4 – CO4

#include <iostream>
#include <string>

class Engine {
public:
    Engine() { std::cout << "Engine\n"; }
    ~Engine() { std::cout << "~Engine\n"; }
};

class Vehicle {
public:
    Vehicle() { std::cout << "Vehicle\n"; }
    ~Vehicle() { std::cout << "~Vehicle\n"; }
};

class Car : public Vehicle {
    Engine engine;          // member object
public:
    Car() { std::cout << "Car\n"; }
    ~Car() { std::cout << "~Car\n"; }
};

int main() {
    std::cout << "===== Task 03 : Construction / Destruction Order =====\n\n";
    {
        Car c;
        std::cout << "\n(end of scope)\n";
    }
    return 0;
}
