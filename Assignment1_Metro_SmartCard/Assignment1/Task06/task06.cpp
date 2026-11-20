// Task 06 – Design the Control Structures
// OOP with C++ | Assignment 1 – Namma Metro Smart-Card System

#include <iostream>

enum class Zone { Inner = 0, Middle = 1, Outer = 2 };

constexpr int BASE_PAISE[] = {1000, 2000, 3000};
constexpr int PEAK_SURCHARGE_PERCENT = 20;
constexpr long long MIN_BALANCE_PAISE = 5000;

long long computeFare(Zone z, bool peak)
{
    long long base = 0;
    switch (z) {
        case Zone::Inner:  base = BASE_PAISE[0]; break;
        case Zone::Middle: base = BASE_PAISE[1]; break;
        case Zone::Outer:  base = BASE_PAISE[2]; break;
        default:           return -1;
    }
    if (peak)
        base += (base * PEAK_SURCHARGE_PERCENT) / 100;
    return base;
}

bool canTravel(long long balance, long long fare)
{
    return (balance - fare) >= MIN_BALANCE_PAISE;
}

int main()
{
    std::cout << "===== Task 06 : Control Structures =====\n\n";

    std::cout << "Inner  off-peak : " << computeFare(Zone::Inner,  false) << " paise\n";
    std::cout << "Middle off-peak : " << computeFare(Zone::Middle, false) << " paise\n";
    std::cout << "Outer  off-peak : " << computeFare(Zone::Outer,  false) << " paise\n";
    std::cout << "Outer  peak     : " << computeFare(Zone::Outer,  true)  << " paise\n\n";

    long long bal = 10000;
    long long f   = computeFare(Zone::Outer, true);   // 3600

    std::cout << "Balance = " << bal << ", Fare = " << f << "\n";
    std::cout << "canTravel? " << std::boolalpha << canTravel(bal, f) << "\n";

    bal = 5400;
    f   = computeFare(Zone::Middle, false);           // 2000
    std::cout << "Balance = " << bal << ", Fare = " << f << "\n";
    std::cout << "canTravel? " << canTravel(bal, f) << "\n";

    return 0;
}
