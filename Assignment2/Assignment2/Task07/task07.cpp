// Q7 – Day-4 Design Challenge (Circular Buffer)
// OOP with C++ | Assignment 2 – CO2

#include <iostream>
#include <iomanip>

class SavingsAccount {
private:
    long long accNo;
    long long balance;
    long long trans[50];     // fixed history of last 50
    int head;                // next write position
    int count;               // how many valid entries (≤ 50)

    void record(long long amount) {
        trans[head] = amount;
        head = (head + 1) % 50;
        if (count < 50) ++count;
    }

public:
    SavingsAccount(long long no, long long bal = 0)
        : accNo(no), balance(bal), head(0), count(0) {
        for (int i = 0; i < 50; ++i) trans[i] = 0;
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
        std::cout << "Account " << accNo
                  << "  Balance: " << balance / 100 << "."
                  << std::setfill('0') << std::setw(2) << balance % 100 << "\n";
        std::cout << "Last " << count << " transactions (newest last):\n";

        // walk the circular buffer in chronological order
        int start = (head - count + 50) % 50;
        for (int i = 0; i < count; ++i) {
            int idx = (start + i) % 50;
            long long t = trans[idx];
            std::cout << "  " << (t >= 0 ? "+" : "")
                      << t / 100 << "."
                      << std::setfill('0') << std::setw(2)
                      << (t >= 0 ? t % 100 : (-t) % 100) << "\n";
        }
    }
};

int main() {
    std::cout << "===== Q7 : Circular-Buffer Savings Account =====\n\n";

    SavingsAccount acc(987654, 200000);   // Rs 2000.00

    acc.deposit(50000);
    acc.withdraw(25000);
    acc.deposit(10000);
    acc.withdraw(5000);
    acc.deposit(30000);

    acc.miniStatement();

    std::cout << "\nInsertion cost is O(1) regardless of history length.\n";
    return 0;
}
