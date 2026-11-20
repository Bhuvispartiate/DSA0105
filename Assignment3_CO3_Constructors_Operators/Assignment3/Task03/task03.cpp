// Task 03 – Dynamic Constructors and Destructors
// OOP with C++ | Assignment 3 – CO3

#include <iostream>

class Event {
    int rows, seats;
    bool** map;
public:
    Event(int r, int s) : rows(r), seats(s), map(nullptr) {
        if (rows <= 0 || seats <= 0) return;
        map = new bool*[rows];
        for (int i = 0; i < rows; ++i)
            map[i] = new bool[seats]();   // value-initialised to false
        std::cout << "Event opened (" << rows << "x" << seats << ")\n";
    }

    ~Event() {
        if (!map) return;
        for (int i = 0; i < rows; ++i) delete[] map[i];
        delete[] map;
        std::cout << "Event closed (" << rows << "x" << seats << ")\n";
    }

    bool book(int r, int s) {
        if (!map || r < 0 || r >= rows || s < 0 || s >= seats) return false;
        if (map[r][s]) return false;
        map[r][s] = true;
        return true;
    }
};

int main() {
    std::cout << "===== Task 03 : Dynamic Constructor / Destructor =====\n\n";

    {
        Event e(5, 10);
        std::cout << "Book seat (2,3): " << (e.book(2, 3) ? "OK" : "Fail") << "\n";
        std::cout << "Book seat (2,3) again: " << (e.book(2, 3) ? "OK" : "Fail") << "\n";
    }   // destructor runs here automatically (RAII)

    std::cout << "\nBlock ended – resources released.\n";
    return 0;
}
