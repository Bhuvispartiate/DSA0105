// Q2 – Return by Reference and Its Dangers
// OOP with C++ | Assignment 2 – CO2

#include <iostream>

int& highest(int marks[], int n) {
    int idx = 0;
    for (int i = 1; i < n; ++i)
        if (marks[i] > marks[idx]) idx = i;
    return marks[idx];          // reference to element inside array
}

// DANGEROUS – do not use
// int& bad() {
//     int local = 42;
//     return local;            // dangling reference
// }

int main() {
    std::cout << "===== Q2 : Return by Reference =====\n\n";

    int marks[] = {78, 92, 85, 67, 91};
    int n = 5;

    std::cout << "Original marks: ";
    for (int i = 0; i < n; ++i) std::cout << marks[i] << " ";
    std::cout << "\n";

    std::cout << "Highest mark  : " << highest(marks, n) << "\n";

    // Because highest returns a reference we can modify the element
    highest(marks, n) = highest(marks, n) + 2;

    std::cout << "After +2      : ";
    for (int i = 0; i < n; ++i) std::cout << marks[i] << " ";
    std::cout << "\n";

    std::cout << "\nNote: returning a reference to a local variable is undefined behaviour.\n";
    return 0;
}
