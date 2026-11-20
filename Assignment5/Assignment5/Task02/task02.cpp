// Task 02 – Runtime Binding (A2)
// OOP with C++ | Assignment 5 – CO5

#include <iostream>
#include <string>

class Bill {
public:
    virtual double total() const { return 0; }
    virtual ~Bill() { std::cout << "~Bill\n"; }
};

class OutPatientBill : public Bill {
    double amount;
public:
    explicit OutPatientBill(double a) : amount(a) {}
    double total() const override { return amount; }
    ~OutPatientBill() override { std::cout << "~OPBill\n"; }
};

void show(Bill* b) {
    // show is non-virtual, but calls the virtual total()
    std::cout << b->total() << "\n";
}

int main() {
    std::cout << "===== Task 02 : Runtime Binding =====\n\n";

    Bill* b = new OutPatientBill(500);
    show(b);          // late binding → OPBill::total
    delete b;         // virtual destructor → ~OPBill then ~Bill

    return 0;
}
