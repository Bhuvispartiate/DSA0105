// Task 10 – Compare Expectations with Execution
// OOP with C++ | Assignment 1 – Namma Metro Smart-Card System

#include <iostream>
#include <climits>

int main()
{
    std::cout << "===== Task 10 : Accumulator Type Comparison =====\n\n";

    std::cout << "Assuming average fare = 2000 paise (Rs 20)\n\n";
    std::cout << "Journeys     Expected (paise)   long long   int            double\n";
    std::cout << "--------------------------------------------------------------------\n";
    std::cout << "20 000       40 000 000         exact       exact          exact\n";
    std::cout << "500 000      1 000 000 000      exact       overflow risk  still exact\n";
    std::cout << "3 000 000    6 000 000 000      exact       overflow       loses low bits\n\n";

    std::cout << "INT_MAX = " << INT_MAX << "\n";
    std::cout << "Conclusion : only long long remains exact for network-scale totals.\n";

    return 0;
}
