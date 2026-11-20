// Q4 – Specifying a Class with an Array Inside It
// OOP with C++ | Assignment 2 – CO2

#include <iostream>
#include <iomanip>

class SavingsAccount {
private:
    long long accNo;
    long long balance;          // paise
    long long trans[10];        // last 10 amounts (+credit / -debit)
    int count;                  // how many slots filled (0..10)
public:
    SavingsAccount(long long no, long long bal = 0)
        : accNo(no), balance(bal), count(0) {
        for (int i = 0; i < 10; ++i) trans[i] = 0;
    }

    void deposit(long long amount) {
        if (amount <= 0) return;
        balance += amount;
        record(amount);
    }

    bool withdraw(long long amount) {
        if (amount <= 0 || amount > balance) return false;
        balance -= amount;
        record(-amount);
        return true;
    }

    void miniStatement() const {
        std::cout << "Account " << accNo << "  Balance: "
                  << balance / 100 << "." << std::setfill('0')
                  << std::setw(2) << balance % 100 << "\n";
        std::cout << "Last " << count << " transactions:\n";
        for (int i = 0; i < count; ++i) {
            long long t = trans[i];
            std::cout << "  " << (t >= 0 ? "+" : "")
                      << t / 100 << "." << std::setfill('0')
                      << std::setw(2) << (t >= 0 ? t % 100 : (-t) % 100) << "\n";
        }
    }

private:
    void record(long long amount) {
        if (count < 10) {
            trans[count++] = amount;
        } else {
            // shift left – drop oldest
            for (int i = 0; i < 9; ++i) trans[i] = trans[i + 1];
            trans[9] = amount;
        }
    }
};

int main() {
    std::cout << "===== Q4 : Savings Account Mini-Statement =====\n\n";

    SavingsAccount acc(123456, 100000);   // Rs 1000.00
    acc.deposit(50000);                   // +500
    acc.withdraw(20000);                  // -200
    acc.deposit(15000);                   // +150
    acc.miniStatement();

    return 0;
}
