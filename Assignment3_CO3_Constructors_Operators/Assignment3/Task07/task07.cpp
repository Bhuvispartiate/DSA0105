// Task 07 – Design a Complex-Number Class from First Principles
// OOP with C++ | Assignment 3 – CO3

#include <iostream>
#include <cmath>
#include <stdexcept>

class Complex {
    double re, im;
public:
    Complex(double r = 0, double i = 0) : re(r), im(i) {}

    Complex operator+(const Complex& o) const { return {re + o.re, im + o.im}; }
    Complex operator-(const Complex& o) const { return {re - o.re, im - o.im}; }

    Complex operator*(const Complex& o) const {
        return {re * o.re - im * o.im, re * o.im + im * o.re};
    }

    Complex operator/(const Complex& o) const {
        double d = o.re * o.re + o.im * o.im;
        if (d == 0.0) throw std::runtime_error("division by zero impedance");
        return {(re * o.re + im * o.im) / d, (im * o.re - re * o.im) / d};
    }

    bool operator==(const Complex& o) const {
        return re == o.re && im == o.im;
    }

    double mag() const { return std::sqrt(re * re + im * im); }

    friend std::ostream& operator<<(std::ostream& os, const Complex& z) {
        os << z.re << (z.im >= 0 ? " + " : " - ") << std::abs(z.im) << "j";
        return os;
    }
};

int main() {
    std::cout << "===== Task 07 : Complex Number / Impedance =====\n\n";

    Complex z1(30, 40);          // series R-L : 30 + j40
    std::cout << "Z1 = " << z1 << "\n";
    std::cout << "Magnitude = " << z1.mag() << "\n\n";

    Complex z2(0, -30);
    Complex series = z1 + z2;
    std::cout << "Series  Z1+Z2 = " << series << "\n";

    Complex parallel = (z1 * z2) / (z1 + z2);
    std::cout << "Parallel      = " << parallel << "\n";

    try {
        Complex bad = z1 / Complex(0, 0);
        std::cout << bad << "\n";
    } catch (const std::runtime_error& e) {
        std::cout << "\nCaught: " << e.what() << "\n";
    }

    return 0;
}
