// Task 01 – Class Design Review (Diagnose & Repair)
// OOP with C++ | Assignment 4 – CO4

#include <iostream>
#include <string>
#include <vector>
#include <memory>

class Staff {
protected:
    std::string name;
    int id;
    double basic;
    static constexpr double DA_RATE = 0.42;
public:
    Staff(const std::string& n, int i, double b)
        : name(n), id(i), basic(b) {}
    virtual double pay() const = 0;
    virtual void print() const {
        std::cout << name << " (ID " << id << ")  Pay = " << pay() << "\n";
    }
    virtual ~Staff() = default;
};

class Professor : public Staff {
    double research;
public:
    Professor(const std::string& n, int i, double b, double r)
        : Staff(n, i, b), research(r) {}
    double pay() const override {
        return basic + basic * DA_RATE + research;
    }
};

class LabAssistant : public Staff {
public:
    LabAssistant(const std::string& n, int i, double b)
        : Staff(n, i, b) {}
    double pay() const override {
        return basic + basic * DA_RATE;
    }
};

class GuestFaculty : public Staff {
    double hours, rate;
public:
    GuestFaculty(const std::string& n, int i, double h, double r)
        : Staff(n, i, 0), hours(h), rate(r) {}
    double pay() const override {
        return hours * rate;
    }
};

int main() {
    std::cout << "===== Task 01 : Staff Hierarchy =====\n\n";

    std::vector<std::unique_ptr<Staff>> staff;
    staff.push_back(std::make_unique<Professor>("Dr. Rao", 101, 50000, 8000));
    staff.push_back(std::make_unique<LabAssistant>("Ravi", 202, 25000));
    staff.push_back(std::make_unique<GuestFaculty>("Priya", 303, 40, 800));

    for (const auto& s : staff)
        s->print();

    return 0;
}
