// Task 01 – Understand the Problem and Identify the Objects
// OOP with C++ | Assignment 1 – Namma Metro Smart-Card System

#include <iostream>
#include <string>

// Simple illustration of the objects identified in the problem

enum class Zone { Inner, Middle, Outer };

class SmartCard {
private:
    long long balancePaise;
    std::string cardId;
public:
    SmartCard(std::string id, long long initial)
        : balancePaise(initial), cardId(id) {}

    long long getBalance() const { return balancePaise; }
    std::string getId() const { return cardId; }

    bool canTravel(long long fare) const {
        return (balancePaise - fare) >= 5000;   // min Rs 50
    }

    bool deduct(long long fare) {
        if (!canTravel(fare)) return false;
        balancePaise -= fare;
        return true;
    }
};

int main() {
    std::cout << "===== Task 01 : Object Identification Demo =====\n\n";

    SmartCard card("NM-2025-001", 15000);   // Rs 150.00

    std::cout << "Card ID      : " << card.getId() << "\n";
    std::cout << "Balance      : " << card.getBalance() << " paise\n";
    std::cout << "Can travel (fare 3000)? "
              << (card.canTravel(3000) ? "Yes" : "No") << "\n";

    if (card.deduct(3000)) {
        std::cout << "Journey accepted. New balance : "
                  << card.getBalance() << " paise\n";
    }

    std::cout << "\nObjects identified in the system:\n";
    std::cout << "  1. SmartCard   - owns balance and enforces rules\n";
    std::cout << "  2. Passenger   - carries category for concession\n";
    std::cout << "  3. Journey     - entry/exit zone + time\n";
    std::cout << "  4. Zone        - Inner / Middle / Outer\n";
    std::cout << "  5. FareTable   - holds all monetary constants\n";

    return 0;
}
