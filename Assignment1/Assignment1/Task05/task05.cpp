// Task 05 – Expressions and Operator Precedence
// OOP with C++ | Assignment 1 – Namma Metro Smart-Card System

#include <iostream>

int main() {
    int hour = 9;
    bool isPeak = (hour >= 8 && hour < 10) || (hour >= 17 && hour < 20);

    long long base = 3000;
    int PEAK_SURCHARGE_PERCENT = 20;
    long long fare = base + (base * PEAK_SURCHARGE_PERCENT) / 100;

    int concessionPercent = 25;
    long long afterConcession = fare - (fare * concessionPercent) / 100;

    std::cout << "===== Task 05 : Expressions Demo =====\n\n";
    std::cout << "Hour                  : " << hour << "\n";
    std::cout << "Is peak hour?         : " << std::boolalpha << isPeak << "\n";
    std::cout << "Base fare (Outer)     : " << base << " paise\n";
    std::cout << "After 20% peak        : " << fare << " paise\n";
    std::cout << "After 25% concession  : " << afterConcession << " paise\n\n";

    // Precedence examples
    int x = 5, y = 3;
    std::cout << "x + y * 2             = " << (x + y * 2) << "\n";
    std::cout << "(x + y) * 2           = " << ((x + y) * 2) << "\n";
    std::cout << "10 / 3                = " << (10 / 3) << "\n";
    std::cout << "10 % 3                = " << (10 % 3) << "\n";
    std::cout << "-10 / 3               = " << (-10 / 3) << "\n";
    std::cout << "-10 % 3               = " << (-10 % 3) << "\n";

    return 0;
}
