// Task 05 – Recovery and Rethrowing (A5)
// OOP with C++ | Assignment 5 – CO5

#include <iostream>
#include <stdexcept>
#include <string>

class Guard {
    std::string name;
public:
    explicit Guard(const std::string& n) : name(n) {
        std::cout << "+" << name << "\n";
    }
    ~Guard() { std::cout << "~" << name << "\n"; }
};

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
        std::cout << "audit: " << e.what() << "\n";
        throw;          // rethrow same object
    }
}

int main() {
    std::cout << "===== Task 05 : Recovery & Rethrow =====\n\n";
    Guard mainGuard("main");

    try {
        Guard f("f");
        std::cout << "settle 5000/10000 = " << settle(5000, 10000) << "\n";
        std::cout << "settle 15000/10000 = " << settle(15000, 10000) << "\n";
        std::cout << "not printed\n";
    } catch (const std::exception& e) {
        std::cout << "caught: " << e.what() << "\n";
    }

    return 0;
}
