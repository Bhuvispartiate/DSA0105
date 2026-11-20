// Task 08 – Read and Print Data Safely
// OOP with C++ | Assignment 1 – Namma Metro Smart-Card System

#include <iostream>
#include <iomanip>
#include <limits>

int main()
{
    std::cout << "===== Task 08 : Safe Input / Formatted Output =====\n\n";

    // Demonstration of the safe input pattern (non-interactive for this file)
    int hour = 9;   // pretend validated input
    std::cout << "Validated hour (0-23) : " << hour << "\n";

    bool isPeak = (hour >= 8 && hour < 10) || (hour >= 17 && hour < 20);
    std::cout << "Peak hour?            : " << std::boolalpha << isPeak << "\n\n";

    // Formatted money output
    long long farePaise = 3605;   // Rs 36.05
    std::cout << "Fare Rs "
              << (farePaise / 100) << '.'
              << std::setw(2) << std::setfill('0') << (farePaise % 100)
              << "\n";

    farePaise = 5;                // Rs 0.05
    std::cout << "Fare Rs "
              << (farePaise / 100) << '.'
              << std::setw(2) << std::setfill('0') << (farePaise % 100)
              << "\n";

    return 0;
}
