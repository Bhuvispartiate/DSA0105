// Q6 – Friend Functions and Inline Functions
// OOP with C++ | Assignment 2 – CO2

#include <iostream>

class Time {
    int h, m;
public:
    Time(int hour, int min) : h(hour), m(min) {}

    friend int  minutesBetween(const Time& a, const Time& b);
    friend bool earlier(const Time& a, const Time& b);

    void print() const {
        std::cout << h << ":" << (m < 10 ? "0" : "") << m;
    }
};

inline int minutesBetween(const Time& a, const Time& b) {
    int ma = a.h * 60 + a.m;
    int mb = b.h * 60 + b.m;
    int diff = ma - mb;
    return diff >= 0 ? diff : -diff;
}

inline bool earlier(const Time& a, const Time& b) {
    return (a.h * 60 + a.m) < (b.h * 60 + b.m);
}

int main() {
    std::cout << "===== Q6 : Friend & Inline Functions =====\n\n";

    Time t1(9, 15);
    Time t2(12, 40);

    std::cout << "Time A : "; t1.print(); std::cout << "\n";
    std::cout << "Time B : "; t2.print(); std::cout << "\n";
    std::cout << "Minutes between : " << minutesBetween(t1, t2) << "\n";
    std::cout << "A earlier than B? " << std::boolalpha << earlier(t1, t2) << "\n";

    return 0;
}
