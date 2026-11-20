// Task 11 – Recommend and Defend the Final Design
// OOP with C++ | Assignment 1 – Namma Metro Smart-Card System

#include <iostream>

int main()
{
    std::cout << "===== Task 11 : Final Design Recommendation =====\n\n";

    std::cout << "Recommended design choices:\n\n";
    std::cout << "  Money representation : long long (paise)\n";
    std::cout << "  Zones                : enum class Zone + constexpr array\n";
    std::cout << "  Constants            : constexpr for all fares and limits\n";
    std::cout << "  Card                 : SmartCard with private balance\n";
    std::cout << "  Peak test            : parenthesised boolean expression\n";
    std::cout << "  Control flow         : switch (or array) for base fare\n\n";

    std::cout << "Fare revision requires changing only one entry in BASE_PAISE.\n";
    std::cout << "The design scales from 20 000 to 3 000 000 journeys without overflow.\n";

    return 0;
}
