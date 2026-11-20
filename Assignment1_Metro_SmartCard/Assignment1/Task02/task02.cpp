// Task 02 – Tokens, Keywords, Identifiers and Constants
// OOP with C++ | Assignment 1 – Namma Metro Smart-Card System

#include <iostream>

enum class Zone { Inner = 0, Middle = 1, Outer = 2 };

constexpr int BASE_PAISE[] = {1000, 2000, 3000};          // Rs 10, 20, 30
constexpr int PEAK_SURCHARGE_PERCENT = 20;
constexpr long long MIN_BALANCE_PAISE = 5000;             // Rs 50
constexpr int STUDENT_CONCESSION_PERCENT = 25;
constexpr int SENIOR_CONCESSION_PERCENT  = 30;

int main() {
    std::cout << "===== Task 02 : Constants Demo =====\n\n";

    std::cout << "Zone::Inner  base fare : " << BASE_PAISE[0] << " paise\n";
    std::cout << "Zone::Middle base fare : " << BASE_PAISE[1] << " paise\n";
    std::cout << "Zone::Outer  base fare : " << BASE_PAISE[2] << " paise\n\n";

    std::cout << "Peak surcharge         : " << PEAK_SURCHARGE_PERCENT << " %\n";
    std::cout << "Minimum balance        : " << MIN_BALANCE_PAISE << " paise\n";
    std::cout << "Student concession     : " << STUDENT_CONCESSION_PERCENT << " %\n";
    std::cout << "Senior concession      : " << SENIOR_CONCESSION_PERCENT << " %\n";

    return 0;
}
