// Q1 – Call by Value, by Reference and by Pointer
// OOP with C++ | Assignment 2 – CO2

#include <iostream>

void applyByValue(long long fee, int percent) {
    fee -= fee * percent / 100;          // local copy only
}

void applyByReference(long long& fee, int percent) {
    fee -= fee * percent / 100;          // changes caller
}

void applyByPointer(long long* fee, int percent) {
    *fee -= *fee * percent / 100;        // changes caller via pointer
}

int& bigger(int& x, int& y) {
    return (x > y) ? x : y;
}

void show(int x, int y = 5, int z = 10) {
    std::cout << (x + y + z) << " ";
}

int main() {
    std::cout << "===== Q1 : Call by Value / Reference / Pointer =====\n\n";

    long long fee1 = 50000;
    long long fee2 = 50000;
    long long fee3 = 50000;

    applyByValue(fee1, 10);
    applyByReference(fee2, 10);
    applyByPointer(&fee3, 10);

    std::cout << "After 10% concession:\n";
    std::cout << "  Call by value     : " << fee1 << " (unchanged)\n";
    std::cout << "  Call by reference : " << fee2 << "\n";
    std::cout << "  Call by pointer   : " << fee3 << "\n\n";

    int a = 1, b = 12, c = 13;
    std::cout << a << " " << b << " " << c << "\n";

    bigger(a, c) = 100;
    std::cout << a << " " << c << "\n";

    show(1);
    show(1, 2);
    show(1, 2, 3);
    std::cout << "\n";

    return 0;
}
