# OOP with C++ | Assignment 1 – Namma Metro Smart-Card System

**Student:** Bhuvanesh H  
**Register Number:** 192524027  
**Academic Year:** 2025–26  
**Total Marks:** 50  

## Objective
Design, implement and validate the fare logic of a city metro smart-card system using object-oriented programming principles in C++.

## Repository Structure

```
Assignment1/
├── README.md
├── Task01/
├── Task02/
├── Task03/
├── Task04/
├── Task05/
├── Task06/
├── Task07/
├── Task08/
├── Task09/
├── Task10/
├── Task11/
└── Task12/
```

Each Task folder contains:
- `taskXX.cpp` – source code
- `output.txt` – program output
- `README.md` – explanation of the task

## Compile & Run

```bash
cd Task0X
g++ -std=c++17 -Wall -Wextra -o task0X task0X.cpp
./task0X
```

## Key Design Decisions

| Decision              | Choice                                      |
|-----------------------|---------------------------------------------|
| Money representation  | `long long` (paise)                         |
| Zones                 | `enum class Zone`                           |
| Constants             | `constexpr`                                 |
| Minimum balance       | ₹50 (5000 paise)                            |
| Peak hours            | [8,10) and [17,20)                          |
| Student concession    | 25%                                         |
| Senior concession     | 30%                                         |
