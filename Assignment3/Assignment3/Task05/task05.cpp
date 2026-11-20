// Task 05 – Overloading Increment, Comparison and Subscript
// OOP with C++ | Assignment 3 – CO3

#include <iostream>

class Time {
    int hour, min, sec;
public:
    Time(int h = 0, int m = 0, int s = 0) : hour(h), min(m), sec(s) {}

    // prefix ++ : increment, return new value
    Time& operator++() {
        if (++sec == 60) {
            sec = 0;
            if (++min == 60) { min = 0; ++hour; }
        }
        return *this;
    }

    // postfix ++ : return old value, then increment
    Time operator++(int) {
        Time old = *this;
        ++(*this);
        return old;
    }

    void print() const {
        std::cout << hour << ":" 
                  << (min < 10 ? "0" : "") << min << ":"
                  << (sec < 10 ? "0" : "") << sec;
    }
};

int main() {
    std::cout << "===== Task 05 : Prefix vs Postfix ++ =====\n\n";

    Time t(1, 59, 58);
    std::cout << "Start          : "; t.print(); std::cout << "\n";

    Time a = t++;          // postfix – a gets old value
    std::cout << "After t++      : "; t.print(); std::cout << "\n";
    std::cout << "a (old value)  : "; a.print(); std::cout << "\n";

    Time b = ++t;          // prefix – b gets new value
    std::cout << "After ++t      : "; t.print(); std::cout << "\n";
    std::cout << "b (new value)  : "; b.print(); std::cout << "\n";

    return 0;
}
