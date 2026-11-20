// Task 06 – Design Challenge: Vehicle Rental Fleet
// OOP with C++ | Assignment 4 – CO4

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <iomanip>

class Vehicle {
protected:
    std::string reg;
    double dailyRate;
    bool out = false;
public:
    Vehicle(const std::string& r, double rate) : reg(r), dailyRate(rate) {}
    virtual double charge(int days) const = 0;
    virtual void rent() {
        if (out) { std::cout << reg << " already rented\n"; return; }
        out = true;
        std::cout << reg << " rented\n";
    }
    virtual void returnVehicle(int days, int daysLate = 0) {
        if (!out) { std::cout << reg << " was not out\n"; return; }
        double fee = charge(days);
        if (daysLate > 0) fee += daysLate * dailyRate * 0.5;  // late surcharge
        out = false;
        std::cout << reg << " returned. Charge = Rs "
                  << std::fixed << std::setprecision(2) << fee << "\n";
    }
    virtual ~Vehicle() = default;
    std::string id() const { return reg; }
};

class Bicycle : public Vehicle {
public:
    Bicycle(const std::string& r, double rate) : Vehicle(r, rate) {}
    double charge(int days) const override { return days * dailyRate; }
};

class MotorVehicle : public Vehicle {
protected:
    double fuelLevel;
public:
    MotorVehicle(const std::string& r, double rate, double fuel = 100)
        : Vehicle(r, rate), fuelLevel(fuel) {}
};

class Car : public MotorVehicle {
    int seats;
public:
    Car(const std::string& r, double rate, int s = 5)
        : MotorVehicle(r, rate), seats(s) {}
    double charge(int days) const override {
        return days * dailyRate + (seats > 5 ? 200 : 0);
    }
};

class ElectricCar : public Car {
    double batteryKWh;
public:
    ElectricCar(const std::string& r, double rate, double kWh)
        : Car(r, rate, 5), batteryKWh(kWh) {}
    double charge(int days) const override {
        return days * dailyRate + batteryKWh * 8.0;  // per-kWh component
    }
};

class ElectricBus : public MotorVehicle {
    int capacity;
public:
    ElectricBus(const std::string& r, double rate, int cap)
        : MotorVehicle(r, rate), capacity(cap) {}
    double charge(int days) const override {
        return days * dailyRate + capacity * 2.0;
    }
};

int main() {
    std::cout << "===== Task 06 : Vehicle Rental Fleet =====\n\n";

    std::vector<std::unique_ptr<Vehicle>> fleet;
    fleet.push_back(std::make_unique<Bicycle>("BIKE-01", 150));
    fleet.push_back(std::make_unique<Car>("CAR-101", 1200, 5));
    fleet.push_back(std::make_unique<ElectricCar>("EV-55", 1800, 40));
    fleet.push_back(std::make_unique<ElectricBus>("EBUS-7", 5000, 40));

    // rent a few
    fleet[0]->rent();
    fleet[1]->rent();
    fleet[2]->rent();

    // return car two days late
    fleet[1]->returnVehicle(3, 2);

    // return others normally
    fleet[0]->returnVehicle(1);
    fleet[2]->returnVehicle(2);

    // ElectricBus never needed a change to existing classes
    fleet[3]->rent();
    fleet[3]->returnVehicle(1);

    return 0;
}
