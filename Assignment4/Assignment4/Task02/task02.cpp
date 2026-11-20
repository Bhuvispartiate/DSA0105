// Task 02 – University People Hierarchy (Virtual Bases)
// OOP with C++ | Assignment 4 – CO4

#include <iostream>
#include <string>

class Person {
protected:
    std::string name;
    int id;
public:
    Person(const std::string& n, int i) : name(n), id(i) {
        std::cout << "Person(" << name << ")\n";
    }
    virtual ~Person() { std::cout << "~Person\n"; }
    void show() const { std::cout << name << " (ID " << id << ")\n"; }
};

class Student : virtual public Person {
protected:
    std::string programme;
public:
    Student(const std::string& n, int i, const std::string& prog)
        : Person(n, i), programme(prog) {
        std::cout << "Student\n";
    }
    ~Student() { std::cout << "~Student\n"; }
};

class Staff : virtual public Person {
protected:
    std::string dept;
public:
    Staff(const std::string& n, int i, const std::string& d)
        : Person(n, i), dept(d) {
        std::cout << "Staff\n";
    }
    ~Staff() { std::cout << "~Staff\n"; }
};

class TeachingAssistant : public Student, public Staff {
public:
    // most-derived class must initialise the virtual base
    TeachingAssistant(const std::string& n, int i,
                      const std::string& prog, const std::string& d)
        : Person(n, i), Student(n, i, prog), Staff(n, i, d) {
        std::cout << "TeachingAssistant\n";
    }
    ~TeachingAssistant() { std::cout << "~TeachingAssistant\n"; }
};

int main() {
    std::cout << "===== Task 02 : Virtual Bases (Diamond) =====\n\n";
    {
        TeachingAssistant ta("Ananya", 501, "M.Tech", "CSE");
        std::cout << "\nIdentity: ";
        ta.show();
        std::cout << "\nDestruction order:\n";
    }
    return 0;
}
