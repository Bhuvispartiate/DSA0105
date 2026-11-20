// Task 07 – From Procedural to Object-Oriented
// OOP with C++ | Assignment 1 – Namma Metro Smart-Card System

#include <iostream>

constexpr long long MIN_BALANCE_PAISE = 5000;

class SmartCard {
private:
    long long balancePaise;
public:
    explicit SmartCard(long long initial = 0) : balancePaise(initial) {}

    long long getBalance() const { return balancePaise; }

    bool canTravel(long long fare) const {
        return (balancePaise - fare) >= MIN_BALANCE_PAISE;
    }

    bool deduct(long long fare) {
        if (!canTravel(fare)) return false;
        balancePaise -= fare;
        return true;
    }

    void recharge(long long amount) {
        if (amount > 0) balancePaise += amount;
    }
};

int main()
{
    std::cout << "===== Task 07 : SmartCard Class =====\n\n";

    SmartCard card(10000);   // Rs 100.00
    std::cout << "Initial balance : " << card.getBalance() << " paise\n";

    long long fare = 3600;   // Outer peak
    if (card.deduct(fare)) {
        std::cout << "Journey of " << fare << " paise accepted.\n";
        std::cout << "New balance     : " << card.getBalance() << " paise\n";
    } else {
        std::cout << "Journey refused – insufficient balance.\n";
    }

    // Attempt a journey that would break the minimum balance rule
    fare = 2000;
    std::cout << "\nAttempt fare " << fare << " with balance "
              << card.getBalance() << " → ";
    if (card.deduct(fare))
        std::cout << "Accepted\n";
    else
        std::cout << "Refused (would go below Rs 50)\n";

    return 0;
}
