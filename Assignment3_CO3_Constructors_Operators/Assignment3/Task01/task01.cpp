// Task 01 – Constructors and Constructor Overloading
// OOP with C++ | Assignment 3 – CO3

#include <iostream>
#include <string>

class Book {
    std::string title;
    std::string author;
    int copies;
public:
    // default – empty record
    Book() : Book("", "", 0) {}

    // donated book – only title known
    explicit Book(const std::string& t) : Book(t, "Unknown", 1) {}

    // full details
    Book(const std::string& t, const std::string& a, int c)
        : title(t), author(a), copies(c < 0 ? 0 : c) {}

    void print() const {
        std::cout << "\"" << title << "\" by " << author
                  << "  (copies: " << copies << ")\n";
    }
};

int main() {
    std::cout << "===== Task 01 : Constructors =====\n\n";

    Book b1;                                    // default
    Book b2("Clean Code");                      // single-arg (explicit)
    Book b3("The C++ Programming Language", "Bjarne Stroustrup", 5);

    b1.print();
    b2.print();
    b3.print();

    return 0;
}
