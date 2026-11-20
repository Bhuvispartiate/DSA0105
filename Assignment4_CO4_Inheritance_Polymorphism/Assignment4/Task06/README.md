# Task 06 – Design Challenge: Vehicle Rental Fleet

**Marks:** 13

## Hierarchy
```
Vehicle (abstract – pure virtual charge())
 ├── Bicycle
 └── MotorVehicle
      ├── Car
      │    └── ElectricCar
      └── ElectricBus          ← added without editing existing classes
```

## Critical Decisions
- `charge()` is pure virtual → every concrete class must supply it
- Destructor of `Vehicle` is virtual
- Fleet stored as `vector<unique_ptr<Vehicle>>`
- Late return adds a defined surcharge

## How to Run
```bash
g++ -std=c++17 -Wall -Wextra -o task06 task06.cpp
./task06
```
