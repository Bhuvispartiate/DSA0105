// Q5 – Static Members and Arrays of Objects
// OOP with C++ | Assignment 2 – CO2

#include <iostream>
#include <string>

class BusTicket {
    static int issued;
    static const int CAPACITY = 3;
    int seat;
    std::string name;
public:
    explicit BusTicket(const std::string& n) : seat(0), name(n) {
        if (issued < CAPACITY) {
            ++issued;
            seat = issued;          // seats 1, 2, 3
        }
    }

    static int count() { return issued; }

    void print() const {
        if (seat > 0)
            std::cout << name << " – Seat " << seat << "\n";
        else
            std::cout << name << " – Waiting list\n";
    }
};

int BusTicket::issued = 0;   // definition required

int main() {
    std::cout << "===== Q5 : College Bus Seat Tickets =====\n\n";

    BusTicket t1("Asha");
    BusTicket t2("Bala");
    BusTicket t3("Chitra");
    BusTicket t4("Dev");

    t1.print();
    t2.print();
    t3.print();
    t4.print();

    std::cout << "\nTotal issued : " << BusTicket::count() << "\n";
    return 0;
}
