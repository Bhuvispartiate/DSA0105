// Task 04 – Exception Strategy (A4)
// OOP with C++ | Assignment 5 – CO5

#include <iostream>
#include <stdexcept>
#include <string>

class BillingError : public std::runtime_error {
public:
    explicit BillingError(const std::string& msg) : std::runtime_error(msg) {}
};

class ClaimError : public BillingError {
public:
    explicit ClaimError(const std::string& msg) : BillingError(msg) {}
};

class PackageError : public BillingError {
public:
    explicit PackageError(const std::string& msg) : BillingError(msg) {}
};

double computeCharge(double amount) {
    if (amount < 0)
        throw BillingError("negative charge not allowed");
    return amount * 1.18;   // with GST
}

int main() {
    std::cout << "===== Task 04 : Exception Hierarchy =====\n\n";

    try {
        std::cout << "Valid charge: " << computeCharge(1000) << "\n";
        std::cout << "Invalid charge: " << computeCharge(-50) << "\n";
    } catch (const BillingError& e) {
        std::cout << "Caught BillingError: " << e.what() << "\n";
    }

    return 0;
}
