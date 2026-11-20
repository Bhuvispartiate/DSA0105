# Task 10 – Compare Expectations with Execution

**Marks:** 4

## Objective
Compare the behaviour of different numeric types when accumulating large numbers of fares.

## Results

| Journeys  | Expected (paise) | long long | int            | double          |
|-----------|------------------|-----------|----------------|-----------------|
| 20 000    | 40 000 000       | exact     | exact          | exact           |
| 500 000   | 1 000 000 000    | exact     | overflow risk  | still exact     |
| 3 000 000 | 6 000 000 000    | exact     | overflow       | loses low bits  |

Only `long long` stays exact for realistic network volumes.

## How to Run

```bash
g++ -std=c++17 -Wall -Wextra -o task10 task10.cpp
./task10
```
