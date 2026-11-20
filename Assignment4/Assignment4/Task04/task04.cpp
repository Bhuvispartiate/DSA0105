// Task 04 – Pointers to Derived Classes: Forensics
// OOP with C++ | Assignment 4 – CO4

#include <iostream>

class Account {
public:
    virtual double rate() const { return 3.5; }
    virtual ~Account() { std::cout << "~Account\n"; }
};

class SeniorAccount : public Account {
    double* history;
public:
    SeniorAccount() : history(new double[100]()) {
        std::cout << "SeniorAccount created\n";
    }
    double rate() const override { return 4.0; }
    ~SeniorAccount() override {
        delete[] history;
        std::cout << "~SeniorAccount\n";
    }
};

int main() {
    std::cout << "===== Task 04 : Virtual Functions & Destructor =====\n\n";

    Account* p = new SeniorAccount;
    std::cout << "rate through Account* : " << p->rate() << "\n";

    // proper polymorphic deletion
    delete p;

    std::cout << "\nSlicing demonstration:\n";
    SeniorAccount sa;
    Account copy = sa;          // slices – only Account part kept
    std::cout << "sliced rate : " << copy.rate() << "\n";

    return 0;
}
