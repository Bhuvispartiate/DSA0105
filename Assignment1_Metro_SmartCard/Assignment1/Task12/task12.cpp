// Task 12 – Day-4 Design Challenge
// OOP with C++ | Assignment 1 – Namma Metro Smart-Card System

#include <iostream>

enum class Zone { Inner, Middle, Outer, Airport };

constexpr int BASE_PAISE[] = {1000, 2000, 3000, 6000};   // Airport = Rs 60
constexpr int PEAK_SURCHARGE_PERCENT = 20;
constexpr int WEEKEND_DISCOUNT_PERCENT = 10;

long long computeFare(Zone z, bool peak, bool weekend)
{
    long long base = BASE_PAISE[static_cast<int>(z)];
    if (weekend)
        base -= (base * WEEKEND_DISCOUNT_PERCENT) / 100;   // 10 % discount
    else if (peak)
        base += (base * PEAK_SURCHARGE_PERCENT) / 100;     // 20 % surcharge
    return base;
}

class MonthlyPass {
    Zone zoneA, zoneB;
public:
    MonthlyPass(Zone a, Zone b) : zoneA(a), zoneB(b) {}
    bool covers(Zone z) const {
        return z == zoneA || z == zoneB;
    }
};

int main()
{
    std::cout << "===== Task 12 : Extended Fare Logic =====\n\n";

    std::cout << "Airport off-peak  : " << computeFare(Zone::Airport, false, false) << " paise\n";
    std::cout << "Airport peak      : " << computeFare(Zone::Airport, true,  false) << " paise\n";
    std::cout << "Airport weekend   : " << computeFare(Zone::Airport, true,  true)  << " paise\n";
    std::cout << "(weekend + peak both true → only weekend discount applied)\n\n";

    MonthlyPass pass(Zone::Inner, Zone::Middle);
    std::cout << "MonthlyPass covers Inner?  " << std::boolalpha << pass.covers(Zone::Inner)  << "\n";
    std::cout << "MonthlyPass covers Outer?  " << pass.covers(Zone::Outer)  << "\n";
    std::cout << "MonthlyPass covers Airport?" << pass.covers(Zone::Airport) << "\n";

    return 0;
}
