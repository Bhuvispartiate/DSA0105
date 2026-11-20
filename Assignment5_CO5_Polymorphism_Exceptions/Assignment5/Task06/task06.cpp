// Task 06 – Implement and Validate Polymorphic Billing (B1)
// OOP with C++ | Assignment 5 – CO5

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <iomanip>

class Bill {
protected:
    std::string patient;
public:
    explicit Bill(const std::string& p) : patient(p) {}
    virtual double total() const = 0;
    virtual std::string type() const { return "Bill"; }
    virtual void print() const {
        std::cout << type() << " | " << patient
                  << " | Rs " << std::fixed << std::setprecision(2) << total() << "\n";
    }
    virtual ~Bill() = default;
};

class OutPatientBill : public Bill {
    double consultation, tests;
public:
    OutPatientBill(const std::string& p, double c, double t)
        : Bill(p), consultation(c), tests(t) {}
    double total() const override { return consultation + tests; }
    std::string type() const override { return "OP"; }
};

class InPatientBill : public Bill {
    int days; double perDay, surgery;
public:
    InPatientBill(const std::string& p, int d, double rate, double surg)
        : Bill(p), days(d), perDay(rate), surgery(surg) {}
    double total() const override { return days * perDay + surgery; }
    std::string type() const override { return "IP"; }
};

int main() {
    std::cout << "===== Task 06 : Validate Polymorphic Billing =====\n\n";

    // Hand calculation: OP 500+1200=1700 ; IP 3*2500+40000=47500 ; day=49200
    std::vector<std::unique_ptr<Bill>> bills;
    bills.push_back(std::make_unique<OutPatientBill>("Ravi", 500, 1200));
    bills.push_back(std::make_unique<InPatientBill>("Priya", 3, 2500, 40000));

    double sum = 0;
    for (const auto& b : bills) {
        b->print();
        sum += b->total();
    }
    std::cout << "\nDay total = Rs " << sum
              << "  (expected 49200)\n";
    return 0;
}
