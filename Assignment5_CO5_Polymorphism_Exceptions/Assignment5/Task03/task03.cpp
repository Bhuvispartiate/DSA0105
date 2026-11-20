// Task 03 – Abstract Payer Classes (A3)
// OOP with C++ | Assignment 5 – CO5

#include <iostream>
#include <string>
#include <memory>

class Bill {
    double amount;
public:
    explicit Bill(double a) : amount(a) {}
    double total() const { return amount; }
};

class Payer {
public:
    virtual double amountCovered(const Bill& b) const = 0;
    virtual ~Payer() = default;
};

class SelfPay : public Payer {
public:
    double amountCovered(const Bill&) const override { return 0; }
};

class Insurance : public Payer {
    double limit, copayPercent;
public:
    Insurance(double lim, double copay) : limit(lim), copayPercent(copay) {}
    double amountCovered(const Bill& b) const override {
        double covered = b.total() * (1.0 - copayPercent / 100.0);
        return covered > limit ? limit : covered;
    }
};

class GovernmentScheme : public Payer {
    double packageRate;
public:
    explicit GovernmentScheme(double rate) : packageRate(rate) {}
    double amountCovered(const Bill& b) const override {
        return b.total() < packageRate ? b.total() : packageRate;
    }
};

int main() {
    std::cout << "===== Task 03 : Payer Hierarchy =====\n\n";

    Bill bill(20000);
    std::unique_ptr<Payer> payers[] = {
        std::make_unique<SelfPay>(),
        std::make_unique<Insurance>(15000, 20),
        std::make_unique<GovernmentScheme>(10000)
    };

    const char* names[] = {"SelfPay", "Insurance", "GovtScheme"};
    for (int i = 0; i < 3; ++i) {
        double covered = payers[i]->amountCovered(bill);
        std::cout << names[i] << " covers Rs " << covered
                  << "  (patient pays " << bill.total() - covered << ")\n";
    }
    return 0;
}
