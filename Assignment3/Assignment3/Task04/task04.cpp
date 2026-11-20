// Task 04 – Overloading Arithmetic and Stream Operators
// OOP with C++ | Assignment 3 – CO3

#include <iostream>
#include <iomanip>

class Money {
    long long paise;
public:
    Money(long long rupees = 0, int p = 0) : paise(rupees * 100 + p) {}

    Money operator+(const Money& other) const {
        return Money(0, static_cast<int>(paise + other.paise));
    }

    bool operator==(const Money& other) const {
        return paise == other.paise;
    }

    friend std::ostream& operator<<(std::ostream& os, const Money& m) {
        long long r = m.paise / 100;
        int p = static_cast<int>(m.paise % 100);
        if (p < 0) p = -p;
        os << "Rs " << r << '.' << std::setw(2) << std::setfill('0') << p;
        return os;
    }
};

int main() {
    std::cout << "===== Task 04 : Money Operators =====\n\n";

    Money a(100, 0);          // Rs 100.00
    Money b(100, 0);
    Money c = a + b;

    std::cout << "a + b = " << c << "\n";
    std::cout << "a == b ? " << std::boolalpha << (a == b) << "\n";

    Money x(0, 75);
    Money y(0, 50);
    std::cout << "0.75 + 0.50 = " << (x + y) << "\n";

    return 0;
}
