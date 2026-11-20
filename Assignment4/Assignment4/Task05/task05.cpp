// Task 05 – Pointers to Objects and the this Pointer
// OOP with C++ | Assignment 4 – CO4

#include <iostream>
#include <string>
#include <iomanip>

class Cart {
    std::string items[10];
    double prices[10];
    int n = 0;
    double discount = 0.0;
public:
    Cart& add(const std::string& item, double price) {
        if (n < 10) {
            items[n] = item;
            prices[n] = price;
            ++n;
        }
        return *this;                 // enable chaining
    }

    Cart& setDiscount(double d) {
        this->discount = d;           // member vs parameter
        return *this;
    }

    double total() const {
        double sum = 0;
        for (int i = 0; i < n; ++i) sum += prices[i];
        return sum * (1.0 - discount);
    }

    void print() const {
        for (int i = 0; i < n; ++i)
            std::cout << "  " << items[i] << "  " << prices[i] << "\n";
        std::cout << "Total (after discount): "
                  << std::fixed << std::setprecision(2) << total() << "\n";
    }
};

int main() {
    std::cout << "===== Task 05 : this Pointer & Method Chaining =====\n\n";

    Cart c;
    c.add("Notebook", 120.0)
     .add("Pen Set", 80.0)
     .add("Bag", 160.0)
     .setDiscount(0.10);

    c.print();
    return 0;
}
