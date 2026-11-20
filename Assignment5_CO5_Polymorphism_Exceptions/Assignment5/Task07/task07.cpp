// Task 07 – Engineer Error Conditions (B2)
// OOP with C++ | Assignment 5 – CO5

#include <iostream>
#include <stdexcept>
#include <string>

class ClaimError : public std::runtime_error {
public:
    explicit ClaimError(const std::string& m) : std::runtime_error(m) {}
};

double approve(double billed, double cover) {
    if (billed < 0 || cover < 0)
        throw std::invalid_argument("negative amount");
    if (billed > cover)
        throw ClaimError("exceeds cover");
    return billed;
}

double settle(double billed, double cover) {
    try {
        return approve(billed, cover);
    } catch (const ClaimError& e) {
        std::cout << "[log] " << e.what() << "\n";
        throw;      // rethrow – do NOT write throw e;
    }
}

int main() {
    std::cout << "===== Task 07 : Error Conditions =====\n\n";

    try {
        std::cout << "ok " << settle(100, 500) << "\n";
    } catch (...) {}

    try {
        settle(-5, 100);
    } catch (const std::exception& e) {
        std::cout << "caught negative: " << e.what() << "\n";
    }

    try {
        settle(2000000, 100000);
    } catch (const std::exception& e) {
        std::cout << "caught too large: " << e.what() << "\n";
    }

    return 0;
}
