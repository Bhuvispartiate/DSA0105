// Task 04 – Control Implicit Conversions
// OOP with C++ | Assignment 1 – Namma Metro Smart-Card System

#include <iostream>
#include <iomanip>

int main() {
    int a = 7, b = 2;
    double c = a / b;                               // integer division
    double d = static_cast<double>(a) / b;          // correct floating division

    char ch = 'A' + 2;

    int s = -1;
    unsigned int u = 1;

    std::cout << "===== Task 04 : Implicit Conversion Demo =====\n\n";

    std::cout << "7 / 2  (integer)          = " << c << "\n";
    std::cout << "7 / 2  (with cast)        = " << d << "\n";
    std::cout << "'A' + 2                   = " << ch << "\n";
    std::cout << "ch + 1 (promoted to int)  = " << (ch + 1) << "\n";
    std::cout << "bool(5)                   = " << std::boolalpha << bool(5) << "\n";
    std::cout << "(-1 < 1u)                 = " << (s < u ? "true (wrong!)" : "false") << "\n";
    std::cout << "\nLesson: always use explicit casts for clarity and safety.\n";

    return 0;
}
