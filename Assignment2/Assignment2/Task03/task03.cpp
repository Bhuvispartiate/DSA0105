// Q3 – Default Arguments and Function Overloading
// OOP with C++ | Assignment 2 – CO2

#include <iostream>

struct Parcel {
    double kg;
    bool express  = false;
    bool insured  = false;
};

double charge(double kg) {
    return kg * 50.0;                    // basic Rs 50 per kg
}

double charge(double kg, bool express) {
    double c = kg * 50.0;
    if (express) c *= 1.5;
    return c;
}

double charge(double kg, bool express, bool insured) {
    double c = charge(kg, express);
    if (insured) c += 100.0;
    return c;
}

double charge(int grams) {
    return charge(grams / 1000.0);
}

double charge(const Parcel& p) {
    return charge(p.kg, p.express, p.insured);
}

int main() {
    std::cout << "===== Q3 : Overload Resolution =====\n\n";

    std::cout << "charge(2)            = " << charge(2) << "\n";
    std::cout << "charge(2.0)          = " << charge(2.0) << "\n";
    std::cout << "charge(2.5, true)    = " << charge(2.5, true) << "\n";
    std::cout << "charge(2.5, true, true) = " << charge(2.5, true, true) << "\n";

    Parcel p{3.0, true, false};
    std::cout << "charge(p)            = " << charge(p) << "\n";

    return 0;
}
