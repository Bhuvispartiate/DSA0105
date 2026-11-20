# Task 08 – Read and Print Data Safely

**Marks:** 4

## Objective
Read numeric input safely and print monetary values with correct formatting.

## Safe Numeric Input Pattern

```cpp
int hour;
std::cout << "Enter hour (0-23): ";
while (!(std::cin >> hour) || hour < 0 || hour > 23) {
    std::cin.clear();
    std::cin.ignore(10000, '\n');
    std::cout << "Invalid hour, re-enter: ";
}
```

## Formatted Money Output

```cpp
std::cout << "Fare Rs " << (fare / 100) << '.'
          << std::setw(2) << std::setfill('0') << (fare % 100);
```

This guarantees that 5 paise is printed as `.05` rather than `.5`.

## How to Run

```bash
g++ -std=c++17 -Wall -Wextra -o task08 task08.cpp
./task08
```
