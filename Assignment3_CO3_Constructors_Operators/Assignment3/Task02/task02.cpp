// Task 02 – Copy Constructors: Shallow and Deep Copies
// OOP with C++ | Assignment 3 – CO3

#include <iostream>
#include <string>

class Prescription {
    int n;
    std::string* medicines;
public:
    Prescription(int count, const std::string* list)
        : n(count), medicines(new std::string[count]) {
        for (int i = 0; i < n; ++i) medicines[i] = list[i];
    }

    // Deep copy constructor (Rule of Three)
    Prescription(const Prescription& other)
        : n(other.n), medicines(new std::string[other.n]) {
        for (int i = 0; i < n; ++i) medicines[i] = other.medicines[i];
    }

    // Deep copy assignment
    Prescription& operator=(const Prescription& other) {
        if (this == &other) return *this;
        std::string* neu = new std::string[other.n];
        for (int i = 0; i < other.n; ++i) neu[i] = other.medicines[i];
        delete[] medicines;
        medicines = neu;
        n = other.n;
        return *this;
    }

    ~Prescription() { delete[] medicines; }

    void print() const {
        for (int i = 0; i < n; ++i) {
            if (i) std::cout << " ";
            std::cout << medicines[i];
        }
        std::cout << "\n";
    }
};

int main() {
    std::cout << "===== Task 02 : Deep Copy (Rule of Three) =====\n\n";

    std::string list1[] = {"Ibuprofen", "ORS"};
    std::string list2[] = {"Paracetamol", "Cetirizine"};

    Prescription p1(2, list1);
    Prescription p2 = p1;               // copy constructor
    Prescription p3(2, list2);
    p3 = p1;                            // copy assignment

    std::cout << "p1: "; p1.print();
    std::cout << "p2: "; p2.print();
    std::cout << "p3: "; p3.print();

    return 0;
}
