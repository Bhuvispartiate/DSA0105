// Task 03 – Choose Data Types and Check Type Compatibility
// OOP with C++ | Assignment 1 – Namma Metro Smart-Card System

#include <iostream>
#include <climits>

int main() {
    std::cout << "===== Task 03 : Data Type Choices =====\n\n";

    std::cout << "Value            Type          Size (bytes)   Reason\n";
    std::cout << "----------------------------------------------------------------\n";
    std::cout << "Fare / balance   long long     " << sizeof(long long)
              << "             Exact arithmetic, no floating error\n";
    std::cout << "Zone index       enum class    4             Small discrete set\n";
    std::cout << "Hour (0-23)      int           " << sizeof(int)
              << "             Simple arithmetic\n";
    std::cout << "Peak flag        bool          " << sizeof(bool)
              << "             True / false only\n";
    std::cout << "Journey count    long long     " << sizeof(long long)
              << "             Supports millions of journeys\n";
    std::cout << "Concession %     int           " << sizeof(int)
              << "             Small integer percentage\n\n";

    std::cout << "INT_MAX          = " << INT_MAX << "\n";
    std::cout << "Why integer paise? Floating-point cannot represent 0.10 exactly.\n";

    return 0;
}
