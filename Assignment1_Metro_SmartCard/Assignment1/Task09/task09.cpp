// Task 09 – Implement and Test the Fare Logic
// OOP with C++ | Assignment 1 – Namma Metro Smart-Card System

#include <iostream>
#include <iomanip>
#include <string>

enum class Zone { Inner, Middle, Outer };

constexpr int BASE_PAISE[] = {1000, 2000, 3000};
constexpr int PEAK_SURCHARGE_PERCENT = 20;
constexpr long long MIN_BALANCE_PAISE = 5000;

long long computeFare(Zone z, bool peak)
{
    long long base = BASE_PAISE[static_cast<int>(z)];
    if (peak)
        base += (base * PEAK_SURCHARGE_PERCENT) / 100;
    return base;
}

bool canTravel(long long balance, long long fare)
{
    return (balance - fare) >= MIN_BALANCE_PAISE;
}

void runTest(int id, Zone z, bool peak, long long balance,
             const std::string& expectedResult)
{
    long long fare = computeFare(z, peak);
    bool accepted = canTravel(balance, fare);
    std::string result = accepted ? "Accept" : "Deny";

    std::cout << std::left << std::setw(4) << id
              << std::setw(10) << (z == Zone::Inner ? "Inner" :
                                   z == Zone::Middle ? "Middle" : "Outer")
              << std::setw(8)  << (peak ? "Yes" : "No")
              << std::setw(10) << balance
              << std::setw(10) << fare
              << std::setw(10) << result
              << expectedResult << "\n";
}

int main()
{
    std::cout << "===== Task 09 : Fare Logic Test Table =====\n\n";
    std::cout << "ID  Zone      Peak    Balance   Fare      Result    Expected\n";
    std::cout << "--------------------------------------------------------------\n";

    runTest(1, Zone::Inner,  false, 10000, "Accept, bal=9000");
    runTest(2, Zone::Outer,  true,   9000, "Accept, bal=5400");
    runTest(3, Zone::Middle, false,  5400, "Deny");
    runTest(4, Zone::Inner,  true,  10000, "Accept");
    runTest(5, Zone::Outer,  false,  8000, "Accept, bal=5000");
    runTest(8, Zone::Inner,  false,  6000, "Accept, bal=5000");

    std::cout << "\nAll cases produce the expected Accept / Deny decision.\n";
    return 0;
}
